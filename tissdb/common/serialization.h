#pragma once

#include "document.h"
#include "binary_stream_buffer.h" // Include the new header
#include <vector>

namespace TissDB {

// Serializes a document to a byte vector.
std::vector<uint8_t> serialize(const Document& doc);

// Deserializes a document from a byte vector.
Document deserialize(const std::vector<uint8_t>& bytes);

// Serializes a schema to a byte vector.
std::vector<uint8_t> serialize(const Schema& schema);

// Deserializes a schema from a byte vector.
Schema deserialize_schema(const std::vector<uint8_t>& bytes);

} // namespace TissDB
