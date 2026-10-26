#include "json.h"
#include <cstdio>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <vector>

namespace TissDB {
namespace Json {

namespace {

// Writes s as a quoted JSON string, escaping quotes, backslashes and control characters.
void write_escaped_string(std::stringstream& ss, const std::string& s) {
    ss << '"';
    for (char ch : s) {
        switch (ch) {
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(ch));
                    ss << buf;
                } else {
                    ss << ch;
                }
        }
    }
    ss << '"';
}

} // anonymous namespace

// --- Tokenizer ---

enum class TokenType {
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    COMMA, COLON,
    STRING, NUMBER, BOOLEAN, JSON_NULL,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
public:
    Tokenizer(const std::string& input) : text(input), pos(0) {}
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < text.length()) {
            char c = text[pos];
            if (std::isspace(c)) { pos++; continue; }
            switch (c) {
                case '{': tokens.push_back({TokenType::LEFT_BRACE, "{"}); pos++; break;
                case '}': tokens.push_back({TokenType::RIGHT_BRACE, "}"}); pos++; break;
                case '[': tokens.push_back({TokenType::LEFT_BRACKET, "["}); pos++; break;
                case ']': tokens.push_back({TokenType::RIGHT_BRACKET, "]"}); pos++; break;
                case ',': tokens.push_back({TokenType::COMMA, ","}); pos++; break;
                case ':': tokens.push_back({TokenType::COLON, ":"}); pos++; break;
                case '"': tokens.push_back({TokenType::STRING, parse_string()}); break;
                default:
                    if (c == 't' || c == 'f') tokens.push_back({TokenType::BOOLEAN, parse_literal("true", "false")});
                    else if (c == 'n') tokens.push_back({TokenType::JSON_NULL, parse_literal("null", "")});
                    else if (std::isdigit(c) || c == '-') tokens.push_back({TokenType::NUMBER, parse_number()});
                    else throw std::runtime_error("Tokenizer error: Unexpected character.");
            }
        }
        tokens.push_back({TokenType::END_OF_FILE, ""});
        return tokens;
    }
private:
    std::string text; size_t pos;
    std::string parse_string() {
        pos++; std::stringstream ss;
        while (pos < text.length() && text[pos] != '"') {
            if (text[pos] == '\\') {
                pos++;
                if (pos >= text.length()) throw std::runtime_error("Unterminated string.");
                switch (text[pos]) {
                    case '"': ss << '"'; break; case '\\': ss << '\\'; break;
                    case '/': ss << '/'; break; case 'b': ss << '\b'; break;
                    case 'f': ss << '\f'; break; case 'n': ss << '\n'; break;
                    case 'r': ss << '\r'; break; case 't': ss << '\t'; break;
                    case 'u': ss << parse_unicode_escape(); break;
                    default: throw std::runtime_error("Invalid escape sequence.");
                }
            } else { ss << text[pos]; }
            pos++;
        }
        if (pos >= text.length()) throw std::runtime_error("Unterminated string.");
        pos++; return ss.str();
    }
    // Reads the XXXX of a \\uXXXX escape (pos is on the 'u'), combining UTF-16
    // surrogate pairs, and returns the code point encoded as UTF-8.
    std::string parse_unicode_escape() {
        auto read_hex4 = [this]() {
            if (pos + 4 >= text.length()) throw std::runtime_error("Invalid unicode escape.");
            unsigned int value = 0;
            for (int i = 1; i <= 4; ++i) {
                char h = text[pos + i];
                if (!std::isxdigit(static_cast<unsigned char>(h))) throw std::runtime_error("Invalid unicode escape.");
                value = value * 16 + (std::isdigit(static_cast<unsigned char>(h)) ? h - '0' : (std::tolower(static_cast<unsigned char>(h)) - 'a' + 10));
            }
            pos += 4;
            return value;
        };
        unsigned int cp = read_hex4();
        if (cp >= 0xD800 && cp <= 0xDBFF && pos + 2 < text.length() && text[pos + 1] == '\\' && text[pos + 2] == 'u') {
            pos += 2;
            unsigned int low = read_hex4();
            if (low < 0xDC00 || low > 0xDFFF) throw std::runtime_error("Invalid unicode surrogate pair.");
            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
        }
        std::string out;
        if (cp < 0x80) {
            out += static_cast<char>(cp);
        } else if (cp < 0x800) {
            out += static_cast<char>(0xC0 | (cp >> 6));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            out += static_cast<char>(0xE0 | (cp >> 12));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (cp >> 18));
            out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
        return out;
    }
    std::string parse_number() {
        std::stringstream ss;
        while (pos < text.length() && (std::isdigit(text[pos]) || text[pos] == '.' || text[pos] == '-' || text[pos] == 'e' || text[pos] == 'E' || text[pos] == '+')) {
            ss << text[pos++];
        }
        return ss.str();
    }
    std::string parse_literal(const std::string& v1, const std::string& v2) {
        if (text.substr(pos, v1.length()) == v1) { pos += v1.length(); return v1; }
        if (!v2.empty() && text.substr(pos, v2.length()) == v2) { pos += v2.length(); return v2; }
        throw std::runtime_error("Invalid literal.");
    }
};

