#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace TissDB {
namespace Query {

// Represents a simple WHERE clause: `field = value`.
// More complex conditions (AND, OR, >, <) will be added later.
struct WhereClause {
    std::string field;
    // For now, we'll treat all values in the WHERE clause as strings for simplicity.
    // A proper implementation would use a variant for different types.
    std::string value;
};

// Represents a TissQL SELECT statement.
struct SelectStatement {
    std::vector<std::string> fields;
    std::string collection_name;
    std::optional<WhereClause> where_clause;
};

// The Abstract Syntax Tree (AST) for a query.
// It's a variant that can hold any type of statement.
// For now, it only supports SELECT.
using AST = std::variant<SelectStatement>;

} // namespace Query
} // namespace TissDB
