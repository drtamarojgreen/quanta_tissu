#include "executor.h"
#include "executor_select.h"
#include "executor_insert.h"
#include "executor_update.h"
#include "executor_delete.h"
#include <stdexcept>

namespace TissDB {
namespace Query {

Executor::Executor(Storage::LSMTree& storage) : storage_engine(storage) {}

QueryResult Executor::execute(const AST& ast, const std::vector<Literal>& params) {
    if (auto* select_stmt = std::get_if<SelectStatement>(&ast)) {
        return execute_select_statement(storage_engine, *select_stmt, params);
    } else if (auto* insert_stmt = std::get_if<InsertStatement>(&ast)) {
        // INSERT statements do not use the parameter substitution logic in the same way,
        // as their values are already parsed into a list of literals.
        // We could enhance this to allow `INSERT INTO ... VALUES (?, ?)` in the future.
        return execute_insert_statement(storage_engine, *insert_stmt);
    } else if (auto* update_stmt = std::get_if<UpdateStatement>(&ast)) {
        return execute_update_statement(storage_engine, *update_stmt, params);
    } else if (auto* delete_stmt = std::get_if<DeleteStatement>(&ast)) {
        return execute_delete_statement(storage_engine, *delete_stmt, params);
    }
    return {};
}

} // namespace Query
} // namespace TissDB