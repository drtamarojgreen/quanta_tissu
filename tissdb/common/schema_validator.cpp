#include "schema_validator.h"
#include <iostream>

namespace TissDB {

bool SchemaValidator::validate(const Document& doc, const Schema& schema) {
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
                    // For simplicity, we'll just print to stderr for now.
                    // A more robust solution would involve a proper error reporting mechanism.
                    std::cerr << "Schema validation failed for document " << doc.id
                              << ": field '" << field_schema.name << "' has incorrect type." << std::endl;
                    return false;
                }
                break; 
            }
        }
        if (field_schema.required && !field_found) {
            std::cerr << "Schema validation failed for document " << doc.id
                      << ": required field '" << field_schema.name << "' is missing." << std::endl;
            return false;
        }
    }
    return true;
}

} // namespace TissDB
