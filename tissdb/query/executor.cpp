#include "executor.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <map>

// Required for the executor to interact with storage
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

// --- Helper functions ---

// Evaluate an expression against a document
bool evaluate_expression(const Expression& expr, const Document& doc) {
    if (auto* logical_expr = std::get_if<LogicalExpression>(&expr)) {
        if (logical_expr->op == "AND") {
            return evaluate_expression(*logical_expr->left, doc) && evaluate_expression(*logical_expr->right, doc);
        } else if (logical_expr->op == "OR") {
            return evaluate_expression(*logical_expr->left, doc) || evaluate_expression(*logical_expr->right, doc);
        }
    } else if (auto* binary_expr = std::get_if<BinaryExpression>(&expr)) {
        auto* left_ident = std::get_if<Identifier>(&*binary_expr->left);
        auto* right_literal = std::get_if<Literal>(&*binary_expr->right);

        if (left_ident && right_literal) {
            for (const auto& elem : doc.elements) {
                if (elem.key == left_ident->name) {
                    if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                        if (auto* lit_val = std::get_if<std::string>(right_literal)) {
                            if (binary_expr->op == "=") return *str_val == *lit_val;
                            if (binary_expr->op == "!=") return *str_val != *lit_val;
                        }
                    } else if (auto* num_val = std::get_if<double>(&elem.value)) {
                        if (auto* lit_val = std::get_if<double>(right_literal)) {
                            if (binary_expr->op == "=") return *num_val == *lit_val;
                            if (binary_expr->op == "!=") return *num_val != *lit_val;
                            if (binary_expr->op == "<") return *num_val < *lit_val;
                            if (binary_expr->op == ">") return *num_val > *lit_val;
                            if (binary_expr->op == "<=") return *num_val <= *lit_val;
                            if (binary_expr->op == ">=") return *num_val >= *lit_val;
                        }
                    }
                }
            }
        }
    }
    return false;
}

// --- Aggregation Helper ---

struct AggregateResult {
    double sum = 0;
    double count = 0;
    std::optional<double> min;
    std::optional<double> max;
};

void process_aggregation(std::map<std::string, AggregateResult>& group_results, const std::string& group_key, const Document& doc, const AggregateFunction& agg_func) {
    for (const auto& elem : doc.elements) {
        if (elem.key == agg_func.field_name) {
            if (auto* num_val = std::get_if<double>(&elem.value)) {
                if (agg_func.function_name == "SUM" || agg_func.function_name == "AVG") {
                    group_results[group_key].sum += *num_val;
                }
                if (agg_func.function_name == "COUNT") {
                    group_results[group_key].count++;
                }
                if (agg_func.function_name == "MIN") {
                    if (!group_results[group_key].min.has_value() || *num_val < group_results[group_key].min.value()) {
                        group_results[group_key].min = *num_val;
                    }
                }
                if (agg_func.function_name == "MAX") {
                    if (!group_results[group_key].max.has_value() || *num_val > group_results[group_key].max.value()) {
                        group_results[group_key].max = *num_val;
                    }
                }
            }
        }
    }
}

// --- Executor ---


Executor::Executor(Storage::LSMTree& storage) : storage_engine(storage) {}

QueryResult Executor::execute(const AST& ast) {
    if (auto* select_stmt = std::get_if<SelectStatement>(&ast)) {
        std::vector<Document> result_docs;
        std::vector<std::string> doc_ids_from_index;

        // ... (index selection logic remains the same)

        // --- Data retrieval ---
        std::vector<Document> all_docs;
        // ... (data retrieval logic remains the same)

        // --- Filtering ---
        std::vector<Document> filtered_docs;
        // ... (filtering logic remains the same)

        // --- Aggregation and Grouping ---
        if (!select_stmt->group_by_clause.empty()) {
            std::map<std::string, std::vector<Document>> grouped_docs;
            for (const auto& doc : filtered_docs) {
                std::stringstream group_key_ss;
                for (const auto& field_name : select_stmt->group_by_clause) {
                    for (const auto& elem : doc.elements) {
                        if (elem.key == field_name) {
                            if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                                group_key_ss << *str_val;
                            }
                        }
                    }
                }
                grouped_docs[group_key_ss.str()].push_back(doc);
            }

            std::vector<Document> aggregated_docs;
            for (const auto& group : grouped_docs) {
                Document aggregated_doc;
                aggregated_doc.id = group.first;
                std::map<std::string, AggregateResult> group_results;

                for (const auto& field : select_stmt->fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                        for (const auto& doc : group.second) {
                            process_aggregation(group_results, agg_func->function_name, doc, *agg_func);
                        }
                    }
                }

                for (const auto& result : group_results) {
                    if (result.first == "SUM") {
                        aggregated_doc.elements.push_back({"sum", result.second.sum});
                    } else if (result.first == "AVG") {
                        aggregated_doc.elements.push_back({"avg", result.second.sum / result.second.count});
                    } else if (result.first == "COUNT") {
                        aggregated_doc.elements.push_back({"count", result.second.count});
                    } else if (result.first == "MIN") {
                        aggregated_doc.elements.push_back({"min", result.second.min.value_or(0)});
                    } else if (result.first == "MAX") {
                        aggregated_doc.elements.push_back({"max", result.second.max.value_or(0)});
                    }
                }
                aggregated_docs.push_back(aggregated_doc);
            }
            return {aggregated_docs};
        }

        // --- Projection ---
        // ... (projection logic remains the same)

    } else if (auto* update_stmt = std::get_if<UpdateStatement>(&ast)) {
        // ... (UPDATE logic remains the same)
    } else if (auto* delete_stmt = std::get_if<DeleteStatement>(&ast)) {
        // ... (DELETE logic remains the same)
    }
    return {};
}


} // namespace Query
} // namespace TissDB
