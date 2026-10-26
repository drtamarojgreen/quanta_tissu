#include "sparql_parser.h"
#include "../common/log.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <stdexcept>

namespace TissDB {
namespace Query {

namespace {

std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return s;
}

bool is_name_start(char ch) {
    unsigned char c = static_cast<unsigned char>(ch);
    return std::isalpha(c) || c == '_' || c >= 0x80;
}

bool is_name_char(char ch) {
    unsigned char c = static_cast<unsigned char>(ch);
    return std::isalnum(c) || c == '_' || c == '-' || c >= 0x80;
}

bool is_digit(char ch) {
    return std::isdigit(static_cast<unsigned char>(ch)) != 0;
}

// Constructs that SPARQL has but TissDB does not, reported by name.
const std::set<std::string>& unsupported_keywords() {
    static const std::set<std::string> keywords = {"UNION", "MINUS", "GRAPH", "BIND", "VALUES", "SERVICE"};
    return keywords;
}

// Decodes a \u or \U escape. i points at the 'u'/'U'; on success it is advanced
// past the hex digits and the code point is appended to out.
bool decode_unicode_escape(const std::string& q, size_t& i, std::string& out) {
    size_t digits = q[i] == 'u' ? 4 : 8;
    if (i + digits >= q.size()) return false;
    uint32_t cp = 0;
    for (size_t k = 1; k <= digits; ++k) {
        char h = q[i + k];
        if (!std::isxdigit(static_cast<unsigned char>(h))) return false;
        cp = cp * 16 + static_cast<uint32_t>(is_digit(h) ? h - '0' : std::tolower(static_cast<unsigned char>(h)) - 'a' + 10);
    }
    if (cp > 0x10FFFF) return false;
    Rdf::append_utf8(out, cp);
    i += digits + 1;
    return true;
}

bool has_scheme(const std::string& iri) {
    if (iri.empty() || !std::isalpha(static_cast<unsigned char>(iri[0]))) return false;
    for (char c : iri) {
        if (c == ':') return true;
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' && c != '-' && c != '.') return false;
    }
    return false;
}

std::shared_ptr<SparqlExpression> make_binary(const std::string& op, std::shared_ptr<SparqlExpression> left,
                                              std::shared_ptr<SparqlExpression> right) {
    auto e = std::make_shared<SparqlExpression>();
    e->kind = SparqlExpression::Kind::BINARY;
    e->op = op;
    e->args = {std::move(left), std::move(right)};
    return e;
}

std::shared_ptr<SparqlExpression> make_term(const Rdf::Term& term) {
    auto e = std::make_shared<SparqlExpression>();
    e->kind = SparqlExpression::Kind::TERM;
    e->term = term;
    return e;
}

} // anonymous namespace

bool is_sparql_query(const std::string& query_string) {
    size_t i = 0;
    const size_t n = query_string.size();
    while (i < n) {
        if (std::isspace(static_cast<unsigned char>(query_string[i]))) {
            i++;
        } else if (query_string[i] == '#') {
            while (i < n && query_string[i] != '\n') i++;
        } else {
            break;
        }
    }
    static const std::string keyword = "PREFIX";
    if (n - i < keyword.size()) return false;
    if (to_upper(query_string.substr(i, keyword.size())) != keyword) return false;
    return i + keyword.size() == n || !is_name_char(query_string[i + keyword.size()]);
}

// --- Tokenizer ---

std::vector<SparqlToken> SparqlParser::tokenize(const std::string& q) {
    using Type = SparqlToken::Type;
    std::vector<SparqlToken> out;
    const size_t n = q.size();
    size_t i = 0;

    auto push = [&](Type type, std::string value, size_t start, size_t end) {
        out.push_back({type, std::move(value), start, q.substr(start, end - start)});
    };

    while (i < n) {
        const char c = q[i];
        const char next = i + 1 < n ? q[i + 1] : '\0';
        const size_t start = i;

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }
        if (c == '#') {
            while (i < n && q[i] != '\n') i++;
            continue;
        }

        if (c == '<') {
            if (next == '=') {
                push(Type::PUNCT, "<=", start, start + 2);
                i += 2;
                continue;
            }
            // '<' starts an IRI if a '>' follows with only IRI characters in between;
            // otherwise it is the less-than operator.
            std::string iri;
            size_t j = i + 1;
            bool is_iri = false;
            while (j < n) {
                char d = q[j];
                unsigned char ud = static_cast<unsigned char>(d);
                if (d == '>') {
                    is_iri = true;
                    break;
                }
                if (ud <= 0x20 || d == '<' || d == '"' || d == '{' || d == '}' || d == '|' || d == '^' || d == '`') break;
                if (d == '\\') {
                    j++;
                    if (j >= n || (q[j] != 'u' && q[j] != 'U') || !decode_unicode_escape(q, j, iri)) break;
                    continue;
                }
                iri += d;
                j++;
            }
            if (is_iri) {
                push(Type::IRI, iri, start, j + 1);
                i = j + 1;
            } else {
                push(Type::PUNCT, "<", start, start + 1);
                i++;
            }
            continue;
        }

        if (c == '>') {
            if (next == '=') {
                push(Type::PUNCT, ">=", start, start + 2);
                i += 2;
            } else {
                push(Type::PUNCT, ">", start, start + 1);
                i++;
            }
            continue;
        }

        if ((c == '?' || c == '$') && is_name_char(next)) {
            size_t j = i + 1;
            while (j < n && (std::isalnum(static_cast<unsigned char>(q[j])) || q[j] == '_' ||
                             static_cast<unsigned char>(q[j]) >= 0x80)) {
                j++;
            }
            push(Type::VAR, q.substr(i + 1, j - i - 1), start, j);
            i = j;
            continue;
        }

        if (c == '"' || c == '\'') {
            const bool long_form = i + 2 < n && q[i + 1] == c && q[i + 2] == c;
            const std::string closing = long_form ? std::string(3, c) : std::string(1, c);
            i += closing.size();
            std::string value;
            while (true) {
                if (i >= n) error_at(start, "unterminated string literal");
                if (q.compare(i, closing.size(), closing) == 0) {
                    i += closing.size();
                    break;
                }
                if (!long_form && (q[i] == '\n' || q[i] == '\r')) {
                    error_at(i, "line break inside a string literal; use \\n or a triple-quoted string");
                }
                if (q[i] == '\\') {
                    i++;
                    if (i >= n) error_at(i - 1, "unterminated escape sequence");
                    switch (q[i]) {
                        case 't': value += '\t'; break;
                        case 'b': value += '\b'; break;
                        case 'n': value += '\n'; break;
                        case 'r': value += '\r'; break;
                        case 'f': value += '\f'; break;
                        case '"': value += '"'; break;
                        case '\'': value += '\''; break;
                        case '\\': value += '\\'; break;
                        case 'u':
                        case 'U':
                            if (!decode_unicode_escape(q, i, value)) error_at(i - 1, "invalid unicode escape; expected \\uXXXX or \\UXXXXXXXX");
                            continue;
                        default:
                            error_at(i - 1, std::string("invalid escape sequence '\\") + q[i] + "' in string literal");
                    }
                    i++;
                    continue;
                }
                value += q[i++];
            }
            push(Type::STRING, value, start, i);
            continue;
        }

        if (c == '@' && std::isalpha(static_cast<unsigned char>(next))) {
            size_t j = i + 1;
            while (j < n && std::isalpha(static_cast<unsigned char>(q[j]))) j++;
            while (j + 1 < n && q[j] == '-' && std::isalnum(static_cast<unsigned char>(q[j + 1]))) {
                j++;
                while (j < n && std::isalnum(static_cast<unsigned char>(q[j]))) j++;
            }
            push(Type::LANGTAG, q.substr(i + 1, j - i - 1), start, j);
            i = j;
            continue;
        }

        if (c == '_' && next == ':') {
            size_t j = i + 2;
            while (j < n && (is_name_char(q[j]) || q[j] == '.')) j++;
            while (j > i + 2 && q[j - 1] == '.') j--;
            if (j == i + 2) error_at(start, "missing blank node label after '_:'");
            push(Type::BLANK, q.substr(i + 2, j - i - 2), start, j);
            i = j;
            continue;
        }

        // Arithmetic is not supported, so '+'/'-' before a digit is always a sign.
        const bool signed_number = (c == '+' || c == '-') &&
                                   (is_digit(next) || (next == '.' && i + 2 < n && is_digit(q[i + 2])));
        if (is_digit(c) || (c == '.' && is_digit(next)) || signed_number) {
            size_t j = i;
            if (q[j] == '+' || q[j] == '-') j++;
            while (j < n && is_digit(q[j])) j++;
            Type type = Type::INTEGER;
            if (j + 1 < n && q[j] == '.' && is_digit(q[j + 1])) {
                j++;
                while (j < n && is_digit(q[j])) j++;
                type = Type::DECIMAL;
            }
            if (j < n && (q[j] == 'e' || q[j] == 'E')) {
                size_t k = j + 1;
                if (k < n && (q[k] == '+' || q[k] == '-')) k++;
                if (k < n && is_digit(q[k])) {
                    j = k;
                    while (j < n && is_digit(q[j])) j++;
                    type = Type::DOUBLE;
                }
            }
            push(type, q.substr(i, j - i), start, j);
            i = j;
            continue;
        }

        if ((c == '&' && next == '&') || (c == '|' && next == '|') || (c == '!' && next == '=') || (c == '^' && next == '^')) {
            push(Type::PUNCT, q.substr(i, 2), start, i + 2);
            i += 2;
            continue;
        }
        if (c == '!' || c == '=' || c == '(' || c == ')' || c == '{' || c == '}' || c == '.' || c == ';' || c == ',' || c == '*') {
            push(Type::PUNCT, std::string(1, c), start, i + 1);
            i++;
            continue;
        }

        if (is_name_start(c) || c == ':') {
            size_t j = i;
            while (j < n && (is_name_char(q[j]) || q[j] == '.' || q[j] == ':')) j++;
            // A name may not end with '.', which terminates the triple instead.
            while (j > i && q[j - 1] == '.') j--;
            std::string word = q.substr(i, j - i);
            push(word.find(':') != std::string::npos ? Type::PNAME : Type::WORD, word, start, j);
            i = j;
            continue;
        }

        error_at(start, std::string("unexpected character '") + c + "'");
    }
    push(Type::EOI, "", n, n);
    return out;
}

