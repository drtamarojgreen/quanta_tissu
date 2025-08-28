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
    ELEMENT_LIST,
    ARRAY,
    OBJECT
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
        } else if constexpr (std::is_same_v<T, std::shared_ptr<Array>>) {
            bsb.write(DataType::ARRAY);
            if (arg) {
                bsb.write(true); // Mark as not null
                bsb.write(arg->values.size());
                for (const auto& val : arg->values) {
                    serialize_value(bsb, val);
                }
            } else {
                bsb.write(false); // Mark as null
            }
        } else if constexpr (std::is_same_v<T, std::shared_ptr<Object>>) {
            bsb.write(DataType::OBJECT);
            if (arg) {
                bsb.write(true); // Mark as not null
                bsb.write(arg->values.size());
                for (const auto& pair : arg->values) {
                    bsb.write_string(pair.first);
                    serialize_value(bsb, pair.second);
                }
            } else {
                bsb.write(false); // Mark as null
            }
        } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
            // This case should ideally be handled by the default Json::JsonValue(nullptr) in value_to_json
            // but including it for completeness in serialization.
            // For now, we'll just write a null type if it somehow gets here.
            // This might need a dedicated DataType::NULL_TYPE if we want to distinguish it.
            // For now, we'll just skip writing anything for nullptr_t if it's part of a larger variant.
            // Or, we could throw an error if we expect all Value types to be explicitly handled.
            // For now, let's assume it won't reach here for nullptr_t if it's part of a larger variant.
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
        case DataType::ARRAY: {
            bool is_not_null;
            bsb.read(is_not_null);
            if (!is_not_null) return std::shared_ptr<Array>(nullptr);

            size_t count;
            bsb.read(count);
            auto arr = std::make_shared<Array>();
            arr->values.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                arr->values.push_back(deserialize_value(bsb));
            }
            return arr;
        }
        case DataType::OBJECT: {
            bool is_not_null;
            bsb.read(is_not_null);
            if (!is_not_null) return std::shared_ptr<Object>(nullptr);

            size_t count;
            bsb.read(count);
            auto obj = std::make_shared<Object>();
            for (size_t i = 0; i < count; ++i) {
                std::string key = bsb.read_string();
                obj->values[key] = deserialize_value(bsb);
            }
            return obj;
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

// Serializes a schema to a byte vector.
std::vector<uint8_t> serialize(const Schema& schema) {
    std::stringstream ss(std::ios::binary | std::ios::out);
    BinaryStreamBuffer bsb(static_cast<std::ostream&>(ss));

    // Serialize fields
    size_t field_count = schema.get_fields().size();
    bsb.write(field_count);
    for (const auto& field : schema.get_fields()) {
        bsb.write_string(field.name);
        bsb.write(static_cast<uint8_t>(field.type));
        bsb.write(field.required);
        bsb.write(field.unique);
    }

    // Serialize primary key
    bsb.write_string(schema.get_primary_key());

    // Serialize foreign keys
    size_t fk_count = schema.get_foreign_keys().size();
    bsb.write(fk_count);
    for (const auto& fk : schema.get_foreign_keys()) {
        bsb.write_string(fk.field_name);
        bsb.write_string(fk.referenced_collection);
        bsb.write_string(fk.referenced_field);
    }

    const std::string& str = ss.str();
    return std::vector<uint8_t>(str.begin(), str.end());
}

// Deserializes a schema from a byte vector.
Schema deserialize_schema(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) {
        return Schema{};
    }
    std::string byte_string(bytes.begin(), bytes.end());
    std::stringstream ss(byte_string, std::ios::binary | std::ios::in);
    BinaryStreamBuffer bsb(static_cast<std::istream&>(ss));

    Schema schema;

    // Deserialize fields
    size_t field_count;
    bsb.read(field_count);
    for (size_t i = 0; i < field_count; ++i) {
        std::string name = bsb.read_string();
        uint8_t type_val;
        bsb.read(type_val);
        FieldType type = static_cast<FieldType>(type_val);
        bool required;
        bsb.read(required);
        bool unique;
        bsb.read(unique);
        schema.add_field(name, type, required, unique);
    }

    // Deserialize primary key
    std::string pk = bsb.read_string();
    schema.set_primary_key(pk);

    // Deserialize foreign keys
    size_t fk_count;
    bsb.read(fk_count);
    for (size_t i = 0; i < fk_count; ++i) {
        std::string field_name = bsb.read_string();
        std::string ref_coll = bsb.read_string();
        std::string ref_field = bsb.read_string();
        schema.add_foreign_key(field_name, ref_coll, ref_field);
    }

    return schema;
}

} // namespace TissDB