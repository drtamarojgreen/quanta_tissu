#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <memory>

namespace TissDB {
namespace Query {

// Forward declaration
struct Expression;

// Represents a literal value in a query
using Literal = std::variant<std::string, double>;

// Represents a binary expression (e.g., field = 'value', price > 100)
struct BinaryExpression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

// Represents a logical expression (e.g., ... AND ...)
struct LogicalExpression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

// Represents a column identifier
struct Identifier {
    std::string name;
};

// Represents an aggregate function call
struct AggregateFunction {
    std::string function_name;
    std::string field_name;
};

// The Expression variant represents any kind of expression in the WHERE clause
struct Expression : std::variant<Identifier, Literal, BinaryExpression, LogicalExpression> {
    using variant::variant;
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

// The Abstract Syntax Tree (AST) for a query.
using AST = std::variant<SelectStatement, UpdateStatement, DeleteStatement>;

} // namespace Query
} // namespace TissDB