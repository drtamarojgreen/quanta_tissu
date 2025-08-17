#include "executor.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <map>
#include <functional>
#include <cmath>
#include <algorithm> // For std::find_if

// Required for the executor to interact with storage
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

// --- Helper functions ---

// Evaluate an expression against a document
bool evaluate_expression(const Expression& expr, const Document& doc) {
    if (const auto* logical_expr_ptr = std::get_if<std::unique_ptr<LogicalExpression>>(&expr)) {
        const auto& logical_expr = *logical_expr_ptr;
        if (logical_expr->op == "AND") {
            return evaluate_expression(logical_expr->left, doc) && evaluate_expression(logical_expr->right, doc);
        } else if (logical_expr->op == "OR") {
            return evaluate_expression(logical_expr->left, doc) || evaluate_expression(logical_expr->right, doc);
        }
    } else if (const auto* binary_expr_ptr = std::get_if<std::unique_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;
        const auto* left_ident = std::get_if<Identifier>(&binary_expr->left);
        const auto* right_literal = std::get_if<Literal>(&binary_expr->right);

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
    double sum_sq = 0;
    std::optional<double> min;
    std::optional<double> max;
};

void process_aggregation(std::map<std::string, AggregateResult>& results_map, const std::string& result_key, const Document& doc, const AggregateFunction& agg_func) {
    // Handle COUNT(*) case where field_name can be "*"
    if (agg_func.field_name == "*") {
        if (agg_func.function_name == "COUNT") {
            results_map[result_key].count++;
        }
        return;
    }

    for (const auto& elem : doc.elements) {
        if (elem.key == agg_func.field_name) {
            if (auto* num_val = std::get_if<double>(&elem.value)) {
                auto& result = results_map[result_key]; // Get reference to the result for this key

                if (agg_func.function_name == "SUM" || agg_func.function_name == "AVG" || agg_func.function_name == "STDDEV") {
                    result.sum += *num_val;
                    result.sum_sq += (*num_val) * (*num_val);
                }
                if (agg_func.function_name == "COUNT" || agg_func.function_name == "AVG" || agg_func.function_name == "STDDEV") {
                    result.count++;
                }
                if (agg_func.function_name == "MIN") {
                    if (!result.min.has_value() || *num_val < result.min.value()) {
                        result.min = *num_val;
                    }
                }
                if (agg_func.function_name == "MAX") {
                    if (!result.max.has_value() || *num_val > result.max.value()) {
                        result.max = *num_val;
                    }
                }
            }
        }
    }
}

// --- NEW HELPER FUNCTION ---
// Extracts all simple equality conditions (field = 'value') from a WHERE clause.
void extract_equality_conditions(const Expression& expr, std::map<std::string, std::string>& conditions) {
    if (const auto* logical_expr_ptr = std::get_if<std::unique_ptr<LogicalExpression>>(&expr)) {
        const auto& logical_expr = *logical_expr_ptr;
        // We can only use indexes for AND'd conditions.
        if (logical_expr->op == "AND") {
            extract_equality_conditions(logical_expr->left, conditions);
            extract_equality_conditions(logical_expr->right, conditions);
        }
    } else if (const auto* binary_expr_ptr = std::get_if<std::unique_ptr<BinaryExpression>>(&expr)) {
        const auto& binary_expr = *binary_expr_ptr;
        if (binary_expr->op == "=") {
            const auto* left_ident = std::get_if<Identifier>(&binary_expr->left);
            const auto* right_literal = std::get_if<Literal>(&binary_expr->right);
            if (left_ident && right_literal) {
                if (const auto* str_lit = std::get_if<std::string>(right_literal)) {
                    conditions[left_ident->name] = *str_lit;
                }
                // Note: This could be extended to handle numbers, bools etc.
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
        bool index_used = false;

        // --- NEW Index Selection Logic ---
        if (select_stmt->where_clause) {
            // 1. Extract all potential fields for indexing from the WHERE clause.
            std::map<std::string, std::string> conditions;
            extract_equality_conditions(*select_stmt->where_clause, conditions);

            if (!conditions.empty()) {
                // 2. Get all available indexes for the collection.
                auto available_indexes = storage_engine.get_available_indexes(select_stmt->from_collection);

                // 3. Find the best index to use.
                std::vector<std::string> best_index_fields;
                for (const auto& index_fields : available_indexes) {
                    bool all_fields_present = true;
                    for (const auto& field : index_fields) {
                        if (conditions.find(field) == conditions.end()) {
                            all_fields_present = false;
                            break;
                        }
                    }

                    if (all_fields_present) {
                        // This index is a candidate. Is it better than the current best?
                        // "Better" means it has more fields.
                        if (index_fields.size() > best_index_fields.size()) {
                            best_index_fields = index_fields;
                        }
                    }
                }

                // 4. If we found a suitable index, use it.
                if (!best_index_fields.empty()) {
                    std::vector<std::string> values;
                    for (const auto& field : best_index_fields) {
                        values.push_back(conditions.at(field));
                    }

                    doc_ids_from_index = storage_engine.find_by_index(select_stmt->from_collection, best_index_fields, values);
                    index_used = true;

                    std::stringstream ss;
                    for(size_t i = 0; i < best_index_fields.size(); ++i) {
                        ss << best_index_fields[i] << (i < best_index_fields.size() - 1 ? ", " : "");
                    }
                    std::cout << "Using compound index for query on fields: " << ss.str() << std::endl;
                }
            }
        }


        // --- Data retrieval ---
        std::vector<Document> all_docs;
        if (index_used) {
            for (const auto& doc_id : doc_ids_from_index) {
                auto doc = storage_engine.get(select_stmt->from_collection, doc_id);
                if (doc) {
                    all_docs.push_back(**doc);
                }
            }
        } else {
            // Full scan if no index is used
            std::cout << "No suitable index found. Performing full collection scan." << std::endl;
            all_docs = storage_engine.scan(select_stmt->from_collection);
        }


        // --- Filtering ---
        std::vector<Document> filtered_docs;
        if (select_stmt->where_clause) {
            for (const auto& doc : all_docs) {
                if (evaluate_expression(*select_stmt->where_clause, doc)) {
                    filtered_docs.push_back(doc);
                }
            }
        } else {
            filtered_docs = all_docs;
        }


        // --- Aggregation and Grouping ---
        bool has_aggregate = false;
        for (const auto& field : select_stmt->fields) {
            if (std::holds_alternative<AggregateFunction>(field)) {
                has_aggregate = true;
                break;
            }
        }

        if (has_aggregate) {
            std::vector<Document> aggregated_docs;
            // Group documents if a GROUP BY clause exists
            if (!select_stmt->group_by_clause.empty()) {
                std::map<std::string, std::vector<Document>> grouped_docs;
                for (const auto& doc : filtered_docs) {
                    std::stringstream group_key_ss;
                    for (const auto& field_name : select_stmt->group_by_clause) {
                        for (const auto& elem : doc.elements) {
                            if (elem.key == field_name) {
                                if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                                    group_key_ss << *str_val << "-";
                                } else if (auto* num_val = std::get_if<double>(&elem.value)) {
                                    group_key_ss << *num_val << "-";
                                }
                            }
                        }
                    }
                    grouped_docs[group_key_ss.str()].push_back(doc);
                }

                for (auto const& [group_key, docs] : grouped_docs) {
                    Document aggregated_doc;
                    aggregated_doc.id = group_key;
                    std::map<std::string, AggregateResult> group_results;

                    // Add group-by fields to the result document
                    if (!docs.empty()) {
                        const auto& first_doc = docs.front();
                        for (const auto& field_name : select_stmt->group_by_clause) {
                            for (const auto& elem : first_doc.elements) {
                                if (elem.key == field_name) {
                                    aggregated_doc.elements.push_back(elem);
                                    break;
                                }
                            }
                        }
                    }

                    // First, process all aggregations for the group
                    for (const auto& field : select_stmt->fields) {
                        if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                            std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                            for (const auto& doc : docs) {
                                process_aggregation(group_results, result_key, doc, *agg_func);
                            }
                        }
                    }

                    // Then, construct the final document from the results
                    for (const auto& field : select_stmt->fields) {
                        if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                            std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                            const auto& result = group_results.at(result_key);
                            if (agg_func->function_name == "SUM") aggregated_doc.elements.push_back({result_key, result.sum});
                            else if (agg_func->function_name == "AVG") aggregated_doc.elements.push_back({result_key, result.count > 0 ? result.sum / result.count : 0});
                            else if (agg_func->function_name == "COUNT") aggregated_doc.elements.push_back({result_key, result.count});
                            else if (agg_func->function_name == "MIN") aggregated_doc.elements.push_back({result_key, result.min.value_or(0)});
                            else if (agg_func->function_name == "MAX") aggregated_doc.elements.push_back({result_key, result.max.value_or(0)});
                        }
                    }
                    aggregated_docs.push_back(aggregated_doc);
                }
            } else { // Handle aggregation without GROUP BY
                Document aggregated_doc;
                aggregated_doc.id = "aggregate";
                std::map<std::string, AggregateResult> group_results;

                // First, process all aggregations for the full result set
                for (const auto& field : select_stmt->fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                         std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                        for (const auto& doc : filtered_docs) {
                           process_aggregation(group_results, result_key, doc, *agg_func);
                        }
                    }
                }

                // Then, construct the final document from the results
                for (const auto& field : select_stmt->fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                        std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                        const auto& result = group_results.at(result_key);
                        if (agg_func->function_name == "SUM") aggregated_doc.elements.push_back({result_key, result.sum});
                        else if (agg_func->function_name == "AVG") aggregated_doc.elements.push_back({result_key, result.count > 0 ? result.sum / result.count : 0});
                        else if (agg_func->function_name == "COUNT") aggregated_doc.elements.push_back({result_key, result.count});
                        else if (agg_func->function_name == "MIN") aggregated_doc.elements.push_back({result_key, result.min.value_or(0)});
                        else if (agg_func->function_name == "MAX") aggregated_doc.elements.push_back({result_key, result.max.value_or(0)});
                        else if (agg_func->function_name == "STDDEV") {
                            if (result.count > 0) {
                                double mean = result.sum / result.count;
                                double variance = (result.sum_sq / result.count) - (mean * mean);
                                aggregated_doc.elements.push_back({result_key, sqrt(variance)});
                            } else {
                                aggregated_doc.elements.push_back({result_key, 0.0});
                            }
                        }
                    }
                }
                aggregated_docs.push_back(aggregated_doc);
            }
            return {aggregated_docs}; // Return QueryResult containing aggregated_docs
        }

        // --- Projection ---
        bool select_all = false;
        if (!select_stmt->fields.empty()) {
            if (auto* field_str = std::get_if<std::string>(&select_stmt->fields[0])) {
                if (*field_str == "*") {
                    select_all = true;
                }
            }
        }

        if (select_all) {
            return {filtered_docs};
        } else {
            std::vector<Document> projected_docs;
            for (const auto& doc : filtered_docs) {
                Document projected_doc;
                projected_doc.id = doc.id;
                for (const auto& field_variant : select_stmt->fields) {
                     if (auto* ident_str = std::get_if<std::string>(&field_variant)) {
                        for (const auto& elem : doc.elements) {
                            if (elem.key == *ident_str) {
                                projected_doc.elements.push_back(elem);
                            }
                        }
                    }
                }
                projected_docs.push_back(projected_doc);
            }
            return {projected_docs};
        }

    } else if (auto* update_stmt = std::get_if<UpdateStatement>(&ast)) {
        auto all_docs = storage_engine.scan(update_stmt->collection_name);
        int updated_count = 0;

        for (auto& doc : all_docs) {
            bool should_update = false;
            if (update_stmt->where_clause) {
                if (evaluate_expression(*update_stmt->where_clause, doc)) {
                    should_update = true;
                }
            } else {
                should_update = true; // No WHERE clause, update all documents
            }

            if (should_update) {
                for (const auto& set_pair : update_stmt->set_clause) {
                    const std::string& field_to_update = set_pair.first;
                    const Literal& new_value = set_pair.second;

                    auto it = std::find_if(doc.elements.begin(), doc.elements.end(),
                                           [&](const Element& elem) { return elem.key == field_to_update; });

                    if (it != doc.elements.end()) {
                        // Field exists, update it
                        if (const auto* str_val = std::get_if<std::string>(&new_value)) {
                            it->value = *str_val;
                        } else if (const auto* num_val = std::get_if<double>(&new_value)) {
                            it->value = *num_val;
                        }
                    } else {
                        // Field does not exist, add it
                        if (const auto* str_val = std::get_if<std::string>(&new_value)) {
                            doc.elements.push_back({field_to_update, *str_val});
                        } else if (const auto* num_val = std::get_if<double>(&new_value)) {
                            doc.elements.push_back({field_to_update, *num_val});
                        }
                    }
                }
                storage_engine.put(update_stmt->collection_name, doc.id, doc);
                updated_count++;
            }
        }

        Document result_doc;
        result_doc.id = "summary";
        result_doc.elements.push_back({"updated_count", (double)updated_count});
        return {result_doc};

    } else if (auto* delete_stmt = std::get_if<DeleteStatement>(&ast)) {
        auto all_docs = storage_engine.scan(delete_stmt->collection_name);
        int deleted_count = 0;

        for (const auto& doc : all_docs) {
            bool should_delete = false;
            if (delete_stmt->where_clause) {
                if (evaluate_expression(*delete_stmt->where_clause, doc)) {
                    should_delete = true;
                }
            } else {
                should_delete = true; // No WHERE clause, delete all documents
            }

            if (should_delete) {
                storage_engine.del(delete_stmt->collection_name, doc.id);
                deleted_count++;
            }
        }

        Document result_doc;
        result_doc.id = "summary";
        result_doc.elements.push_back({"deleted_count", (double)deleted_count});
        return {result_doc};
    }
    return {};
}


} // namespace Query
} // namespace TissDB