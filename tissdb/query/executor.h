#pragma once

#include "ast.h"
#include "../common/document.h"
#include <vector>

// Forward declaration for the storage engine
namespace TissDB { namespace Storage { class LSMTree; } }

namespace TissDB {
namespace Query {

// A query result is typically a list of documents.
using QueryResult = std::vector<Document>;

// The Executor is responsible for taking a parsed AST and
// running it against the storage engine to produce a result.
class Executor {
public:
    // The executor needs a reference to the storage engine to fetch data.
    explicit Executor(Storage::LSMTree& storage);

    // Executes a query represented by an AST.
    QueryResult execute(AST ast);

private:
    // A reference to the underlying storage engine.
    Storage::LSMTree& storage_engine;
};

} // namespace Query
} // namespace TissDB
