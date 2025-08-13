
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

// Helper to write any plain-old-data type in binary.
template<typename T>
void write_binary(std::ostream& os, const T& value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

// Helper to read any plain-old-data type in binary.
template<typename T>
void read_binary(std::istream& is, T& value) {
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
}

// Helper to write a std::string (length-prefixed).
void write_string(std::ostream& os, const std::string& str) {
    size_t len = str.size();
    write_binary(os, len);
    os.write(str.data(), len);
}

// Helper to read a std::string (length-prefixed).
std::string read_string(std::istream& is) {
    size_t len;
    read_binary(is, len);
    if (len > 0) {
        std::string str(len, '\0');
        is.read(&str[0], len);
        return str;
    }
    return "";
}

// Forward declarations for recursive serialization/deserialization.
void serialize_value(std::ostream& os, const Value& value);
Value deserialize_value(std::istream& is);
void serialize_element(std::ostream& os, const Element& element);
Element deserialize_element(std::istream& is);

// Serializes a Value variant.
void serialize_value(std::ostream& os, const Value& value) {
    std::visit([&os](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            write_binary(os, DataType::STRING);
            write_string(os, arg);
        } else if constexpr (std::is_same_v<T, Number>) {
            write_binary(os, DataType::NUMBER);
            write_binary(os, arg);
        } else if constexpr (std::is_same_v<T, Boolean>) {
            write_binary(os, DataType::BOOLEAN);
            write_binary(os, arg);
        } else if constexpr (std::is_same_v<T, DateTime>) {
            write_binary(os, DataType::DATETIME);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(arg.time_since_epoch()).count();
            write_binary(os, seconds);
        } else if constexpr (std::is_same_v<T, BinaryData>) {
            write_binary(os, DataType::BINARY_DATA);
            size_t len = arg.size();
            write_binary(os, len);
            os.write(reinterpret_cast<const char*>(arg.data()), len);
        } else if constexpr (std::is_same_v<T, std::vector<Element>>) {
            write_binary(os, DataType::ELEMENT_LIST);
            size_t count = arg.size();
            write_binary(os, count);
            for(const auto& elem : arg) {
                serialize_element(os, elem);
            }
        }
    }, value);
}

// Serializes a single Element.
void serialize_element(std::ostream& os, const Element& element) {
    write_string(os, element.key);
    serialize_value(os, element.value);
}

// Deserializes a Value variant.
Value deserialize_value(std::istream& is) {
    DataType type;
    read_binary(is, type);
    switch (type) {
        case DataType::STRING:
            return read_string(is);
        case DataType::NUMBER: {
            Number val;
            read_binary(is, val);
            return val;
        }
        case DataType::BOOLEAN: {
            Boolean val;
            read_binary(is, val);
            return val;
        }
        case DataType::DATETIME: {
            long long seconds_count;
            read_binary(is, seconds_count);
            return DateTime(std::chrono::seconds(seconds_count));
        }
        case DataType::BINARY_DATA: {
            size_t len;
            read_binary(is, len);
            BinaryData data(len);
            is.read(reinterpret_cast<char*>(data.data()), len);
            return data;
        }
        case DataType::ELEMENT_LIST: {
            size_t count;
            read_binary(is, count);
            std::vector<Element> elements;
            elements.reserve(count);
            for(size_t i = 0; i < count; ++i) {
                elements.push_back(deserialize_element(is));
            }
            return elements;
        }
        default:
            throw std::runtime_error("Unknown data type in stream during deserialization.");
    }
}

// Deserializes a single Element.
Element deserialize_element(std::istream& is) {
    Element elem;
    elem.key = read_string(is);
    elem.value = deserialize_value(is);
    return elem;
}

} // anonymous namespace

// Public interface for serializing a Document.
std::vector<uint8_t> serialize(const Document& doc) {
    std::stringstream ss(std::ios::binary | std::ios::out);
    write_string(ss, doc.id);
    size_t element_count = doc.elements.size();
    write_binary(ss, element_count);
    for (const auto& elem : doc.elements) {
        serialize_element(ss, elem);
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
    Document doc;
    doc.id = read_string(ss);
    size_t element_count;
    read_binary(ss, element_count);
    doc.elements.reserve(element_count);
    for (size_t i = 0; i < element_count; ++i) {
        doc.elements.push_back(deserialize_element(ss));
    }
    return doc;
}

}
