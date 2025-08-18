#include "parser.h"
#include "../common/log.h"
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
            if (value == "SELECT" || value == "FROM" || value == "WHERE" || value == "AND" || value == "OR" || value == "UPDATE" || value == "DELETE" || value == "SET" || value == "GROUP" || value == "BY" || value == "COUNT" || value == "AVG" || value == "SUM" || value == "MIN" || value == "MAX" || value == "INSERT" || value == "INTO" || value == "VALUES" || value == "STDDEV" || value == "LIKE" || value == "ORDER" || value == "LIMIT" || value == "JOIN" || value == "ON" || value == "UNION" || value == "ALL" || value == "ASC" || value == "DESC") {
                new_tokens.push_back(Token{Token::Type::KEYWORD, value});
            } else {
                new_tokens.push_back(Token{Token::Type::IDENTIFIER, value});
            }
        } else if (std::isdigit(query_string[i])) {
            size_t start = i;
            while (i + 1 < query_string.length() && (std::isdigit(query_string[i + 1]) || query_string[i + 1] == '.')) {
                i++;
            }
            new_tokens.push_back(Token{Token::Type::NUMERIC_LITERAL, query_string.substr(start, i - start + 1)});
        } else if (query_string[i] == '\'') {
            size_t start = ++i;
            while (i < query_string.length() && query_string[i] != '\'') {
                i++;
            }
            new_tokens.push_back(Token{Token::Type::STRING_LITERAL, query_string.substr(start, i - start)});
        } else if (query_string[i] == '=' || query_string[i] == '!' || query_string[i] == '<' || query_string[i] == '>') {
            size_t start = i;
            if (i + 1 < query_string.length() && query_string[i + 1] == '=') {
                i++;
            }
            new_tokens.push_back(Token{Token::Type::OPERATOR, query_string.substr(start, i - start + 1)});
        } else if (query_string[i] == ',' || query_string[i] == '(' || query_string[i] == ')') {
            new_tokens.push_back(Token{Token::Type::OPERATOR, std::string(1, query_string[i])});
        } else if (query_string[i] == '*') {
            new_tokens.push_back(Token{Token::Type::OPERATOR, "*"});
        }
    }
    new_tokens.push_back(Token{Token::Type::EOI, ""});
    return new_tokens;
}


// --- Parser ---

Parser::Parser() = default;

AST Parser::parse(const std::string& query_string) {
    LOG_INFO("Parsing query: " + query_string);
    tokens = tokenize(query_string);
    pos = 0;

    if (peek().type == Token::Type::KEYWORD) {
        if (peek().value == "SELECT") {
            auto ast = parse_select_statement();
            LOG_DEBUG("Successfully parsed SELECT statement.");
            return ast;
        } else if (peek().value == "UPDATE") {
            auto ast = parse_update_statement();
            LOG_DEBUG("Successfully parsed UPDATE statement.");
            return ast;
        } else if (peek().value == "DELETE") {
            auto ast = parse_delete_statement();
            LOG_DEBUG("Successfully parsed DELETE statement.");
            return ast;
        } else if (peek().value == "INSERT") {
            auto ast = parse_insert_statement();
            LOG_DEBUG("Successfully parsed INSERT statement.");
            return ast;
        }
    }

    LOG_ERROR("Unsupported statement type at start of query.");
    throw std::runtime_error("Unsupported statement type");
}

SelectStatement Parser::parse_select_statement() {
    expect(Token::Type::KEYWORD, "SELECT");
    auto fields = parse_select_list();
    expect(Token::Type::KEYWORD, "FROM");
    auto table = parse_table_name();
    auto where = parse_where_clause();
    auto group_by = parse_group_by_clause();
    return {fields, table, std::move(where), group_by};
}

UpdateStatement Parser::parse_update_statement() {
    expect(Token::Type::KEYWORD, "UPDATE");
    auto table = parse_table_name();
    expect(Token::Type::KEYWORD, "SET");
    auto set = parse_set_clause();
    auto where = parse_where_clause();
    return {table, set, std::move(where)};
}

DeleteStatement Parser::parse_delete_statement() {
    expect(Token::Type::KEYWORD, "DELETE");
    expect(Token::Type::KEYWORD, "FROM");
    auto table = parse_table_name();
    auto where = parse_where_clause();
    return {table, std::move(where)};
}

