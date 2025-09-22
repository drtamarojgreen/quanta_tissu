#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>

namespace NexusFlow {
namespace Json {

// Forward declaration for recursive data structures
class JsonValue;

// Define the types that make up a JSON structure
using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;
using JsonNull = std::nullptr_t;

// A JsonValue can be any one of the valid JSON types.
// We use std::variant to represent this polymorphic nature.
class JsonValue {
public:
    using ValueType = std::variant<
        JsonNull,
        bool,
        double,
        std::string,
        JsonArray,
        JsonObject
    >;

    // Constructors
    JsonValue() : value_(JsonNull{}) {}
    JsonValue(JsonNull val) : value_(val) {}
    JsonValue(bool val) : value_(val) {}
    JsonValue(double val) : value_(val) {}
    JsonValue(const char* val) : value_(std::string(val)) {}
    JsonValue(const std::string& val) : value_(val) {}
    JsonValue(const JsonArray& val) : value_(val) {}
    JsonValue(const JsonObject& val) : value_(val) {}

    // A static parsing function that will be implemented in json.cpp
    static JsonValue parse(const std::string& json_string);

    // A method to serialize the JsonValue back into a string.
    std::string serialize() const;

    // Type checking methods
    bool is_null() const { return std::holds_alternative<JsonNull>(value_); }
    bool is_bool() const { return std::holds_alternative<bool>(value_); }
    bool is_number() const { return std::holds_alternative<double>(value_); }
    bool is_string() const { return std::holds_alternative<std::string>(value_); }
    bool is_array() const { return std::holds_alternative<JsonArray>(value_); }
    bool is_object() const { return std::holds_alternative<JsonObject>(value_); }

    // Value accessors (will throw if type is wrong)
    bool as_bool() const { return std::get<bool>(value_); }
    double as_number() const { return std::get<double>(value_); }
    const std::string& as_string() const { return std::get<std::string>(value_); }
    const JsonArray& as_array() const { return std::get<JsonArray>(value_); }
    const JsonObject& as_object() const { return std::get<JsonObject>(value_); }

private:
    ValueType value_;
};

} // namespace Json
} // namespace NexusFlow