// --- Parser ---

SparqlSelectStatement SparqlParser::parse(const std::string& query_string) {
    LOG_INFO("Parsing SPARQL query: " + query_string);
    query_ = query_string;
    tokens = tokenize(query_string);
    pos = 0;
    stmt = SparqlSelectStatement{};

    parse_prologue();
    if (peek_keyword("CONSTRUCT") || peek_keyword("ASK") || peek_keyword("DESCRIBE")) {
        fail_here(to_upper(peek().value) + " queries are not supported; only SELECT queries are");
    }
    if (!peek_keyword("SELECT")) fail("expected SELECT after the PREFIX declarations");
    parse_select_clause();
    if (peek_keyword("WHERE")) consume();
    if (!peek_punct("{")) fail("expected '{' to start the WHERE clause");
    parse_group_graph_pattern(stmt.where);
    parse_solution_modifiers();
    if (peek().type != SparqlToken::Type::EOI) fail_here("unexpected " + describe(peek()) + " after the end of the query");

    LOG_DEBUG("Successfully parsed SPARQL SELECT statement.");
    return stmt;
}

void SparqlParser::parse_prologue() {
    while (true) {
        if (peek_keyword("PREFIX")) {
            consume();
            const SparqlToken& name = peek();
            if (name.type == SparqlToken::Type::WORD) fail("prefix label must end with ':', e.g. '" + name.value + ":'");
            if (name.type != SparqlToken::Type::PNAME) fail("expected a prefix label such as 'ex:' after PREFIX");
            if (name.value.find(':') != name.value.size() - 1) fail("prefix label must end with ':' and have no local name");
            const std::string label = consume().value;
            if (peek().type != SparqlToken::Type::IRI) fail("expected an IRI in angle brackets after '" + label + "'");
            stmt.prefixes[label.substr(0, label.size() - 1)] = resolve_iri(consume().value);
        } else if (peek_keyword("BASE")) {
            consume();
            if (peek().type != SparqlToken::Type::IRI) fail("expected an IRI in angle brackets after BASE");
            stmt.base = resolve_iri(consume().value);
        } else {
            break;
        }
    }
}

