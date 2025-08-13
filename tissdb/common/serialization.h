#pragma once

#include "document.h"
#include <vector>

namespace TissDB {

// Serializes a document to a byte vector.
std::vector<uint8_t> serialize(const Document& doc);

// Deserializes a document from a byte vector.
Document deserialize(const std::vector<uint8_t>& bytes);

}