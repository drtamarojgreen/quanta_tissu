#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <chrono>
#include <memory>

namespace TissDB {

class Element;
struct Array;
struct Object;

// Primitive types supported by TissDB
using Number = double; // Using double to handle both integer and float values
using Boolean = bool;
using DateTime = std::chrono::time_point<std::chrono::system_clock>;
using BinaryData = std::vector<uint8_t>;

using Value = std::variant<
    std::nullptr_t,
    std::string,
    Number,
    Boolean,
    DateTime,
    BinaryData,
    std::vector<Element>,
    std::shared_ptr<Array>,
    std::shared_ptr<Object>
>;

bool operator==(const Value& lhs, const Value& rhs);

struct Array {
    std::vector<Value> values;
    bool operator==(const Array& other) const;
};

struct Object {
    std::map<std::string, Value> values;
    bool operator==(const Object& other) const;
};

// An Element is a key-value pair, representing a tag in the document.
class Element {
public:
    std::string key;
    Value value;

    bool operator==(const Element& other) const {
        return key == other.key && value == other.value;
    }
};

// A Document is the top-level object, identified by an ID and containing a collection of root elements.
class Document {
public:
    std::string id;
    std::vector<Element> elements;
    bool is_tombstone() const { return elements.empty() && !id.empty(); }

    bool operator==(const Document& other) const {
        return id == other.id && elements == other.elements;
    }
};

} // namespace TissDB