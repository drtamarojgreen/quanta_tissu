#include "executor_insert.h"
#include "executor_common.h"
#include <stdexcept>
#include <chrono>
#include <random>

namespace TissDB {
namespace Query {

QueryResult execute_insert_statement(Storage::LSMTree& storage_engine, InsertStatement insert_stmt, const std::vector<Literal>& params) {
    Document new_doc;

    unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<long long> distrib;
    new_doc.id = std::to_string(distrib(gen));

    if (insert_stmt.columns.size() != insert_stmt.values.size()) {
        throw std::runtime_error("Column count does not match value count.");
    }

    for (size_t i = 0; i < insert_stmt.columns.size(); ++i) {
        const auto& col_name = insert_stmt.columns[i];
        const Expression& value_expr = insert_stmt.values[i];
        Element new_element;
        new_element.key = col_name;

        // Evaluate the expression (handles literals, parameters, functions, etc.)
        Literal resolved_literal = evaluate_update_expression(value_expr, new_doc, params);

        std::visit([&new_element](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, TissDB::Query::Null>) {
                new_element.value = nullptr;
            } else {
                new_element.value = arg;
            }
        }, resolved_literal);
        new_doc.elements.push_back(new_element);
    }

    storage_engine.put(insert_stmt.collection_name, new_doc.id, new_doc);

    Document result_doc;
    result_doc.id = "summary";
    result_doc.elements.push_back({"inserted_count", 1.0});
    result_doc.elements.push_back({"inserted_id", new_doc.id});
    return {result_doc};
}

} // namespace Query
} // namespace TissDB
