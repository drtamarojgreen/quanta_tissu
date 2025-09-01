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
    Date,
    Time,
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

// Represents a foreign key constraint
struct ForeignKeyConstraint {
    std::string field_name;
    std::string referenced_collection;
    std::string referenced_field;
};

// Represents the schema for a collection
class Schema {
public:
    void add_field(const std::string& name, FieldType type, bool required, bool unique = false) {
        fields_.push_back({name, type, required, unique});
    }

    void set_primary_key(const std::string& field_name) {
        primary_key_ = field_name;
    }

    void add_foreign_key(const std::string& field_name, const std::string& referenced_collection, const std::string& referenced_field) {
        foreign_keys_.push_back({field_name, referenced_collection, referenced_field});
    }

    const std::vector<FieldSchema>& get_fields() const {
        return fields_;
    }

    const std::string& get_primary_key() const {
        return primary_key_;
    }

    const std::vector<ForeignKeyConstraint>& get_foreign_keys() const {
        return foreign_keys_;
    }

private:
    std::vector<FieldSchema> fields_;
    std::string primary_key_;
    std::vector<ForeignKeyConstraint> foreign_keys_;
};

} // namespace TissDB
