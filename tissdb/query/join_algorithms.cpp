
#include "join_algorithms.h"
#include "executor_common.h" // For combine_documents
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <optional>

namespace TissDB {
namespace Query {

// Helper to get a value from a document by key
static std::optional<Value> get_value_by_key(const Document& doc, const std::string& key) {
    for (const auto& elem : doc.elements) {
        if (elem.key == key) {
            return elem.value;
        }
    }
    return std::nullopt;
}


// Implementation for Nested Loop Join
std::vector<Document> JoinAlgorithms::nestedLoopJoin(
    const std::vector<Document>& left_table,
    const std::vector<Document>& right_table,
    const std::string& left_join_key,
    const std::string& right_join_key
) {
    std::vector<Document> result;
    for (const auto& left_doc : left_table) {
        auto left_value_opt = get_value_by_key(left_doc, left_join_key);
        if (!left_value_opt) continue;

        for (const auto& right_doc : right_table) {
            auto right_value_opt = get_value_by_key(right_doc, right_join_key);
            if (!right_value_opt) continue;

            if (*left_value_opt == *right_value_opt) {
                result.push_back(combine_documents(left_doc, right_doc));
            }
        }
    }
    return result;
}



// Implementation for Hash Join
std::vector<Document> JoinAlgorithms::hashJoin(
    const std::vector<Document>& left_table,
    const std::vector<Document>& right_table,
    const std::string& left_join_key,
    const std::string& right_join_key
) {
    std::vector<Document> result;
    std::unordered_map<std::string, std::vector<Document>> hash_table;

    // Build phase
    for (const auto& right_doc : right_table) {
        auto right_value_opt = get_value_by_key(right_doc, right_join_key);
        if (right_value_opt) {
            hash_table[value_to_string(*right_value_opt)].push_back(right_doc);
        }
    }

    // Probe phase
    for (const auto& left_doc : left_table) {
        auto left_value_opt = get_value_by_key(left_doc, left_join_key);
        if (left_value_opt) {
            auto it = hash_table.find(value_to_string(*left_value_opt));
            if (it != hash_table.end()) {
                for (const auto& right_doc : it->second) {
                    result.push_back(combine_documents(left_doc, right_doc));
                }
            }
        }
    }

    return result;
}

// Implementation for Sort-Merge Join
std::vector<Document> JoinAlgorithms::sortMergeJoin(
    std::vector<Document> left_table,
    std::vector<Document> right_table,
    const std::string& left_join_key,
    const std::string& right_join_key
) {
    auto sort_comparator = [&](const std::string& key) {
        return [&](const Document& a, const Document& b) {
            auto val_a_opt = get_value_by_key(a, key);
            auto val_b_opt = get_value_by_key(b, key);
            if (!val_a_opt.has_value()) return true;
            if (!val_b_opt.has_value()) return false;
            return value_to_string(*val_a_opt) < value_to_string(*val_b_opt);
        };
    };

    std::sort(left_table.begin(), left_table.end(), sort_comparator(left_join_key));
    std::sort(right_table.begin(), right_table.end(), sort_comparator(right_join_key));

    std::vector<Document> result;
    size_t i = 0, j = 0;
    while (i < left_table.size() && j < right_table.size()) {
        auto left_key = value_to_string(get_value_by_key(left_table[i], left_join_key).value_or(""));
        auto right_key = value_to_string(get_value_by_key(right_table[j], right_join_key).value_or(""));

        if (left_key < right_key) {
            i++;
        } else if (right_key < left_key) {
            j++;
        } else {
            size_t j_start = j;
            while (j < right_table.size() && value_to_string(get_value_by_key(right_table[j], right_join_key).value_or("")) == left_key) {
                result.push_back(combine_documents(left_table[i], right_table[j]));
                j++;
            }
            i++;
            if (i < left_table.size() && value_to_string(get_value_by_key(left_table[i], left_join_key).value_or("")) == left_key) {
                 j = j_start;
            }
        }
    }
    return result;
}

} // namespace Query
} // namespace TissDB