void SparqlParser::parse_select_clause() {
    expect_keyword("SELECT");
    if (peek_keyword("DISTINCT")) {
        consume();
        stmt.distinct = true;
    } else if (peek_keyword("REDUCED")) {
        consume(); // REDUCED permits, but does not require, removing duplicates.
    }

    if (peek_punct("*")) {
        consume();
        stmt.select_all = true;
        return;
    }
    while (peek().type == SparqlToken::Type::VAR) {
        stmt.variables.push_back(consume().value);
    }
    if (peek_punct("(")) fail("expressions in SELECT (such as aggregates or AS) are not supported; list variables or use '*'");
    if (stmt.variables.empty()) fail("expected variables or '*' after SELECT");
}

void SparqlParser::parse_group_graph_pattern(SparqlGroupPattern& group) {
    const size_t open_offset = peek().offset;
    expect_punct("{");
    while (!peek_punct("}")) {
        if (peek().type == SparqlToken::Type::EOI) {
            error_at(peek().offset, "missing '}' to close the '{' at " + position_of(open_offset));
        }
        reject_unsupported_construct();
        if (peek_keyword("FILTER")) {
            group.filters.push_back(parse_filter());
            if (peek_punct(".")) consume();
            continue;
        }
        if (peek_keyword("OPTIONAL")) {
            consume();
            if (!peek_punct("{")) fail("expected '{' after OPTIONAL");
            SparqlGroupElement element;
            element.kind = SparqlGroupElement::Kind::OPTIONAL;
            element.group = std::make_shared<SparqlGroupPattern>();
            parse_group_graph_pattern(*element.group);
            group.elements.push_back(std::move(element));
            if (peek_punct(".")) consume();
            continue;
        }
        if (peek().type == SparqlToken::Type::PUNCT && peek().value != "(") {
            fail("expected a triple pattern, FILTER, OPTIONAL or '}'");
        }
        // Consecutive triple patterns (even with FILTERs between them) form one basic graph pattern.
        if (group.elements.empty() || group.elements.back().kind != SparqlGroupElement::Kind::TRIPLES) {
            group.elements.emplace_back();
        }
        parse_triples_same_subject(group.elements.back().triples);
        if (peek_punct(".")) {
            consume();
        } else if (!at_group_element_end()) {
            if (peek().type != SparqlToken::Type::EOI) reject_unsupported_construct();
            if (peek().type == SparqlToken::Type::EOI) {
                error_at(peek().offset, "missing '}' to close the '{' at " + position_of(open_offset));
            }
            fail("expected '.' or '}' after a triple pattern");
        }
    }
    consume(); // '}'
}

