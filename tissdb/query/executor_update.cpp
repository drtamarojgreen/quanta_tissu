#include "executor_update.h"
#include "executor_common.h"
#include <algorithm>

namespace TissDB {
namespace Query {

QueryResult execute_update_statement(Storage::LSMTree& storage_engine, UpdateStatement update_stmt) {
    auto all_docs = storage_engine.scan(update_stmt.collection_name);
    int updated_count = 0;

    for (auto& doc : all_docs) {
        bool should_update = false;
        if (update_stmt.where_clause) {
            if (evaluate_expression(*update_stmt.where_clause, doc)) {
                should_update = true;
            }
        } else {
            should_update = true; // No WHERE clause, update all documents
        }

        if (should_update) {
            Document original_doc = doc; // Copy original document for expression evaluation
            for (const auto& set_pair : update_stmt.set_clause) {
                const std::string& field_to_update = set_pair.first;
                const Expression& value_expr = set_pair.second;

                // Evaluate the expression based on the original document state
                Literal new_value = evaluate_update_expression(value_expr, original_doc);

                auto it = std::find_if(doc.elements.begin(), doc.elements.end(),
                                       [&](const Element& elem) { return elem.key == field_to_update; });

                if (it != doc.elements.end()) {
                    // Field exists, update it
                    if (const auto* str_val = std::get_if<std::string>(&new_value)) {
                        it->value = *str_val;
                    } else if (const auto* num_val = std::get_if<double>(&new_value)) {
                        it->value = *num_val;
                    } else if (const auto* bool_val = std::get_if<bool>(&new_value)) {
                        it->value = *bool_val;
                    } else if (std::get_if<Null>(&new_value)) {
                        it->value = nullptr;
                    }
                } else {
                    // Field does not exist, add it
                    Element new_element;
                    new_element.key = field_to_update;
                    if (const auto* str_val = std::get_if<std::string>(&new_value)) {
                        new_element.value = *str_val;
                    } else if (const auto* num_val = std::get_if<double>(&new_value)) {
                        new_element.value = *num_val;
                    } else if (const auto* bool_val = std::get_if<bool>(&new_value)) {
                        new_element.value = *bool_val;
                    } else if (std::get_if<Null>(&new_value)) {
                        new_element.value = nullptr;
                    }
                    doc.elements.push_back(new_element);
                }
            }
            storage_engine.put(update_stmt.collection_name, doc.id, doc);
            updated_count++;
        }
    }

    Document result_doc;
    result_doc.id = "summary";
    result_doc.elements.push_back({"updated_count", (double)updated_count});
    return {result_doc};
}

} // namespace Query
} // namespace TissDB
