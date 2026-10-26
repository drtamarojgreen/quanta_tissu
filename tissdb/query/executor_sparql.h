#pragma once

#include <map>
#include <string>
#include <vector>

#include "ast.h"
#include "executor.h"
#include "../common/rdf.h"
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

// One SPARQL solution: variable name (without '?') -> bound term.
// Unbound variables are absent from the map.
using SparqlSolution = std::map<std::string, Rdf::Term>;

// Evaluates a SPARQL SELECT against the triples in the database's default RDF
// collection and returns the projected solutions in result order.
std::vector<SparqlSolution> evaluate_sparql_select(Storage::LSMTree& storage_engine, const SparqlSelectStatement& stmt);

// Executes a SPARQL SELECT and returns one document per solution. Each projected,
// bound variable becomes an element whose value is the term in N-Triples syntax
// (as in the SPARQL TSV results format), e.g. <http://ex.org/a> or "42"^^<...#integer>.
QueryResult execute_sparql_select_statement(Storage::LSMTree& storage_engine, const SparqlSelectStatement& stmt);

} // namespace Query
} // namespace TissDB
