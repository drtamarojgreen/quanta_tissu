#include "executor_select.h"
#include "executor_common.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace TissDB {
namespace Query {

// Visitor for creating a string representation of a Value variant for group by keys.
struct GroupKeyVisitor {
    std::stringstream& ss;
    void operator()(const std::string& s) const { ss << s; }
    void operator()(const Number& n) const { ss << n; }
    void operator()(const Boolean& b) const { ss << (b ? "true" : "false"); }
    void operator()(const DateTime& dt) const {
        ss << std::chrono::duration_cast<std::chrono::milliseconds>(dt.time_since_epoch()).count();
    }
    void operator()(const BinaryData& /*bd*/) const {
        // Note: Grouping by binary data is tricky.
        // A proper implementation might hash the data.
        // For now, we'll use a placeholder.
        ss << "[binary_data]";
    }
    void operator()(const std::vector<TissDB::Element>& /*elements*/) const {
        // Note: Grouping by a whole sub-document is also tricky.
        // A proper implementation might serialize and hash.
        // For now, we'll use a placeholder.
        ss << "[sub_document]";
    }
};

QueryResult execute_select_statement(Storage::LSMTree& storage_engine, const SelectStatement& select_stmt) {
    // --- UNION Operation ---
    if (select_stmt.union_clause) {
        // Recursively execute the left and right select statements
        auto left_result = execute_select_statement(storage_engine, *select_stmt.union_clause->left_select);
        auto right_result = execute_select_statement(storage_engine, *select_stmt.union_clause->right_select);

        // Combine the results
        std::vector<Document> combined_docs = left_result;
        combined_docs.insert(combined_docs.end(), right_result.begin(), right_result.end());

        // If it's a UNION (not UNION ALL), remove duplicates
        if (!select_stmt.union_clause->all) {
            // To use std::unique, we need to sort the vector first.
            // A simple sort by document ID should be sufficient for grouping,
            // as the operator== will handle full equality checks.
            std::sort(combined_docs.begin(), combined_docs.end(), [](const Document& a, const Document& b) {
                return a.id < b.id;
            });

            // Remove adjacent duplicates
            combined_docs.erase(std::unique(combined_docs.begin(), combined_docs.end()), combined_docs.end());
        }

        return {combined_docs};
    }

    std::vector<Document> result_docs;
    std::vector<std::string> doc_ids_from_index;
    bool index_used = false;

    // --- Index Selection Logic ---
    if (select_stmt.where_clause) {
        std::map<std::string, std::string> conditions;
        extract_equality_conditions(*select_stmt.where_clause, conditions);

        if (!conditions.empty()) {
            auto available_indexes = storage_engine.get_available_indexes(select_stmt.from_collection);
            std::vector<std::string> best_index_fields;
            for (const auto& index_fields : available_indexes) {
                bool all_fields_present = true;
                for (const auto& field : index_fields) {
                    if (conditions.find(field) == conditions.end()) {
                        all_fields_present = false;
                        break;
                    }
                }
                if (all_fields_present && index_fields.size() > best_index_fields.size()) {
                    best_index_fields = index_fields;
                }
            }

            if (!best_index_fields.empty()) {
                std::vector<std::string> values;
                for (const auto& field : best_index_fields) {
                    values.push_back(conditions.at(field));
                }
                doc_ids_from_index = storage_engine.find_by_index(select_stmt.from_collection, best_index_fields, values);
                index_used = true;
                std::cout << "Using compound index for query." << std::endl;
            }
        }
    }

    // --- Data retrieval ---
    std::vector<Document> all_docs;
    if (index_used) {
        for (const auto& doc_id : doc_ids_from_index) {
            auto doc = storage_engine.get(select_stmt.from_collection, doc_id);
            if (doc) {
                all_docs.push_back(**doc);
            }
        }
    } else {
        std::cout << "No suitable index found. Performing full collection scan." << std::endl;
        all_docs = storage_engine.scan(select_stmt.from_collection);
    }

    // --- Join Operation ---
    if (select_stmt.join_clause) {
        const auto& join_clause = select_stmt.join_clause.value();
        std::vector<Document> right_docs = storage_engine.scan(join_clause.collection_name);
        std::vector<Document> joined_docs;

        if (join_clause.type == JoinType::CROSS) {
            for (const auto& left_doc : all_docs) {
                for (const auto& right_doc : right_docs) {
                    joined_docs.push_back(combine_documents(left_doc, right_doc));
                }
            }
        } else { // For INNER, LEFT, RIGHT, FULL joins that have a condition
            std::vector<bool> right_doc_matched(right_docs.size(), false);

            for (const auto& left_doc : all_docs) {
                bool left_doc_matched = false;
                for (size_t i = 0; i < right_docs.size(); ++i) {
                    const auto& right_doc = right_docs[i];
                    if (evaluate_expression(join_clause.on_condition, combine_documents(left_doc, right_doc))) {
                        joined_docs.push_back(combine_documents(left_doc, right_doc));
                        left_doc_matched = true;
                        right_doc_matched[i] = true;
                    }
                }

                if (!left_doc_matched && (join_clause.type == JoinType::LEFT || join_clause.type == JoinType::FULL)) {
                    // For LEFT and FULL joins, add the left doc with nulls for the right.
                    joined_docs.push_back(left_doc); // Assuming combine with null doc is implicit
                }
            }

            if (join_clause.type == JoinType::RIGHT || join_clause.type == JoinType::FULL) {
                for (size_t i = 0; i < right_docs.size(); ++i) {
                    if (!right_doc_matched[i]) {
                        // For RIGHT and FULL joins, add the unmatched right docs with nulls for the left.
                        joined_docs.push_back(right_docs[i]); // Assuming combine with null doc is implicit
                    }
                }
            }
        }
        all_docs = joined_docs;
    }

    // --- Filtering ---
    std::vector<Document> filtered_docs;
    if (select_stmt.where_clause) {
        for (const auto& doc : all_docs) {
            if (evaluate_expression(*select_stmt.where_clause, doc)) {
                filtered_docs.push_back(doc);
            }
        }
    } else {
        filtered_docs = all_docs;
    }

    // --- Aggregation and Grouping ---
    bool has_aggregate = std::any_of(select_stmt.fields.begin(), select_stmt.fields.end(),
                                     [](const auto& field){ return std::holds_alternative<AggregateFunction>(field); });

    if (has_aggregate) {
        std::vector<Document> aggregated_docs;
        if (!select_stmt.group_by_clause.empty()) {
            std::map<std::string, std::vector<Document>> grouped_docs;
            for (const auto& doc : filtered_docs) {
                std::stringstream group_key_ss;
                for (size_t i = 0; i < select_stmt.group_by_clause.size(); ++i) {
                    const auto& field_name = select_stmt.group_by_clause[i];
                    auto it = std::find_if(doc.elements.begin(), doc.elements.end(),
                                           [&](const Element& e) { return e.key == field_name; });

                    if (it != doc.elements.end()) {
                        std::visit(GroupKeyVisitor{group_key_ss}, it->value);
                    } else {
                        // Handle cases where a group by key is not in a document.
                        // We can represent this as a "null" placeholder.
                        group_key_ss << "NULL";
                    }

                    if (i < select_stmt.group_by_clause.size() - 1) {
                        group_key_ss << "::"; // Use a more distinct separator
                    }
                }
                grouped_docs[group_key_ss.str()].push_back(doc);
            }

            for (auto const& [group_key, docs] : grouped_docs) {
                Document aggregated_doc;
                aggregated_doc.id = group_key;
                std::map<std::string, AggregateResult> group_results;

                if (!docs.empty()) {
                    const auto& first_doc = docs.front();
                    for (const auto& field_name : select_stmt.group_by_clause) {
                        auto it = std::find_if(first_doc.elements.begin(), first_doc.elements.end(),
                                               [&](const Element& e){ return e.key == field_name; });
                        if (it != first_doc.elements.end()) {
                            aggregated_doc.elements.push_back(*it);
                        }
                    }
                }

                for (const auto& field : select_stmt.fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                        std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                        for (const auto& doc : docs) {
                            process_aggregation(group_results, result_key, doc, *agg_func);
                        }
                    }
                }

                for (const auto& field : select_stmt.fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                        std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                        const auto& result = group_results.at(result_key);
                        if (agg_func->function_name == "SUM") aggregated_doc.elements.push_back(TissDB::Element{result_key, result.sum});
                        else if (agg_func->function_name == "AVG") aggregated_doc.elements.push_back(TissDB::Element{result_key, result.count > 0 ? result.sum / static_cast<double>(result.count) : 0});
                        else if (agg_func->function_name == "COUNT") aggregated_doc.elements.push_back(TissDB::Element{result_key, static_cast<double>(result.count)});
                        else if (agg_func->function_name == "MIN") {
                            if (result.min_str.has_value()) {
                                aggregated_doc.elements.push_back(TissDB::Element{result_key, result.min_str.value()});
                            } else {
                                aggregated_doc.elements.push_back(TissDB::Element{result_key, result.min.value_or(0)});
                            }
                        }
                        else if (agg_func->function_name == "MAX") {
                            if (result.max_str.has_value()) {
                                aggregated_doc.elements.push_back(TissDB::Element{result_key, result.max_str.value()});
                            } else {
                                aggregated_doc.elements.push_back(TissDB::Element{result_key, result.max.value_or(0)});
                            }
                        }
                        else if (agg_func->function_name == "STDDEV") {
                            if (result.count > 0) {
                                double mean = result.sum / static_cast<double>(result.count);
                                double variance = (result.sum_sq / static_cast<double>(result.count)) - (mean * mean);
                                aggregated_doc.elements.push_back(TissDB::Element{result_key, sqrt(variance)});
                            } else {
                                aggregated_doc.elements.push_back(TissDB::Element{result_key, 0.0});
                            }
                        }
                    }
                }
                aggregated_docs.push_back(aggregated_doc);
            }
        } else {
            Document aggregated_doc;
            aggregated_doc.id = "aggregate";
            std::map<std::string, AggregateResult> group_results;
            for (const auto& field : select_stmt.fields) {
                if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                    std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                    for (const auto& doc : filtered_docs) {
                       process_aggregation(group_results, result_key, doc, *agg_func);
                    }
                }
            }
             for (const auto& field : select_stmt.fields) {
                if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                    std::string result_key = agg_func->function_name + "(" + agg_func->field_name + ")";
                    const auto& result = group_results.at(result_key);
                    if (agg_func->function_name == "SUM") aggregated_doc.elements.push_back(TissDB::Element{result_key, result.sum});
                    else if (agg_func->function_name == "AVG") aggregated_doc.elements.push_back(TissDB::Element{result_key, result.count > 0 ? result.sum / static_cast<double>(result.count) : 0});
                    else if (agg_func->function_name == "COUNT") aggregated_doc.elements.push_back(TissDB::Element{result_key, static_cast<double>(result.count)});
                    else if (agg_func->function_name == "MIN") {
                        if (result.min_str.has_value()) {
                            aggregated_doc.elements.push_back(TissDB::Element{result_key, result.min_str.value()});
                        } else {
                            aggregated_doc.elements.push_back(TissDB::Element{result_key, result.min.value_or(0)});
                        }
                    }
                    else if (agg_func->function_name == "MAX") {
                        if (result.max_str.has_value()) {
                            aggregated_doc.elements.push_back(TissDB::Element{result_key, result.max_str.value()});
                        } else {
                            aggregated_doc.elements.push_back(TissDB::Element{result_key, result.max.value_or(0)});
                        }
                    }
                    else if (agg_func->function_name == "STDDEV") {
                        if (result.count > 0) {
                            double mean = result.sum / static_cast<double>(result.count);
                            double variance = (result.sum_sq / static_cast<double>(result.count)) - (mean * mean);
                            aggregated_doc.elements.push_back(TissDB::Element{result_key, sqrt(variance)});
                        } else {
                            aggregated_doc.elements.push_back(TissDB::Element{result_key, 0.0});
                        }
                    }
                }
            }
            aggregated_docs.push_back(aggregated_doc);
        }
        return {aggregated_docs};
    }

    // --- Projection ---
    bool select_all = !select_stmt.fields.empty() && std::holds_alternative<std::string>(select_stmt.fields[0]) && std::get<std::string>(select_stmt.fields[0]) == "*";

    if (select_all) {
        return {filtered_docs};
    } else {
        std::vector<Document> projected_docs;
        for (const auto& doc : filtered_docs) {
            Document projected_doc;
            projected_doc.id = doc.id;
            for (const auto& field_variant : select_stmt.fields) {
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
}

} // namespace Query
} // namespace TissDB