void SparqlParser::reject_unsupported_construct() const {
    if (peek_punct("{")) fail_here("nested group patterns are not supported; only OPTIONAL { ... } may contain a group");
    if (peek().type == SparqlToken::Type::WORD && unsupported_keywords().count(to_upper(peek().value))) {
        fail_here(to_upper(peek().value) + " is not supported");
    }
}

bool SparqlParser::at_group_element_end() const {
    return peek_punct(".") || peek_punct("}") || peek_keyword("FILTER") || peek_keyword("OPTIONAL");
}

void SparqlParser::parse_triples_same_subject(std::vector<SparqlTriplePattern>& out) {
    const size_t subject_index = pos;
    SparqlPatternTerm subject = parse_var_or_term();
    if (auto* term = std::get_if<Rdf::Term>(&subject); term && term->is_literal()) {
        fail_at(subject_index, "a literal cannot be the subject of a triple pattern");
    }

    while (true) {
        if (at_group_element_end() || peek().type == SparqlToken::Type::EOI) {
            fail("triple pattern is missing its predicate and object");
        }
        SparqlPatternTerm verb = parse_verb();
        while (true) {
            if (at_group_element_end() || peek_punct(";") || peek_punct(",") || peek().type == SparqlToken::Type::EOI) {
                fail("triple pattern is missing its object");
            }
            out.push_back({subject, verb, parse_var_or_term()});
            if (!peek_punct(",")) break;
            consume();
        }
        if (!peek_punct(";")) break;
        while (peek_punct(";")) consume();
        if (at_group_element_end()) break;
    }
}

SparqlPatternTerm SparqlParser::parse_verb() {
    if (peek().type == SparqlToken::Type::WORD && peek().value == "a") {
        consume();
        return Rdf::Term::iri(Rdf::Vocab::RDF_TYPE);
    }
    const size_t verb_index = pos;
    SparqlPatternTerm verb = parse_var_or_term();
    if (auto* term = std::get_if<Rdf::Term>(&verb); term && !term->is_iri()) {
        fail_at(verb_index, "a predicate must be an IRI or a variable");
    }
    return verb;
}

