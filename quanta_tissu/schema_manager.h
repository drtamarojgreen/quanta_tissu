#ifndef TISSDB_SCHEMA_MANAGER_H
#define TISSDB_SCHEMA_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// Include DDL parser definitions for ColumnDefinition and DataType
#include "ddl_parser.h"

namespace TissDB {
namespace Schema {

struct TableSchema {
    std::string table_name;
    std::unordered_map<std::string, DDL::ColumnDefinition> columns;
    // Primary key can be composite
    std::vector<std::string> primary_key_columns;
    // Each inner vector represents a unique constraint on multiple columns
    std::vector<std::vector<std::string>> unique_constraints;

    struct ForeignKeyConstraint {
        std::string column_name; // Column in this table
        std::string referenced_table_name;
        std::string referenced_column_name;
        // Add ON DELETE/ON UPDATE actions if needed
    };
    std::vector<ForeignKeyConstraint> foreign_keys;
};

class SchemaManager {
public:
    SchemaManager();

    // Registers a new table schema
    bool registerSchema(const TableSchema& schema);

    // Retrieves a table schema by name
    std::shared_ptr<const TableSchema> getSchema(const std::string& table_name) const;

    // Validates a document against a table schema (placeholder)
    // This would involve checking column existence, data types, and constraints
    bool validateDocument(const std::string& table_name, const std::string& document_json) const;

    // Placeholder for system catalogs (where schemas are persisted)
    void loadSchemasFromDisk();
    void saveSchemasToDisk();

private:
    std::unordered_map<std::string, std::shared_ptr<TableSchema>> schemas_;
};

} // namespace Schema
} // namespace TissDB

#endif // TISSDB_SCHEMA_MANAGER_H
