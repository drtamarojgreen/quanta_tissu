#include "executor_insert.h"
#include <stdexcept>
#include <chrono>
#include <random>
#include <type_traits>

namespace TissDB {
namespace Query {

QueryResult execute_insert_statement(Storage::LSMTree& storage_engine, InsertStatement insert_stmt) {
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
        const auto& value = insert_stmt.values[i];
        Element new_element;
        new_element.key = col_name;
        std::visit([&new_element](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Null>) {
                new_element.value = nullptr;
            } else {
                new_element.value = arg;
            }
        }, value);
        new_doc.elements.push_back(std::move(new_element));
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
