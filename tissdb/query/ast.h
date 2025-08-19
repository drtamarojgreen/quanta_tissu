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
using Expression = std::variant<Identifier, Literal, std::shared_ptr<BinaryExpression>, std::shared_ptr<LogicalExpression>>;

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

// Forward declarations for recursive structures
struct SelectStatement; // Forward declare SelectStatement

// Represents a join type
enum class JoinType {
    INNER,
    LEFT,
    RIGHT,
    FULL,
    CROSS
};

// Represents a join clause
struct JoinClause {
    std::string collection_name; // The right-hand side collection to join with
    JoinType type;
    Expression on_condition; // The ON clause expression
};

// Represents a union clause
struct UnionClause {
    std::shared_ptr<SelectStatement> left_select;
    std::shared_ptr<SelectStatement> right_select;
    bool all; // true for UNION ALL, false for UNION DISTINCT
};

// Represents a drilldown clause
struct DrilldownClause {
    std::vector<std::string> fields;
};


// Represents a TissQL SELECT statement.
struct SelectStatement {
    std::vector<std::variant<std::string, AggregateFunction>> fields;
    std::string from_collection;
    std::optional<Expression> where_clause;
    std::vector<std::string> group_by_clause;
    std::vector<std::pair<std::string, std::string>> order_by_clause; // Added for ORDER BY
    std::optional<double> limit_clause; // Added for LIMIT

    // Optional JOIN and UNION clauses
    std::optional<JoinClause> join_clause; // Added for JOIN
    std::optional<UnionClause> union_clause; // Added for UNION
    std::optional<DrilldownClause> drilldown_clause; // Added for WITH DRILLDOWN
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

// Represents a column definition in a CREATE TABLE statement
struct ColumnDefinition {
    std::string column_name;
    std::string data_type;
};

// Represents a primary key constraint
struct PrimaryKeyConstraint {
    std::vector<std::string> columns;
};

// Represents a foreign key constraint
struct ForeignKeyConstraint {
    std::vector<std::string> columns;
    std::string foreign_table;
    std::vector<std::string> foreign_columns;
};

// Represents a constraint in a CREATE TABLE statement
using Constraint = std::variant<PrimaryKeyConstraint, ForeignKeyConstraint>;

// Represents a TissQL CREATE TABLE statement
struct CreateTableStatement {
    std::string table_name;
    std::vector<ColumnDefinition> columns;
    std::vector<Constraint> constraints;
};


// The Abstract Syntax Tree (AST) for a query.
using AST = std::variant<SelectStatement, UpdateStatement, DeleteStatement, InsertStatement, CreateTableStatement>;

} // namespace Query
} // namespace TissDB