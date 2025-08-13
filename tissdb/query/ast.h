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

// The Expression variant represents any kind of expression in the WHERE clause
using Expression = std::variant<Identifier, Literal, BinaryExpression, LogicalExpression>;

// Represents a TissQL SELECT statement.
struct SelectStatement {
    std::vector<std::string> fields;
    std::string collection_name;
    std::optional<Expression> where_clause;
};

// The Abstract Syntax Tree (AST) for a query.
using AST = std::variant<SelectStatement>;

} // namespace Query
} // namespace TissDB