SparqlPatternTerm SparqlParser::parse_var_or_term() {
    const SparqlToken& tok = peek();
    switch (tok.type) {
        case SparqlToken::Type::VAR:
            return SparqlVariable{consume().value};
        case SparqlToken::Type::BLANK:
            return SparqlVariable{"_:" + consume().value};
        case SparqlToken::Type::IRI:
        case SparqlToken::Type::PNAME:
            return parse_iri();
        case SparqlToken::Type::STRING:
        case SparqlToken::Type::INTEGER:
        case SparqlToken::Type::DECIMAL:
        case SparqlToken::Type::DOUBLE:
            return parse_literal();
        case SparqlToken::Type::WORD: {
            const std::string upper = to_upper(tok.value);
            if (upper == "TRUE" || upper == "FALSE") return parse_literal();
            if (unsupported_keywords().count(upper)) fail_here(upper + " is not supported");
            break;
        }
        default:
            break;
    }
    fail("expected a variable, IRI or literal");
}

Rdf::Term SparqlParser::parse_iri() {
    const SparqlToken& tok = peek();
    if (tok.type == SparqlToken::Type::IRI) return Rdf::Term::iri(resolve_iri(consume().value));
    if (tok.type == SparqlToken::Type::PNAME) {
        std::string iri = expand_prefixed_name(tok);
        consume();
        return Rdf::Term::iri(iri);
    }
    fail("expected an IRI");
}

