#pragma once

#include "ast.h"
#include "executor.h"
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

QueryResult execute_select_statement(Storage::LSMTree& storage_engine, const SelectStatement& select_stmt, const std::vector<Literal>& params);

} // namespace Query
} // namespace TissDB
