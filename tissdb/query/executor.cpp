#include "executor.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <map>
#include <functional>

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
                    all_docs.push_back(*doc);
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
        // ... (UPDATE logic remains the same)
    } else if (auto* delete_stmt = std::get_if<DeleteStatement>(&ast)) {
        // ... (DELETE logic remains the same)
    }
    return {};
}


} // namespace Query
} // namespace TissDB
