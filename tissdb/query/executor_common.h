#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "ast.h"
#include "../common/document.h"

namespace TissDB {
namespace Query {

// --- Aggregation Helper ---
#include <cstdint>

struct AggregateResult {
    double sum = 0;
    int64_t count = 0; // For COUNT aggregate
    int64_t avg_count = 0; // For AVG aggregate, only counts numeric values
    double sum_sq = 0;
    std::optional<double> min;
    std::optional<double> max;
    std::optional<std::string> min_str;
    std::optional<std::string> max_str;
};

// --- Helper function declarations ---

// Resolves an expression node to a final Value, using document fields and query parameters.
Value resolve_expression_to_value(const Expression& expr, const Document& doc, const std::vector<Literal>& params);

std::string like_to_regex(std::string pattern);
bool evaluate_expression(const Expression& expr, const Document& doc, const std::vector<Literal>& params);
Literal evaluate_update_expression(const Expression& expr, const Document& doc, const std::vector<Literal>& params);
void process_aggregation(std::map<std::string, AggregateResult>& results_map, const std::string& result_key, const Document& doc, const AggregateFunction& agg_func);
Document combine_documents(const Document& doc1, const std::string& alias1, const Document& doc2, const std::string& alias2);
void extract_equality_conditions(const Expression& expr, std::map<std::string, std::string>& conditions);
const Value* get_value_from_doc(const Document& doc, const std::string& key);
std::string value_to_string(const Value& value);

} // namespace Query
} // namespace TissDB
