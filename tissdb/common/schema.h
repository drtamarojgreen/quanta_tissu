#pragma once

#include <string>
#include <vector>
#include <variant>

namespace TissDB {

// Enum for the possible data types of a field
enum class FieldType {
    String,
    Number,
    Boolean,
    DateTime,
    Binary,
    Object, // For nested objects
    Array   // For arrays of elements
};

// Represents a single field in a schema
struct FieldSchema {
    std::string name;
    FieldType type;
    bool required = false;
    bool unique = false;
};

// Represents the schema for a collection
class Schema {
public:
    void add_field(const std::string& name, FieldType type, bool required, bool unique = false) {
        fields_.push_back({name, type, required, unique});
    }

    const std::vector<FieldSchema>& get_fields() const {
        return fields_;
    }

private:
    std::vector<FieldSchema> fields_;
};

} // namespace TissDB
