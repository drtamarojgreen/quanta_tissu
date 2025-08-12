#pragma once

#include "document.h"
#include <vector>
#include <cstdint>

namespace TissDB {

/**
 * @brief Serializes a Document object into a byte vector using MessagePack.
 *
 * @param doc The Document to serialize.
 * @return A vector of bytes representing the serialized document.
 */
std::vector<uint8_t> serialize(const Document& doc);

/**
 * @brief Deserializes a byte vector into a Document object.
 *
 * @param bytes The byte vector to deserialize.
 * @return The deserialized Document object.
 */
Document deserialize(const std::vector<uint8_t>& bytes);

} // namespace TissDB
