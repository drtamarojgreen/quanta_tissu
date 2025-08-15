#pragma once

#include "document.h"
#include "schema.h"

namespace TissDB {

class SchemaValidator {
public:
    static bool validate(const Document& doc, const Schema& schema);
};

} // namespace TissDB
