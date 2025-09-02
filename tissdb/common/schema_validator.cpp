#include "schema_validator.h"
#include "../storage/indexer.h"
#include <iostream>
#include <stdexcept>

namespace TissDB {

SchemaValidator::SchemaValidator(const Storage::Indexer& indexer) : indexer_(indexer) {}

bool SchemaValidator::validate(const Document& doc, const Schema& schema) const {
    const auto& fields = schema.get_fields();
    for (const auto& field_schema : fields) {
        bool field_found = false;
        for (const auto& element : doc.elements) {
            if (element.key == field_schema.name) {
                field_found = true;
                // Check type
                bool type_matches = false;
                switch (field_schema.type) {
                    case FieldType::String:
                        if (std::holds_alternative<std::string>(element.value)) type_matches = true;
                        break;
                    case FieldType::Number:
                        if (std::holds_alternative<Number>(element.value)) type_matches = true;
                        break;
                    case FieldType::Boolean:
                        if (std::holds_alternative<Boolean>(element.value)) type_matches = true;
                        break;
                    case FieldType::Date:
                        if (std::holds_alternative<Date>(element.value)) type_matches = true;
                        break;
                    case FieldType::Time:
                        if (std::holds_alternative<Time>(element.value)) type_matches = true;
                        break;
                    case FieldType::DateTime:
                        if (std::holds_alternative<DateTime>(element.value)) type_matches = true;
                        break;
                    case FieldType::Binary:
                        if (std::holds_alternative<BinaryData>(element.value)) type_matches = true;
                        break;
                    case FieldType::Object:
                        if (std::holds_alternative<std::vector<Element>>(element.value)) type_matches = true;
                        break;
                    case FieldType::Array:
                        if (std::holds_alternative<std::vector<Element>>(element.value)) type_matches = true;
                        break;
                }
                if (!type_matches) {
                    throw std::runtime_error("Schema validation failed for document " + doc.id +
                                             ": field '" + field_schema.name + "' has incorrect type.");
                }

                // Check uniqueness
                if (field_schema.unique) {
                    if (!indexer_.has_index({field_schema.name})) {
                         throw std::runtime_error("Schema validation failed: field '" + field_schema.name +
                                                  "' is marked as unique but is not indexed.");
                    }
                    std::string value_str;
                    if (std::holds_alternative<std::string>(element.value)) {
                        value_str = std::get<std::string>(element.value);
                    } else {
                         // For simplicity, we only support unique constraints on strings for now.
                         throw std::runtime_error("Schema validation failed: unique constraint is only supported for string types.");
                    }

                    auto found_ids = indexer_.find_by_index(field_schema.name, value_str);
                    if (!found_ids.empty() && (found_ids.size() > 1 || found_ids[0] != doc.id)) {
                        throw std::runtime_error("Schema validation failed for document " + doc.id +
                                                 ": uniqueness constraint violated for field '" + field_schema.name + "'.");
                    }
                }
                break; 
            }
        }
        if (field_schema.required && !field_found) {
            throw std::runtime_error("Schema validation failed for document " + doc.id +
                                     ": required field '" + field_schema.name + "' is missing.");
        }
    }
    return true;
}

} // namespace TissDB
