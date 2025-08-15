#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <memory>

namespace TissDB {
namespace Query {

// Represents a literal value in a query
using Literal = std::variant<std::string, double>;

// Forward-declare recursive types
struct BinaryExpression;
struct LogicalExpression;

// Represents a column identifier
struct Identifier {
    std::string name;
};

// The Expression variant represents any kind of expression in the WHERE clause
using Expression = std::variant<Identifier, Literal, std::unique_ptr<BinaryExpression>, std::unique_ptr<LogicalExpression>>;

// Represents a binary expression (e.g., field = 'value', price > 100)
struct BinaryExpression {
    Expression left;
    std::string op;
    Expression right;
};

// Represents a logical expression (e.g., ... AND ...)
struct LogicalExpression {
    Expression left;
    std::string op;
    Expression right;
};

// Represents an aggregate function call
struct AggregateFunction {
    std::string function_name;
    std::string field_name;
};


// Represents a TissQL SELECT statement.
struct SelectStatement {
    std::vector<std::variant<std::string, AggregateFunction>> fields;
    std::string collection_name;
    std::optional<Expression> where_clause;
    std::vector<std::string> group_by_clause;
};

// Represents a TissQL UPDATE statement.
struct UpdateStatement {
    std::string collection_name;
    std::vector<std::pair<std::string, Literal>> set_clause;
    std::optional<Expression> where_clause;
};

// Represents a TissQL DELETE statement.
struct DeleteStatement {
    std::string collection_name;
    std::optional<Expression> where_clause;
};

// Represents a TissQL INSERT statement.
struct InsertStatement {
    std::string collection_name;
    std::vector<std::string> columns;
    std::vector<Literal> values;
};

// The Abstract Syntax Tree (AST) for a query.
using AST = std::variant<SelectStatement, UpdateStatement, DeleteStatement, InsertStatement>;

} // namespace Query
} // namespace TissDB