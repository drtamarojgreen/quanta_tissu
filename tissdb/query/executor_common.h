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
    int64_t count = 0;
    double sum_sq = 0;
    std::optional<double> min;
    std::optional<double> max;
    std::optional<std::string> min_str;
    std::optional<std::string> max_str;
};

// --- Helper function declarations ---

std::string like_to_regex(std::string pattern);
bool evaluate_expression(const Expression& expr, const Document& doc);
void process_aggregation(std::map<std::string, AggregateResult>& results_map, const std::string& result_key, const Document& doc, const AggregateFunction& agg_func);
Document combine_documents(const Document& doc1, const Document& doc2);
void extract_equality_conditions(const Expression& expr, std::map<std::string, std::string>& conditions);

} // namespace Query
} // namespace TissDB
