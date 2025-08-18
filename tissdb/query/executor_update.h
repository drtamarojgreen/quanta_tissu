#pragma once

#include "ast.h"
#include "executor.h"
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

QueryResult execute_update_statement(Storage::LSMTree& storage_engine, const UpdateStatement& update_stmt);

} // namespace Query
} // namespace TissDB