// --- Parser ---

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {}
    JsonValue parse() {
        JsonValue result = parse_value();
        consume(TokenType::END_OF_FILE, "Expected end of input.");
        return result;
    }
private:
    const std::vector<Token>& tokens_; size_t pos_;
    const Token& peek() const { return tokens_[pos_]; }
    Token consume() { return tokens_[pos_++]; }
    Token consume(TokenType type, const std::string& msg) {
        Token t = consume();
        if (t.type != type) throw std::runtime_error(msg);
        return t;
    }
    JsonValue parse_value() {
        switch (peek().type) {
            case TokenType::LEFT_BRACE: return parse_object();
            case TokenType::LEFT_BRACKET: return parse_array();
            case TokenType::STRING: return JsonValue(consume().value);
            case TokenType::NUMBER: return JsonValue(std::stod(consume().value));
            case TokenType::BOOLEAN: return JsonValue(consume().value == "true");
            case TokenType::JSON_NULL: consume(); return JsonValue(nullptr);
            default: throw std::runtime_error("Unexpected token when parsing value.");
        }
    }
    JsonValue parse_object() {
        consume(TokenType::LEFT_BRACE, "Expected '{'.");
        JsonObject obj;
        if (peek().type != TokenType::RIGHT_BRACE) {
            while (true) {
                Token key = consume(TokenType::STRING, "Expected string key.");
                consume(TokenType::COLON, "Expected ':'.");
                obj[key.value] = parse_value();
                if (peek().type == TokenType::RIGHT_BRACE) break;
                consume(TokenType::COMMA, "Expected ','.");
            }
        }
        consume(TokenType::RIGHT_BRACE, "Expected '}'.");
        return JsonValue(obj);
    }
    JsonValue parse_array() {
        consume(TokenType::LEFT_BRACKET, "Expected '['.");
        JsonArray arr;
        if (peek().type != TokenType::RIGHT_BRACKET) {
            while (true) {
                arr.push_back(parse_value());
                if (peek().type == TokenType::RIGHT_BRACKET) break;
                consume(TokenType::COMMA, "Expected ','.");
            }
        }
        consume(TokenType::RIGHT_BRACKET, "Expected ']'.");
        return JsonValue(arr);
    }
};

// --- JsonValue::parse method ---

JsonValue JsonValue::parse(const std::string& json_string) {
    if (json_string.empty()) throw std::runtime_error("Cannot parse empty string.");
    Tokenizer tokenizer(json_string);
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

// --- JsonValue::serialize method ---

std::string JsonValue::serialize() const {
    std::stringstream ss;
    std::visit(
        [&ss](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, JsonNull>) {
                ss << "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                ss << (arg ? "true" : "false");
            } else if constexpr (std::is_same_v<T, double>) {
                ss << arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                write_escaped_string(ss, arg);
            } else if constexpr (std::is_same_v<T, JsonArray>) {
                ss << '[';
                bool first = true;
                for (const auto& val : arg) {
                    if (!first) ss << ',';
                    ss << val.serialize();
                    first = false;
                }
                ss << ']';
            } else if constexpr (std::is_same_v<T, JsonObject>) {
                ss << '{';
                bool first = true;
                for (const auto& pair : arg) {
                    if (!first) ss << ',';
                    write_escaped_string(ss, pair.first);
                    ss << ':' << pair.second.serialize();
                    first = false;
                }
                ss << '}';
            }
        },
        value_);
    return ss.str();
}

} // namespace Json
} // namespace TissDB