Rdf::Term SparqlParser::parse_literal() {
    SparqlToken tok = consume();
    switch (tok.type) {
        case SparqlToken::Type::INTEGER:
            return Rdf::Term::literal(tok.value, Rdf::XSD::INTEGER);
        case SparqlToken::Type::DECIMAL:
            return Rdf::Term::literal(tok.value, Rdf::XSD::DECIMAL);
        case SparqlToken::Type::DOUBLE:
            return Rdf::Term::literal(tok.value, Rdf::XSD::DOUBLE);
        case SparqlToken::Type::STRING:
            if (peek().type == SparqlToken::Type::LANGTAG) {
                return Rdf::Term::lang_literal(tok.value, consume().value);
            }
            if (peek_punct("^^")) {
                consume();
                return Rdf::Term::literal(tok.value, parse_iri().value);
            }
            return Rdf::Term::literal(tok.value);
        case SparqlToken::Type::WORD: {
            const std::string upper = to_upper(tok.value);
            if (upper == "TRUE") return Rdf::Term::literal("true", Rdf::XSD::BOOLEAN);
            if (upper == "FALSE") return Rdf::Term::literal("false", Rdf::XSD::BOOLEAN);
            break;
        }
        default:
            break;
    }
    fail_at(pos - 1, "expected a literal");
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_filter() {
    expect_keyword("FILTER");
    std::shared_ptr<SparqlExpression> expr;
    if (peek_punct("(")) {
        consume();
        expr = parse_expression();
        expect_punct(")");
    } else if (peek().type == SparqlToken::Type::WORD) {
        expr = parse_function_call();
    } else {
        fail("expected '(' or a function call after FILTER");
    }
    return expr;
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_expression() {
    auto left = parse_and_expression();
    while (peek_punct("||")) {
        consume();
        left = make_binary("||", left, parse_and_expression());
    }
    return left;
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_and_expression() {
    auto left = parse_relational_expression();
    while (peek_punct("&&")) {
        consume();
        left = make_binary("&&", left, parse_relational_expression());
    }
    return left;
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_relational_expression() {
    auto left = parse_unary_expression();
    static const std::set<std::string> ops = {"=", "!=", "<", ">", "<=", ">="};
    if (peek().type == SparqlToken::Type::PUNCT && ops.count(peek().value)) {
        std::string op = consume().value;
        return make_binary(op, left, parse_unary_expression());
    }
    return left;
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_unary_expression() {
    if (peek_punct("!")) {
        consume();
        auto e = std::make_shared<SparqlExpression>();
        e->kind = SparqlExpression::Kind::UNARY;
        e->op = "!";
        e->args = {parse_unary_expression()};
        return e;
    }
    return parse_primary_expression();
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_primary_expression() {
    const SparqlToken& tok = peek();
    if (tok.type == SparqlToken::Type::PUNCT && tok.value == "(") {
        consume();
        auto e = parse_expression();
        expect_punct(")");
        return e;
    }
    if (tok.type == SparqlToken::Type::VAR) {
        auto e = std::make_shared<SparqlExpression>();
        e->kind = SparqlExpression::Kind::VARIABLE;
        e->variable = consume().value;
        return e;
    }
    if (tok.type == SparqlToken::Type::IRI || tok.type == SparqlToken::Type::PNAME) {
        return make_term(parse_iri());
    }
    if (tok.type == SparqlToken::Type::WORD) {
        const std::string upper = to_upper(tok.value);
        if (upper == "TRUE" || upper == "FALSE") return make_term(parse_literal());
        return parse_function_call();
    }
    if (tok.type == SparqlToken::Type::STRING || tok.type == SparqlToken::Type::INTEGER ||
        tok.type == SparqlToken::Type::DECIMAL || tok.type == SparqlToken::Type::DOUBLE) {
        return make_term(parse_literal());
    }
    fail("expected an expression");
}

std::shared_ptr<SparqlExpression> SparqlParser::parse_function_call() {
    // Built-in functions and their accepted argument counts.
    static const std::map<std::string, std::pair<size_t, size_t>> arity = {
        {"BOUND", {1, 1}},    {"STR", {1, 1}},       {"LANG", {1, 1}},      {"DATATYPE", {1, 1}},
        {"ISIRI", {1, 1}},    {"ISURI", {1, 1}},     {"ISBLANK", {1, 1}},   {"ISLITERAL", {1, 1}},
        {"ISNUMERIC", {1, 1}}, {"REGEX", {2, 3}},    {"CONTAINS", {2, 2}},  {"STRSTARTS", {2, 2}},
        {"STRENDS", {2, 2}},  {"SAMETERM", {2, 2}},  {"LANGMATCHES", {2, 2}}};

    const size_t name_index = pos;
    const std::string name = to_upper(peek().value);
    auto it = arity.find(name);
    if (peek().type != SparqlToken::Type::WORD || it == arity.end()) fail_here("unknown function '" + peek().text + "'");
    consume();

    auto e = std::make_shared<SparqlExpression>();
    e->kind = SparqlExpression::Kind::FUNCTION;
    e->op = name;
    expect_punct("(");
    if (!peek_punct(")")) {
        while (true) {
            e->args.push_back(parse_expression());
            if (!peek_punct(",")) break;
            consume();
        }
    }
    expect_punct(")");

    const auto [min_args, max_args] = it->second;
    if (e->args.size() < min_args || e->args.size() > max_args) {
        std::string expected = std::to_string(min_args) + (max_args != min_args ? " or " + std::to_string(max_args) : "");
        error_at(tokens[name_index].offset, name + " expects " + expected + " argument" + (max_args == 1 ? "" : "s") +
                                                " but got " + std::to_string(e->args.size()));
    }
    if (name == "BOUND" && e->args[0]->kind != SparqlExpression::Kind::VARIABLE) {
        error_at(tokens[name_index].offset, "BOUND expects a variable argument");
    }
    return e;
}

void SparqlParser::parse_solution_modifiers() {
    if (peek_keyword("ORDER")) {
        consume();
        expect_keyword("BY");
        while (true) {
            if (peek().type == SparqlToken::Type::VAR) {
                stmt.order_by.push_back({consume().value, false});
            } else if (peek_keyword("ASC") || peek_keyword("DESC")) {
                bool descending = to_upper(consume().value) == "DESC";
                expect_punct("(");
                if (peek().type != SparqlToken::Type::VAR) fail("ORDER BY supports only variables, e.g. DESC(?x)");
                stmt.order_by.push_back({consume().value, descending});
                expect_punct(")");
            } else {
                break;
            }
        }
        if (stmt.order_by.empty()) fail("expected a variable, ASC(?var) or DESC(?var) after ORDER BY");
    }

    while (true) {
        if (peek_keyword("GROUP") || peek_keyword("HAVING")) {
            fail_here(to_upper(peek().value) + (peek_keyword("GROUP") ? " BY" : "") + " is not supported");
        } else if (peek_keyword("LIMIT")) {
            if (stmt.limit) fail_here("LIMIT may only be given once");
            consume();
            stmt.limit = parse_non_negative_integer("LIMIT");
        } else if (peek_keyword("OFFSET")) {
            if (stmt.offset) fail_here("OFFSET may only be given once");
            consume();
            stmt.offset = parse_non_negative_integer("OFFSET");
        } else {
            break;
        }
    }
}

size_t SparqlParser::parse_non_negative_integer(const std::string& clause) {
    const SparqlToken& tok = peek();
    if (tok.type != SparqlToken::Type::INTEGER || tok.value[0] == '-' || tok.value[0] == '+') {
        fail(clause + " expects a non-negative integer");
    }
    return static_cast<size_t>(std::stoull(consume().value));
}

std::string SparqlParser::resolve_iri(const std::string& iri) const {
    // Relative IRIs are resolved by appending them to BASE, which covers the
    // common "BASE <http://example.org/> ... <alice>" style.
    if (stmt.base.empty() || has_scheme(iri)) return iri;
    return stmt.base + iri;
}

std::string SparqlParser::expand_prefixed_name(const SparqlToken& token) const {
    size_t colon = token.value.find(':');
    std::string prefix = token.value.substr(0, colon);
    auto it = stmt.prefixes.find(prefix);
    if (it == stmt.prefixes.end()) {
        error_at(token.offset, "undeclared prefix '" + prefix + ":'; declare it with PREFIX " + prefix + ": <namespace-iri>");
    }
    return it->second + token.value.substr(colon + 1);
}

const SparqlToken& SparqlParser::peek() const {
    return tokens[pos];
}

SparqlToken SparqlParser::consume() {
    SparqlToken tok = tokens[pos];
    if (pos + 1 < tokens.size()) pos++;
    return tok;
}

bool SparqlParser::peek_punct(const std::string& value) const {
    return peek().type == SparqlToken::Type::PUNCT && peek().value == value;
}

bool SparqlParser::peek_keyword(const std::string& keyword) const {
    return peek().type == SparqlToken::Type::WORD && to_upper(peek().value) == keyword;
}

void SparqlParser::expect_punct(const std::string& value) {
    if (!peek_punct(value)) fail("expected '" + value + "'");
    consume();
}

void SparqlParser::expect_keyword(const std::string& keyword) {
    if (!peek_keyword(keyword)) fail("expected " + keyword);
    consume();
}

void SparqlParser::fail(const std::string& message) const {
    fail_at(pos, message);
}

void SparqlParser::fail_here(const std::string& message) const {
    error_at(peek().offset, message);
}

void SparqlParser::fail_at(size_t token_index, const std::string& message) const {
    const SparqlToken& tok = tokens[token_index];
    const std::string where = tok.type == SparqlToken::Type::EOI ? " at end of query" : " near " + describe(tok);
    error_at(tok.offset, message + where);
}

void SparqlParser::error_at(size_t offset, const std::string& message) const {
    const auto [line, column] = line_and_column(offset);
    throw SparqlSyntaxError(message, line, column);
}

std::pair<size_t, size_t> SparqlParser::line_and_column(size_t offset) const {
    size_t line = 1;
    size_t line_start = 0;
    for (size_t i = 0; i < offset && i < query_.size(); ++i) {
        if (query_[i] == '\n') {
            line++;
            line_start = i + 1;
        }
    }
    return {line, offset - line_start + 1};
}

std::string SparqlParser::position_of(size_t offset) const {
    const auto [line, column] = line_and_column(offset);
    return "line " + std::to_string(line) + ", column " + std::to_string(column);
}

std::string SparqlParser::describe(const SparqlToken& token) const {
    if (token.type == SparqlToken::Type::EOI) return "end of query";
    const size_t max_length = 40;
    std::string text = token.text.size() > max_length ? token.text.substr(0, max_length) + "..." : token.text;
    return "'" + text + "'";
}

SparqlSyntaxError::SparqlSyntaxError(const std::string& detail, size_t line, size_t column)
    : std::runtime_error("SPARQL syntax error at line " + std::to_string(line) + ", column " + std::to_string(column) +
                         ": " + detail),
      detail_(detail),
      line_(line),
      column_(column) {}

} // namespace Query
} // namespace TissDB
