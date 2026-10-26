#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <memory>
#include <map>
#include "../common/document.h"
#include "../common/rdf.h"

namespace TissDB {
namespace Query {

// Represents a literal value in a query
struct Null {};
using Literal = std::variant<std::string, double, bool, Null, Date, Time, DateTime, TissDB::Timestamp>;

// Forward-declare recursive types
struct BinaryExpression;
struct LogicalExpression;
struct FunctionExpression;
struct BetweenExpression;

struct IntervalLiteral {
    double value;
    std::string unit;
};

// Represents a column identifier
struct Identifier {
    std::string name;
};

// Represents a parameter placeholder '?' in a query.
struct ParameterExpression {
    // The zero-based index of the parameter in the parameter list.
    size_t index;
};

// The Expression variant represents any kind of expression in the WHERE clause
using Expression = std::variant<Identifier, Literal, IntervalLiteral, ParameterExpression, std::shared_ptr<BinaryExpression>, std::shared_ptr<LogicalExpression>, std::shared_ptr<FunctionExpression>, std::shared_ptr<BetweenExpression>>;

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

struct FunctionExpression {
    std::string name;
    std::vector<Expression> args;
};

struct BetweenExpression {
    Expression value;
    Expression lower;
    Expression upper;
    bool negated = false;
};

// Represents the type of an aggregate function
enum class AggregateType {
    COUNT,
    AVG,
    SUM,
    MIN,
    MAX
};

// Represents an aggregate function call
struct AggregateFunction {
    AggregateType type;
    // The field to aggregate on. nullopt represents '*' for COUNT(*).
    std::optional<std::string> field_name;
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
    std::string join_alias;
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
    std::string from_alias;
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
    std::vector<std::pair<std::string, Expression>> set_clause;
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
    std::vector<Expression> values;
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


// --- SPARQL ---

// A SPARQL variable (?name or $name), stored without the sigil. Blank nodes in a
// query pattern act as variables and are stored as "_:label", which cannot clash
// with variable names and is never projected by SELECT *.
struct SparqlVariable {
    std::string name;
};

// A position in a triple pattern: either a variable or a concrete RDF term.
using SparqlPatternTerm = std::variant<SparqlVariable, Rdf::Term>;

struct SparqlTriplePattern {
    SparqlPatternTerm subject;
    SparqlPatternTerm predicate;
    SparqlPatternTerm object;
};

// An expression inside FILTER(...).
struct SparqlExpression {
    enum class Kind {
        TERM,      // a constant RDF term
        VARIABLE,  // a variable reference
        UNARY,     // op applied to args[0] ("!")
        BINARY,    // args[0] op args[1] ("||", "&&", "=", "!=", "<", ">", "<=", ">=")
        FUNCTION   // built-in call; op holds the upper-cased function name
    };
    Kind kind = Kind::TERM;
    std::string op;
    Rdf::Term term;
    std::string variable;
    std::vector<std::shared_ptr<SparqlExpression>> args;
};

struct SparqlGroupPattern;

// One element of a group graph pattern. Elements are evaluated left to right:
// a TRIPLES element is joined with the solutions so far, and an OPTIONAL element
// is left-joined (solutions it cannot extend are kept unchanged).
struct SparqlGroupElement {
    enum class Kind { TRIPLES, OPTIONAL };
    Kind kind = Kind::TRIPLES;
    std::vector<SparqlTriplePattern> triples;  // TRIPLES: a basic graph pattern
    std::shared_ptr<SparqlGroupPattern> group; // OPTIONAL: the optional group
};

// A { ... } group. Its filters restrict the whole group's solutions; for an
// OPTIONAL group they instead decide whether the optional part matches.
struct SparqlGroupPattern {
    std::vector<SparqlGroupElement> elements;
    std::vector<std::shared_ptr<SparqlExpression>> filters;
};

struct SparqlOrderCondition {
    std::string variable;
    bool descending = false;
};

// Represents a SPARQL SELECT query over the database's RDF triples.
struct SparqlSelectStatement {
    std::map<std::string, std::string> prefixes; // prefix label (without ':') -> namespace IRI
    std::string base;
    bool distinct = false;
    bool select_all = false;            // SELECT *
    std::vector<std::string> variables; // projected variables when !select_all
    SparqlGroupPattern where;
    std::vector<SparqlOrderCondition> order_by;
    std::optional<size_t> limit;
    std::optional<size_t> offset;
};


// The Abstract Syntax Tree (AST) for a query.
using AST = std::variant<SelectStatement, UpdateStatement, DeleteStatement, InsertStatement, CreateTableStatement, SparqlSelectStatement>;

} // namespace Query
} // namespace TissDB
