#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <chrono>

namespace TissDB {

// Forward declaration for recursive Element structure
class Element;

// Primitive types supported by TissDB
using Number = double; // Using double to handle both integer and float values
using Boolean = bool;
using DateTime = std::chrono::time_point<std::chrono::system_clock>;
using BinaryData = std::vector<uint8_t>;

// A list of primitive values. Note: nested lists are not supported.
using PrimitiveList = std::vector<std::variant<std::string, Number, Boolean>>;

// An Element's value can be a single primitive, a list of primitives, or a list of nested Elements.
using Value = std::variant<
    std::string,
    Number,
    Boolean,
    DateTime,
    BinaryData,
    PrimitiveList,
    std::vector<Element> // For nested elements like <specs> or <reviews> in the example
>;

// An Element is a key-value pair, representing a tag in the document.
class Element {
public:
    std::string key;
    Value value;
};

// A Document is the top-level object, identified by an ID and containing a collection of root elements.
class Document {
public:
    std::string id;
    std::vector<Element> elements;
};

} // namespace TissDB
