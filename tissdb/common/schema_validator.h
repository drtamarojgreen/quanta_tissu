#pragma once

#include "document.h"
#include "schema.h"

namespace TissDB {
namespace Storage {
    class Indexer; // Forward declaration
}

class SchemaValidator {
public:
    explicit SchemaValidator(const Storage::Indexer& indexer);
    bool validate(const Document& doc, const Schema& schema) const;

private:
    const Storage::Indexer& indexer_;
};

} // namespace TissDB
