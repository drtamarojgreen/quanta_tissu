#include "ntriples.h"

#include <cctype>
#include <sstream>

namespace TissDB {
namespace NTriples {

namespace {

bool is_hex(char c) {
    return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

uint32_t hex_value(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint32_t>(c - '0');
    return static_cast<uint32_t>(std::tolower(static_cast<unsigned char>(c)) - 'a' + 10);
}

// Sequential reader over a single line of N-Triples.
class Cursor {
public:
    Cursor(const std::string& text, size_t line) : text_(text), line_(line) {}

    bool at_end() const { return pos_ >= text_.size(); }
    char peek() const { return at_end() ? '\0' : text_[pos_]; }

    void skip_whitespace() {
        while (!at_end() && (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\r')) pos_++;
    }

    // True if nothing but whitespace and an optional comment remains.
    bool only_comment_left() {
        skip_whitespace();
        return at_end() || peek() == '#';
    }

    [[noreturn]] void fail(const std::string& message) const {
        throw ParseError(message + " (column " + std::to_string(pos_ + 1) + ")", line_);
    }

    void expect(char c) {
        if (peek() != c) fail(std::string("expected '") + c + "'");
        pos_++;
    }

    Rdf::Term read_term() {
        char c = peek();
        if (c == '<') return Rdf::Term::iri(read_iri());
        if (c == '_') return read_blank_node();
        if (c == '"') return read_literal();
        fail("expected an IRI, blank node or literal");
    }

    std::string read_iri() {
        expect('<');
        std::string iri;
        while (true) {
            if (at_end()) fail("unterminated IRI");
            char c = text_[pos_];
            if (c == '>') break;
            if (c == '\\') {
                pos_++;
                char kind = peek();
                if (kind != 'u' && kind != 'U') fail("only \\u and \\U escapes are allowed in IRIs");
                read_unicode_escape(iri);
                continue;
            }
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc <= 0x20 || c == '<' || c == '"' || c == '{' || c == '}' || c == '|' || c == '^' || c == '`') {
                fail("invalid character in IRI");
            }
            iri += c;
            pos_++;
        }
        pos_++; // consume '>'
        return iri;
    }

    Rdf::Term read_blank_node() {
        expect('_');
        expect(':');
        size_t start = pos_;
        while (!at_end()) {
            char c = text_[pos_];
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.' ||
                static_cast<unsigned char>(c) >= 0x80) {
                pos_++;
            } else {
                break;
            }
        }
        // A label may not end with '.', which would be the statement terminator.
        while (pos_ > start && text_[pos_ - 1] == '.') pos_--;
        if (pos_ == start) fail("empty blank node label");
        if (text_[start] == '-') fail("blank node label may not start with '-'");
        return Rdf::Term::blank(text_.substr(start, pos_ - start));
    }

    Rdf::Term read_literal() {
        expect('"');
        std::string lexical;
        while (true) {
            if (at_end()) fail("unterminated string literal");
            char c = text_[pos_];
            if (c == '"') break;
            if (c == '\n' || c == '\r') fail("raw line break in string literal");
            if (c == '\\') {
                pos_++;
                read_string_escape(lexical);
                continue;
            }
            lexical += c;
            pos_++;
        }
        pos_++; // consume closing quote

        if (peek() == '@') {
            pos_++;
            size_t start = pos_;
            while (!at_end() && std::isalpha(static_cast<unsigned char>(peek()))) pos_++;
            if (pos_ == start) fail("empty language tag");
            while (peek() == '-') {
                pos_++;
                size_t sub_start = pos_;
                while (!at_end() && std::isalnum(static_cast<unsigned char>(peek()))) pos_++;
                if (pos_ == sub_start) fail("malformed language tag");
            }
            return Rdf::Term::lang_literal(lexical, text_.substr(start, pos_ - start));
        }
        if (peek() == '^') {
            pos_++;
            expect('^');
            return Rdf::Term::literal(lexical, read_iri());
        }
        return Rdf::Term::literal(lexical);
    }

private:
    void read_string_escape(std::string& out) {
        char c = peek();
        switch (c) {
            case 't': out += '\t'; break;
            case 'b': out += '\b'; break;
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 'f': out += '\f'; break;
            case '"': out += '"'; break;
            case '\'': out += '\''; break;
            case '\\': out += '\\'; break;
            case 'u':
            case 'U':
                read_unicode_escape(out);
                return;
            default:
                fail("invalid escape sequence");
        }
        pos_++;
    }

    // Reads uXXXX or UXXXXXXXX (the backslash has already been consumed).
    void read_unicode_escape(std::string& out) {
        size_t digits = peek() == 'u' ? 4 : 8;
        pos_++;
        if (pos_ + digits > text_.size()) fail("truncated unicode escape");
        uint32_t cp = 0;
        for (size_t i = 0; i < digits; ++i) {
            char h = text_[pos_ + i];
            if (!is_hex(h)) fail("invalid hex digit in unicode escape");
            cp = cp * 16 + hex_value(h);
        }
        if (cp > 0x10FFFF) fail("unicode escape out of range");
        pos_ += digits;
        Rdf::append_utf8(out, cp);
    }

    const std::string& text_;
    size_t line_;
    size_t pos_ = 0;
};

} // anonymous namespace

std::optional<Rdf::Triple> parse_line(const std::string& line, size_t line_number) {
    Cursor cursor(line, line_number);
    if (cursor.only_comment_left()) return std::nullopt;

    Rdf::Triple triple;
    triple.subject = cursor.read_term();
    if (triple.subject.is_literal()) cursor.fail("subject must be an IRI or blank node");

    cursor.skip_whitespace();
    if (cursor.peek() != '<') cursor.fail("predicate must be an IRI");
    triple.predicate = cursor.read_term();

    cursor.skip_whitespace();
    triple.object = cursor.read_term();

    cursor.skip_whitespace();
    cursor.expect('.');
    if (!cursor.only_comment_left()) cursor.fail("unexpected content after '.'");
    return triple;
}

std::vector<Rdf::Triple> parse(const std::string& document) {
    std::vector<Rdf::Triple> triples;
    std::istringstream stream(document);
    std::string line;
    size_t line_number = 0;
    while (std::getline(stream, line)) {
        line_number++;
        if (auto triple = parse_line(line, line_number)) {
            triples.push_back(std::move(*triple));
        }
    }
    return triples;
}

Rdf::Term parse_term(const std::string& text) {
    Cursor cursor(text, 0);
    cursor.skip_whitespace();
    Rdf::Term term = cursor.read_term();
    cursor.skip_whitespace();
    if (!cursor.at_end()) cursor.fail("unexpected content after term");
    return term;
}

std::string serialize(const std::vector<Rdf::Triple>& triples) {
    std::string out;
    for (const auto& triple : triples) {
        out += triple.to_ntriples();
        out += '\n';
    }
    return out;
}

} // namespace NTriples
} // namespace TissDB
