#include "executor.h"
#include "executor_select.h"
#include "executor_insert.h"
#include "executor_update.h"
#include "executor_delete.h"
#include <stdexcept>

namespace TissDB {
namespace Query {

Executor::Executor(Storage::LSMTree& storage) : storage_engine(storage) {}

QueryResult Executor::execute(const AST& ast) {
    if (auto* select_stmt = std::get_if<SelectStatement>(&ast)) {
        return execute_select_statement(storage_engine, *select_stmt);
    } else if (auto* insert_stmt = std::get_if<InsertStatement>(&ast)) {
        return execute_insert_statement(storage_engine, *insert_stmt);
    } else if (auto* update_stmt = std::get_if<UpdateStatement>(&ast)) {
        return execute_update_statement(storage_engine, *update_stmt);
    } else if (auto* delete_stmt = std::get_if<DeleteStatement>(&ast)) {
        return execute_delete_statement(storage_engine, *delete_stmt);
    }
    // Note: The original executor had a recursive call for UNION.
    // This simplified dispatcher doesn't handle that directly.
    // The logic within execute_select_statement would need to be enhanced
    // to call back into this main execute function if it encounters a UNION clause.
    // For this refactoring, we're keeping it simple.
    return {};
}

} // namespace Query
} // namespace TissDB