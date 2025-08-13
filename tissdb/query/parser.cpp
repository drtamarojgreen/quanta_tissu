#include "parser.h"
#include <stdexcept>
#include <cctype>

namespace TissDB {
namespace Query {

// --- Tokenizer ---

std::vector<Token> Parser::tokenize(const std::string& query_string) {
    std::vector<Token> new_tokens;
    for (size_t i = 0; i < query_string.length(); ++i) {
        if (std::isspace(query_string[i])) continue;

        if (std::isalpha(query_string[i]) || query_string[i] == '_') {
            size_t start = i;
            while (i + 1 < query_string.length() && (std::isalnum(query_string[i + 1]) || query_string[i + 1] == '_')) {
                i++;
            }
            std::string value = query_string.substr(start, i - start + 1);
            if (value == "SELECT" || value == "FROM" || value == "WHERE" || value == "AND" || value == "OR") {
                new_tokens.push_back({Token::Type::KEYWORD, value});
            } else {
                new_tokens.push_back({Token::Type::IDENTIFIER, value});
            }
        } else if (std::isdigit(query_string[i])) {
            size_t start = i;
            while (i + 1 < query_string.length() && (std::isdigit(query_string[i + 1]) || query_string[i + 1] == '.')) {
                i++;
            }
            new_tokens.push_back({Token::Type::LITERAL, query_string.substr(start, i - start + 1)});
        } else if (query_string[i] == '\'') {
            size_t start = ++i;
            while (i < query_string.length() && query_string[i] != '\'') {
                i++;
            }
            new_tokens.push_back({Token::Type::LITERAL, query_string.substr(start, i - start)});
        } else if (query_string[i] == '=' || query_string[i] == '!' || query_string[i] == '<' || query_string[i] == '>') {
            size_t start = i;
            if (i + 1 < query_string.length() && query_string[i + 1] == '=') {
                i++;
            }
            new_tokens.push_back({Token::Type::OPERATOR, query_string.substr(start, i - start + 1)});
        } else if (query_string[i] == ',') {
            new_tokens.push_back({Token::Type::OPERATOR, ","});
        } else if (query_string[i] == '*') {
            new_tokens.push_back({Token::Type::OPERATOR, "*"});
        }
    }
    new_tokens.push_back({Token::Type::EOI, ""});
    return new_tokens;
}

// --- Parser ---

Parser::Parser() = default;

AST Parser::parse(const std::string& query_string) {
    tokens = tokenize(query_string);
    pos = 0;
    return parse_select_statement();
}

SelectStatement Parser::parse_select_statement() {
    expect(Token::Type::KEYWORD, "SELECT");
    auto fields = parse_select_list();
    expect(Token::Type::KEYWORD, "FROM");
    auto table = parse_table_name();
    auto where = parse_where_clause();
    return {fields, table, where};
}

std::vector<std::string> Parser::parse_select_list() {
    std::vector<std::string> fields;
    if (peek().type == Token::Type::OPERATOR && peek().value == "*") {
        consume();
        fields.push_back("*");
        return fields;
    }

    do {
        fields.push_back(consume().value);
        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return fields;
}

std::string Parser::parse_table_name() {
    return consume().value;
}

std::optional<Expression> Parser::parse_where_clause() {
    if (peek().type == Token::Type::KEYWORD && peek().value == "WHERE") {
        consume();
        return parse_expression();
    }
    return std::nullopt;
}

Expression Parser::parse_expression(int precedence) {
    auto left = parse_primary_expression();

    while (true) {
        auto op = peek().value;
        int new_precedence = 0;
        if (op == "AND" || op == "OR") new_precedence = 1;
        if (op == "=" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") new_precedence = 2;

        if (new_precedence <= precedence) break;

        consume();
        auto right = parse_expression(new_precedence);
        if (op == "AND" || op == "OR") {
            left = LogicalExpression{std::make_unique<Expression>(std::move(left)), op, std::make_unique<Expression>(std::move(right))};
        } else {
            left = BinaryExpression{std::make_unique<Expression>(std::move(left)), op, std::make_unique<Expression>(std::move(right))};
        }
    }

    return left;
}

Expression Parser::parse_primary_expression() {
    auto token = consume();
    if (token.type == Token::Type::IDENTIFIER) {
        return Identifier{token.value};
    } else if (token.type == Token::Type::LITERAL) {
        if (token.value.find('.') != std::string::npos) {
            return Literal{std::stod(token.value)};
        } else {
            return Literal{token.value};
        }
    }
    throw std::runtime_error("Unexpected token in expression");
}

// --- Helper methods ---

Token Parser::peek() {
    return tokens[pos];
}

Token Parser::consume() {
    return tokens[pos++];
}

void Parser::expect(Token::Type type, const std::string& value) {
    auto token = consume();
    if (token.type != type || (!value.empty() && token.value != value)) {
        throw std::runtime_error("Unexpected token");
    }
}

} // namespace Query
} // namespace TissDB