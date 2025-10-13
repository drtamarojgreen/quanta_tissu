#include "executor_select.h"
#include "executor_common.h"
#include "../common/checksum.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iomanip>

namespace TissDB {
namespace Query {

// Visitor for creating a string representation of a Value variant for group by keys.
struct GroupKeyVisitor {
    std::stringstream& ss;
    void operator()(const std::string& s) const { ss << s; }
    void operator()(const Number& n) const { ss << n; }
    void operator()(const Boolean& b) const { ss << (b ? "true" : "false"); }
    void operator()(const Date& d) const {
        ss << std::setfill('0') << std::setw(4) << d.year << "-"
           << std::setw(2) << static_cast<int>(d.month) << "-"
           << std::setw(2) << static_cast<int>(d.day);
    }
    void operator()(const Time& t) const {
        ss << std::setfill('0') << std::setw(2) << static_cast<int>(t.hour) << ":"
           << std::setw(2) << static_cast<int>(t.minute) << ":"
           << std::setw(2) << static_cast<int>(t.second);
    }
    void operator()(const DateTime& dt) const {
        ss << std::chrono::duration_cast<std::chrono::milliseconds>(dt.time_since_epoch()).count();
    }
    void operator()(const TissDB::Timestamp& ts) const {
        ss << ts.microseconds_since_epoch_utc;
    }
    void operator()(const BinaryData& /*bd*/) const {
        // Note: Grouping by binary data is tricky.
        // A proper implementation might hash the data.
        ss << "hash:" << TissDB::Common::crc32(bd.data(), bd.size());
    }
    void operator()(const std::vector<TissDB::Element>& elements) const {
        // Note: Grouping by a whole sub-document is also tricky.
        // A proper implementation might serialize and hash.
        // For now, we'll use a placeholder.
        ss << "[sub_document]";
    }
    void operator()(std::nullptr_t) const { ss << "null"; }
    void operator()(const std::shared_ptr<TissDB::Array>&) const { ss << "[array]"; }
    void operator()(const std::shared_ptr<TissDB::Object>&) const { ss << "[object]"; }
};

// Helper to create a string representation for an aggregate function, e.g., "COUNT(field)" or "COUNT(*)"
std::string get_aggregate_result_key(const AggregateFunction& agg_func) {
    std::string key;
    switch (agg_func.type) {
        case AggregateType::COUNT: key = "COUNT"; break;
        case AggregateType::AVG:   key = "AVG";   break;
        case AggregateType::SUM:   key = "SUM";   break;
        case AggregateType::MIN:   key = "MIN";   break;
        case AggregateType::MAX:   key = "MAX";   break;
    }
    key += "(";
    if (agg_func.field_name.has_value()) {
        key += agg_func.field_name.value();
    } else {
        key += "*";
    }
    key += ")";
    return key;
}

QueryResult execute_select_statement(Storage::LSMTree& storage_engine, const SelectStatement& select_stmt, const std::vector<Literal>& params) {
    // --- UNION Operation ---
    if (select_stmt.union_clause) {
        // Recursively execute the left and right select statements
        auto left_result = execute_select_statement(storage_engine, *select_stmt.union_clause->left_select, params);
        auto right_result = execute_select_statement(storage_engine, *select_stmt.union_clause->right_select, params);

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
        std::vector<Document> joined_docs;

        if (join_clause.type == JoinType::CROSS) {
            std::vector<Document> right_docs = storage_engine.scan(join_clause.collection_name);
            for (const auto& left_doc : all_docs) {
                for (const auto& right_doc : right_docs) {
                    joined_docs.push_back(combine_documents(left_doc, right_doc));
                }
            }
        } else {
            // Improved join logic with index lookup
            const auto* on_cond = std::get_if<std::shared_ptr<BinaryExpression>>(&join_clause.on_condition);
            std::string left_key, right_key;

            if (on_cond && (*on_cond)->op == "=") {
                if (const auto* left_ident = std::get_if<Identifier>(&(*on_cond)->left)) {
                    left_key = left_ident->name;
                }
                if (const auto* right_ident = std::get_if<Identifier>(&(*on_cond)->right)) {
                    right_key = right_ident->name;
                }
            }

            bool can_use_index = !left_key.empty() && !right_key.empty() && storage_engine.has_index(join_clause.collection_name, {right_key});

            for (const auto& left_doc : all_docs) {
                bool left_doc_matched = false;
                std::vector<Document> right_docs_to_join;

                if (can_use_index) {
                    const auto* left_val_ptr = get_value_from_doc(left_doc, left_key);
                    if (left_val_ptr) {
                        std::string val_str = value_to_string(*left_val_ptr);
                        auto doc_ids = storage_engine.find_by_index(join_clause.collection_name, {right_key}, {val_str});
                        right_docs_to_join = storage_engine.get_many(join_clause.collection_name, doc_ids);
                    }
                } else {
                    right_docs_to_join = storage_engine.scan(join_clause.collection_name);
                }

                for (const auto& right_doc : right_docs_to_join) {
                    if (evaluate_expression(join_clause.on_condition, combine_documents(left_doc, right_doc), params)) {
                        joined_docs.push_back(combine_documents(left_doc, right_doc));
                        left_doc_matched = true;
                    }
                }

                if (!left_doc_matched && (join_clause.type == JoinType::LEFT || join_clause.type == JoinType::FULL)) {
                    joined_docs.push_back(left_doc);
                }
            }

            if (join_clause.type == JoinType::RIGHT || join_clause.type == JoinType::FULL) {
                std::vector<Document> right_docs = storage_engine.scan(join_clause.collection_name);
                for (const auto& right_doc : right_docs) {
                    bool right_doc_matched = false;
                    for (const auto& left_doc : all_docs) {
                        if (evaluate_expression(join_clause.on_condition, combine_documents(left_doc, right_doc), params)) {
                            right_doc_matched = true;
                            break;
                        }
                    }
                    if (!right_doc_matched) {
                        joined_docs.push_back(right_doc);
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
            if (evaluate_expression(*select_stmt.where_clause, doc, params)) {
                filtered_docs.push_back(doc);
            }
        }
    } else {
        filtered_docs = all_docs;
    }

    // --- Aggregation and Grouping ---
    bool has_aggregate = std::any_of(select_stmt.fields.begin(), select_stmt.fields.end(),
                                     [](const auto& field){ return std::holds_alternative<AggregateFunction>(field); });

    if (has_aggregate || !select_stmt.group_by_clause.empty()) {
        std::vector<Document> aggregated_docs;
        // --- GROUP BY flow ---
        if (!select_stmt.group_by_clause.empty()) {
            std::map<std::string, std::vector<Document>> grouped_docs;
            for (const auto& doc : filtered_docs) {
                std::stringstream group_key_ss;
                for (size_t i = 0; i < select_stmt.group_by_clause.size(); ++i) {
                    const auto& field_name = select_stmt.group_by_clause[i];
                    const auto* val_ptr = get_value_from_doc(doc, field_name);
                    if (val_ptr) {
                        std::visit(GroupKeyVisitor{group_key_ss}, *val_ptr);
                    } else {
                        group_key_ss << "NULL";
                    }
                    if (i < select_stmt.group_by_clause.size() - 1) {
                        group_key_ss << "::";
                    }
                }
                grouped_docs[group_key_ss.str()].push_back(doc);
            }

            for (auto const& [group_key, docs] : grouped_docs) {
                Document aggregated_doc;
                aggregated_doc.id = group_key;
                std::map<std::string, AggregateResult> group_results;

                // Add the group by fields to the result doc from the first doc in the group
                if (!docs.empty()) {
                    const auto& first_doc = docs.front();
                    for (const auto& field_name : select_stmt.group_by_clause) {
                        if (const auto* val_ptr = get_value_from_doc(first_doc, field_name)) {
                            aggregated_doc.elements.push_back({field_name, *val_ptr});
                        }
                    }
                }

                // Process aggregations for the group
                for (const auto& field : select_stmt.fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                        std::string result_key = get_aggregate_result_key(*agg_func);
                        for (const auto& doc : docs) {
                            process_aggregation(group_results, result_key, doc, *agg_func);
                        }
                    }
                }

                // Finalize and add results to the aggregated doc
                for (const auto& field : select_stmt.fields) {
                    if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                        std::string result_key = get_aggregate_result_key(*agg_func);
                        const auto& result = group_results.at(result_key);
                        switch(agg_func->type) {
                            case AggregateType::SUM:
                                aggregated_doc.elements.push_back({result_key, result.sum});
                                break;
                            case AggregateType::AVG:
                                aggregated_doc.elements.push_back({result_key, result.avg_count > 0 ? result.sum / static_cast<double>(result.avg_count) : 0.0});
                                break;
                            case AggregateType::COUNT:
                                aggregated_doc.elements.push_back({result_key, static_cast<double>(result.count)});
                                break;
                            case AggregateType::MIN:
                                if (result.min_str.has_value()) {
                                    aggregated_doc.elements.push_back({result_key, result.min_str.value()});
                                } else {
                                    aggregated_doc.elements.push_back({result_key, result.min.value_or(0.0)});
                                }
                                break;
                            case AggregateType::MAX:
                                if (result.max_str.has_value()) {
                                    aggregated_doc.elements.push_back({result_key, result.max_str.value()});
                                } else {
                                    aggregated_doc.elements.push_back({result_key, result.max.value_or(0.0)});
                                }
                                break;
                        }
                    }
                }
                aggregated_docs.push_back(aggregated_doc);
            }
        } else { // --- Aggregation without GROUP BY ---
            Document aggregated_doc;
            aggregated_doc.id = "aggregate";
            std::map<std::string, AggregateResult> group_results;

            for (const auto& field : select_stmt.fields) {
                if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                    std::string result_key = get_aggregate_result_key(*agg_func);
                    for (const auto& doc : filtered_docs) {
                       process_aggregation(group_results, result_key, doc, *agg_func);
                    }
                }
            }
             for (const auto& field : select_stmt.fields) {
                if (auto* agg_func = std::get_if<AggregateFunction>(&field)) {
                    std::string result_key = get_aggregate_result_key(*agg_func);
                    const auto& result = group_results.at(result_key);
                     switch(agg_func->type) {
                        case AggregateType::SUM:
                            aggregated_doc.elements.push_back({result_key, result.sum});
                            break;
                        case AggregateType::AVG:
                            aggregated_doc.elements.push_back({result_key, result.avg_count > 0 ? result.sum / static_cast<double>(result.avg_count) : 0.0});
                            break;
                        case AggregateType::COUNT:
                            aggregated_doc.elements.push_back({result_key, static_cast<double>(result.count)});
                            break;
                        case AggregateType::MIN:
                            if (result.min_str.has_value()) {
                                aggregated_doc.elements.push_back({result_key, result.min_str.value()});
                            } else {
                                aggregated_doc.elements.push_back({result_key, result.min.value_or(0.0)});
                            }
                            break;
                        case AggregateType::MAX:
                            if (result.max_str.has_value()) {
                                aggregated_doc.elements.push_back({result_key, result.max_str.value()});
                            } else {
                                aggregated_doc.elements.push_back({result_key, result.max.value_or(0.0)});
                            }
                            break;
                    }
                }
            }
            aggregated_docs.push_back(aggregated_doc);
        }
        result_docs = aggregated_docs;
    } else {
        result_docs = filtered_docs;
    }

    // --- Sorting ---
    if (!select_stmt.order_by_clause.empty()) {
        std::sort(result_docs.begin(), result_docs.end(), [&](const Document& a, const Document& b) {
            for (const auto& order_by_pair : select_stmt.order_by_clause) {
                const std::string& field_name = order_by_pair.first;
                const std::string& sort_order = order_by_pair.second;

                auto get_value = [&](const Document& doc) -> std::optional<Value> {
                    for (const auto& elem : doc.elements) {
                        if (elem.key == field_name) {
                            return elem.value;
                        }
                    }
                    return std::nullopt;
                };

                auto val_a_opt = get_value(a);
                auto val_b_opt = get_value(b);

                if (!val_a_opt.has_value() || !val_b_opt.has_value()) {
                    // Handle missing fields by treating them as equal for this level of sorting
                    continue;
                }

                auto& val_a = *val_a_opt;
                auto& val_b = *val_b_opt;

                bool is_asc = (sort_order != "DESC");

                // If types are different, they are considered "equal" for sorting purposes
                if (val_a.index() != val_b.index()) {
                    continue;
                }

                if (std::get_if<std::string>(&val_a)) {
                    int cmp = std::get<std::string>(val_a).compare(std::get<std::string>(val_b));
                    if (cmp != 0) return is_asc ? cmp < 0 : cmp > 0;
                } else if (std::get_if<double>(&val_a)) {
                    double diff = std::get<double>(val_a) - std::get<double>(val_b);
                    if (diff != 0) return is_asc ? diff < 0 : diff > 0;
                }
            }
            return false; // Equal
        });
    }

    // --- Projection ---
    bool select_all = !select_stmt.fields.empty() && std::holds_alternative<std::string>(select_stmt.fields[0]) && std::get<std::string>(select_stmt.fields[0]) == "*";

    if (select_all) {
        return {result_docs};
    } else {
        std::vector<Document> projected_docs;
        for (const auto& doc : result_docs) {
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
