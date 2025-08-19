#pragma once

#include <string>
#include <vector>
#include "document.h"

namespace TissDB {
namespace Transactions {

// Defines the type of operation within a transaction.
enum class OperationType {
    PUT,
    DELETE
};

// Represents a single operation (PUT or DELETE) within a transaction.
struct Operation {
    OperationType type;
    std::string collection_name;
    std::string key;
    Document doc; // Used for PUT operations.
};

} // namespace Transactions
} // namespace TissDB
