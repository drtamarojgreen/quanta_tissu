#include "serialization.h"
#include <sstream>
#include <stdexcept>
#include <variant>

namespace TissDB {

// Anonymous namespace for helper functions and types private to this file.
namespace {

// Enum to mark data types in the binary stream.
enum class DataType : uint8_t {
    STRING,
    NUMBER,
    BOOLEAN,
    DATETIME,
    BINARY_DATA,
    ELEMENT_LIST
};

// Forward declarations for recursive serialization/deserialization.
void serialize_value(BinaryStreamBuffer& bsb, const Value& value);
Value deserialize_value(BinaryStreamBuffer& bsb);
void serialize_element(BinaryStreamBuffer& bsb, const Element& element);
Element deserialize_element(BinaryStreamBuffer& bsb);

// Serializes a Value variant.
void serialize_value(BinaryStreamBuffer& bsb, const Value& value) {
    std::visit([&bsb](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            bsb.write(DataType::STRING);
            bsb.write_string(arg);
        } else if constexpr (std::is_same_v<T, Number>) {
            bsb.write(DataType::NUMBER);
            bsb.write(arg);
        } else if constexpr (std::is_same_v<T, Boolean>) {
            bsb.write(DataType::BOOLEAN);
            bsb.write(arg);
        } else if constexpr (std::is_same_v<T, DateTime>) {
            bsb.write(DataType::DATETIME);
            auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(arg.time_since_epoch()).count();
            bsb.write(nanoseconds);
        } else if constexpr (std::is_same_v<T, BinaryData>) {
            bsb.write(DataType::BINARY_DATA);
            bsb.write_bytes(arg);
        } else if constexpr (std::is_same_v<T, std::vector<Element>>) {
            bsb.write(DataType::ELEMENT_LIST);
            size_t count = arg.size();
            bsb.write(count);
            for(const auto& elem : arg) {
                serialize_element(bsb, elem);
            }
        }
    }, value);
}

// Serializes a single Element.
void serialize_element(BinaryStreamBuffer& bsb, const Element& element) {
    bsb.write_string(element.key);
    serialize_value(bsb, element.value);
}

// Deserializes a Value variant.
Value deserialize_value(BinaryStreamBuffer& bsb) {
    DataType type;
    bsb.read(type);
    switch (type) {
        case DataType::STRING:
            return bsb.read_string();
        case DataType::NUMBER: {
            Number val;
            bsb.read(val);
            return val;
        }
        case DataType::BOOLEAN: {
            Boolean val;
            bsb.read(val);
            return val;
        }
        case DataType::DATETIME: {
            long long nanoseconds_count;
            bsb.read(nanoseconds_count);
            return DateTime(std::chrono::nanoseconds(nanoseconds_count));
        }
        case DataType::BINARY_DATA: {
            return bsb.read_bytes();
        }
        case DataType::ELEMENT_LIST: {
            size_t count;
            bsb.read(count);
            std::vector<Element> elements;
            elements.reserve(count);
            for(size_t i = 0; i < count; ++i) {
                elements.push_back(deserialize_element(bsb));
            }
            return elements;
        }
        default:
            throw std::runtime_error("Unknown data type in stream during deserialization.");
    }
}

// Deserializes a single Element.
Element deserialize_element(BinaryStreamBuffer& bsb) {
    Element elem;
    elem.key = bsb.read_string();
    elem.value = deserialize_value(bsb);
    return elem;
}

} // anonymous namespace

// Public interface for serializing a Document.
std::vector<uint8_t> serialize(const Document& doc) {
    std::stringstream ss(std::ios::binary | std::ios::out);
    BinaryStreamBuffer bsb(static_cast<std::ostream&>(ss));

    bsb.write_string(doc.id);
    size_t element_count = doc.elements.size();
    bsb.write(element_count);
    for (const auto& elem : doc.elements) {
        serialize_element(bsb, elem);
    }
    const std::string& str = ss.str();
    return std::vector<uint8_t>(str.begin(), str.end());
}

// Public interface for deserializing a Document.
Document deserialize(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) {
        return Document{};
    }
    std::string byte_string(bytes.begin(), bytes.end());
    std::stringstream ss(byte_string, std::ios::binary | std::ios::in);
    BinaryStreamBuffer bsb(static_cast<std::istream&>(ss));

    Document doc;
    doc.id = bsb.read_string();
    size_t element_count;
    bsb.read(element_count);
    doc.elements.reserve(element_count);
    for (size_t i = 0; i < element_count; ++i) {
        doc.elements.push_back(deserialize_element(bsb));
    }
    return doc;
}

} // namespace TissDB