InsertStatement Parser::parse_insert_statement() {
    expect(Token::Type::KEYWORD, "INSERT");
    expect(Token::Type::KEYWORD, "INTO");
    auto table = parse_table_name();
    std::vector<std::string> columns;
    if (peek().type == Token::Type::OPERATOR && peek().value == "(") {
        consume();
        columns = parse_column_list();
        expect(Token::Type::OPERATOR, ")");
    }
    expect(Token::Type::KEYWORD, "VALUES");
    expect(Token::Type::OPERATOR, "(");
    auto values = parse_value_list();
    expect(Token::Type::OPERATOR, ")");
    return {table, columns, values};
}

std::vector<std::variant<std::string, AggregateFunction>> Parser::parse_select_list() {
    std::vector<std::variant<std::string, AggregateFunction>> fields;
    if (peek().type == Token::Type::OPERATOR && peek().value == "*") {
        consume();
        fields.push_back("*");
        return fields;
    }

    do {
        if (peek().type == Token::Type::KEYWORD && (peek().value == "COUNT" || peek().value == "AVG" || peek().value == "SUM" || peek().value == "MIN" || peek().value == "MAX" || peek().value == "STDDEV")) {
            std::string func_name = consume().value;
            expect(Token::Type::OPERATOR, "(");
            std::string field_name = consume().value;
            expect(Token::Type::OPERATOR, ")");
            fields.push_back(AggregateFunction{func_name, field_name});
        } else {
            fields.push_back(consume().value);
        }

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

std::vector<std::string> Parser::parse_column_list() {
    std::vector<std::string> columns;
    do {
        columns.push_back(consume().value);
        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return columns;
}

std::vector<Literal> Parser::parse_value_list() {
    std::vector<Literal> values;
    do {
        auto token = consume();
        if (token.type == Token::Type::NUMERIC_LITERAL) {
            values.push_back(std::stod(token.value));
        } else if (token.type == Token::Type::STRING_LITERAL) {
            values.push_back(token.value);
        } else {
            LOG_ERROR("Parse error: Expected a literal value in value list.");
            throw std::runtime_error("Expected a literal value.");
        }

        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return values;
}

std::optional<Expression> Parser::parse_where_clause() {
    if (peek().type == Token::Type::KEYWORD && peek().value == "WHERE") {
        consume();
        return parse_expression();
    }
    return std::nullopt;
}

std::vector<std::pair<std::string, Literal>> Parser::parse_set_clause() {
    std::vector<std::pair<std::string, Literal>> set_clause;
    do {
        auto identifier = consume();
        expect(Token::Type::OPERATOR, "=");
        auto literal = consume();
        set_clause.push_back({identifier.value, literal.value});
        if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
            consume();
        } else {
            break;
        }
    } while (true);
    return set_clause;
}

std::vector<std::string> Parser::parse_group_by_clause() {
    std::vector<std::string> group_by_fields;
    if (peek().type == Token::Type::KEYWORD && peek().value == "GROUP") {
        consume();
        expect(Token::Type::KEYWORD, "BY");
        do {
            group_by_fields.push_back(consume().value);
            if (peek().type == Token::Type::OPERATOR && peek().value == ",") {
                consume();
            } else {
                break;
            }
        } while (true);
    }
    return group_by_fields;
}

Expression Parser::parse_expression(int precedence) {
    auto left = parse_primary_expression();

    while (true) {
        auto op = peek().value;
        int new_precedence = 0;
        if (op == "AND" || op == "OR") new_precedence = 1;
        if (op == "=" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "LIKE") new_precedence = 2;

        if (new_precedence <= precedence) break;

        consume();
        auto right = parse_expression(new_precedence);
        if (op == "AND" || op == "OR") {
            left = std::make_unique<LogicalExpression>(LogicalExpression{std::move(left), op, std::move(right)});
        } else {
            left = std::make_unique<BinaryExpression>(BinaryExpression{std::move(left), op, std::move(right)});
        }
    }

    return left;
}

Expression Parser::parse_primary_expression() {
    auto token = consume();
    if (token.type == Token::Type::IDENTIFIER) {
        return Identifier{token.value};
    } else if (token.type == Token::Type::NUMERIC_LITERAL) {
        return Literal{std::stod(token.value)};
    } else if (token.type == Token::Type::STRING_LITERAL) {
        return Literal{token.value};
    }
    LOG_ERROR("Parse error: Unexpected token in expression: " + token.value);
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
        std::string error_msg = "Expected token " + value + " but got " + token.value;
        LOG_ERROR("Parse error: " + error_msg);
        throw std::runtime_error(error_msg);
    }
}

} // namespace Query
} // namespace TissDB