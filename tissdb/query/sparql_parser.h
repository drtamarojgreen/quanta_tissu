#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ast.h"

namespace TissDB {
namespace Query {

// True if the query is SPARQL. TissDB identifies SPARQL by a leading PREFIX
// keyword (case-insensitive, after optional whitespace and '#' comments).
bool is_sparql_query(const std::string& query_string);

// Thrown for malformed or unsupported SPARQL. what() reads
// "SPARQL syntax error at line L, column C: <detail>" (1-based, in bytes).
class SparqlSyntaxError : public std::runtime_error {
public:
    SparqlSyntaxError(const std::string& detail, size_t line, size_t column);
    const std::string& detail() const { return detail_; }
    size_t line() const { return line_; }
    size_t column() const { return column_; }

private:
    std::string detail_;
    size_t line_;
    size_t column_;
};

struct SparqlToken {
    enum class Type { IRI, PNAME, VAR, BLANK, STRING, LANGTAG, INTEGER, DECIMAL, DOUBLE, WORD, PUNCT, EOI };
    Type type;
    std::string value;
    size_t offset;    // byte offset into the query, for error messages
    std::string text; // the token as written in the query
};

// Parses the supported SPARQL subset:
//   PREFIX / BASE declarations
//   SELECT [DISTINCT|REDUCED] (?var ... | *)
//   [WHERE] { triple patterns (with ';', ',' and 'a'), FILTER(...) and
//             OPTIONAL { ... } (nestable) }
//   [ORDER BY ?v | ASC(?v) | DESC(?v) ...] [LIMIT n] [OFFSET n]
class SparqlParser {
public:
    SparqlSelectStatement parse(const std::string& query_string);

private:
    std::string query_;
    std::vector<SparqlToken> tokens;
    size_t pos = 0;
    SparqlSelectStatement stmt;

    std::vector<SparqlToken> tokenize(const std::string& query_string);

    void parse_prologue();
    void parse_select_clause();
    void parse_group_graph_pattern(SparqlGroupPattern& group);
    void parse_triples_same_subject(std::vector<SparqlTriplePattern>& out);
    bool at_group_element_end() const;
    void reject_unsupported_construct() const;
    std::shared_ptr<SparqlExpression> parse_filter();
    void parse_solution_modifiers();

    SparqlPatternTerm parse_var_or_term();
    SparqlPatternTerm parse_verb();
    Rdf::Term parse_iri();
    Rdf::Term parse_literal();
    size_t parse_non_negative_integer(const std::string& clause);

    std::shared_ptr<SparqlExpression> parse_expression();
    std::shared_ptr<SparqlExpression> parse_and_expression();
    std::shared_ptr<SparqlExpression> parse_relational_expression();
    std::shared_ptr<SparqlExpression> parse_unary_expression();
    std::shared_ptr<SparqlExpression> parse_primary_expression();
    std::shared_ptr<SparqlExpression> parse_function_call();

    std::string resolve_iri(const std::string& iri) const;
    std::string expand_prefixed_name(const SparqlToken& token) const;

    const SparqlToken& peek() const;
    SparqlToken consume();
    bool peek_punct(const std::string& value) const;
    bool peek_keyword(const std::string& keyword) const;
    void expect_punct(const std::string& value);
    void expect_keyword(const std::string& keyword);
    [[noreturn]] void fail(const std::string& message) const;
    // Like fail, for messages that already name the offending token (no "near ..." suffix).
    [[noreturn]] void fail_here(const std::string& message) const;
    [[noreturn]] void fail_at(size_t token_index, const std::string& message) const;
    [[noreturn]] void error_at(size_t offset, const std::string& message) const;
    std::pair<size_t, size_t> line_and_column(size_t offset) const;
    std::string position_of(size_t offset) const;
    std::string describe(const SparqlToken& token) const;
};

} // namespace Query
} // namespace TissDB
