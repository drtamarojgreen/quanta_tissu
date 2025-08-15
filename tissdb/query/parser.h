#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <variant>
#include <optional>

#include "ast.h"

namespace TissDB {
namespace Query {

// Represents a token from the query string
struct Token {
    enum class Type { IDENTIFIER, KEYWORD, NUMERIC_LITERAL, STRING_LITERAL, OPERATOR, EOI };
    Type type;
    std::string value;
};

class Parser {
public:
    Parser();
    AST parse(const std::string& query_string);

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    // Tokenizer
    std::vector<Token> tokenize(const std::string& query_string);

    // Parser methods
    SelectStatement parse_select_statement();
    UpdateStatement parse_update_statement();
    DeleteStatement parse_delete_statement();
    InsertStatement parse_insert_statement();
    std::vector<std::variant<std::string, AggregateFunction>> parse_select_list();
    std::string parse_table_name();
    std::vector<std::string> parse_column_list();
    std::vector<Literal> parse_value_list();
    std::optional<Expression> parse_where_clause();
    std::vector<std::pair<std::string, Literal>> parse_set_clause();
    std::vector<std::string> parse_group_by_clause();
    Expression parse_expression(int precedence = 0);
    Expression parse_primary_expression();

    // Helper methods
    Token peek();
    Token consume();
    void expect(Token::Type type, const std::string& value = "");
};

} // namespace Query
} // namespace TissDB
