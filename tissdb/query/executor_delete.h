#pragma once

#include "ast.h"
#include "executor.h"
#include "../storage/lsm_tree.h"

namespace TissDB {
namespace Query {

QueryResult execute_delete_statement(Storage::LSMTree& storage_engine, const DeleteStatement& delete_stmt);

} // namespace Query
} // namespace TissDB
