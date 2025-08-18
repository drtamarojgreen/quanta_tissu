#include "executor_delete.h"
#include "executor_common.h"

namespace TissDB {
namespace Query {

QueryResult execute_delete_statement(Storage::LSMTree& storage_engine, DeleteStatement delete_stmt) {
    auto all_docs = storage_engine.scan(delete_stmt.collection_name);
    int deleted_count = 0;

    for (const auto& doc : all_docs) {
        bool should_delete = false;
        if (delete_stmt.where_clause) {
            if (evaluate_expression(*delete_stmt.where_clause, doc)) {
                should_delete = true;
            }
        } else {
            should_delete = true; // No WHERE clause, delete all documents
        }

        if (should_delete) {
            storage_engine.del(delete_stmt.collection_name, doc.id);
            deleted_count++;
        }
    }

    Document result_doc;
    result_doc.id = "summary";
    result_doc.elements.push_back({"deleted_count", (double)deleted_count});
    return {result_doc};
}

} // namespace Query
} // namespace TissDB
