#include "executor_select.h"
#include "executor_common.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace TissDB {
namespace Query {

QueryResult execute_select_statement(Storage::LSMTree& storage_engine, const SelectStatement& select_stmt) {
    // --- UNION Operation ---
    if (select_stmt.union_clause) {
        // This part is tricky because it requires re-executing queries.
        // For this refactoring, we'll assume the main `execute` function handles the recursion.
        // This is a simplification for now.
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
        if (join_clause.type == JoinType::INNER) {
            for (const auto& left_doc : all_docs) {
                for (const auto& right_doc : right_docs) {
                    if (evaluate_expression(join_clause.on_condition, combine_documents(left_doc, right_doc))) {
                        joined_docs.push_back(combine_documents(left_doc, right_doc));
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
                for (const auto& field_name : select_stmt.group_by_clause) {
                    for (const auto& elem : doc.elements) {
                        if (elem.key == field_name) {
                            std::visit([&group_key_ss](auto&& arg) { group_key_ss << arg << "-"; }, elem.value);
                        }
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
