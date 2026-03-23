#include "schema_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../../../../tissdb/json/json.h"

namespace TissDB {
namespace Schema {

SchemaManager::SchemaManager() {
    loadSchemasFromDisk();
}

bool SchemaManager::registerSchema(const TableSchema& schema) {
    if (schemas_.count(schema.table_name)) {
        std::cerr << "Error: Schema for table '" << schema.table_name << "' already exists." << std::endl;
        return false;
    }
    schemas_[schema.table_name] = std::make_shared<TableSchema>(schema);
    std::cout << "Schema for table '" << schema.table_name << "' registered." << std::endl;
    saveSchemasToDisk();
    return true;
}

std::shared_ptr<const TableSchema> SchemaManager::getSchema(const std::string& table_name) const {
    auto it = schemas_.find(table_name);
    if (it != schemas_.end()) {
        return it->second;
    }
    return nullptr;
}

bool SchemaManager::validateDocument(const std::string& table_name, const std::string& document_json) const {
    auto schema = getSchema(table_name);
    if (!schema) {
        std::cerr << "SchemaManager Error: Table '" << table_name << "' not found." << std::endl;
        return false;
    }

    try {
        TissDB::Json::JsonValue doc = TissDB::Json::JsonValue::parse(document_json);
        if (!doc.is_object()) {
            std::cerr << "SchemaManager Error: Document is not a JSON object." << std::endl;
            return false;
        }

        const auto& doc_obj = doc.as_object();

        for (const auto& pair : schema->columns) {
            const std::string& col_name = pair.first;
            const auto& col_def = pair.second;

            bool present = doc_obj.count(col_name) > 0;
            bool is_not_null = false;
            for (auto c : col_def.constraints) {
                if (c == DDL::ColumnConstraint::NOT_NULL || c == DDL::ColumnConstraint::PRIMARY_KEY) {
                    is_not_null = true;
                    break;
                }
            }

            if (!present) {
                if (is_not_null) {
                    std::cerr << "SchemaManager Error: Required column '" << col_name << "' is missing." << std::endl;
                    return false;
                }
                continue;
            }

            const auto& val = doc_obj.at(col_name);

            switch (col_def.type) {
                case DDL::DataType::INT:
                case DDL::DataType::FLOAT:
                    if (!val.is_number()) {
                        std::cerr << "SchemaManager Error: Column '" << col_name << "' expected number." << std::endl;
                        return false;
                    }
                    break;
                case DDL::DataType::STRING:
                case DDL::DataType::DATETIME:
                    if (!val.is_string()) {
                        std::cerr << "SchemaManager Error: Column '" << col_name << "' expected string." << std::endl;
                        return false;
                    }
                    break;
                case DDL::DataType::BOOL:
                    if (!val.is_bool()) {
                        std::cerr << "SchemaManager Error: Column '" << col_name << "' expected boolean." << std::endl;
                        return false;
                    }
                    break;
            }
        }

        for (const auto& pk_col : schema->primary_key_columns) {
            if (doc_obj.count(pk_col) == 0 || doc_obj.at(pk_col).is_null()) {
                std::cerr << "SchemaManager Error: Primary key column '" << pk_col << "' must be present and not null." << std::endl;
                return false;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "SchemaManager Error parsing document: " << e.what() << std::endl;
        return false;
    }
}

void SchemaManager::loadSchemasFromDisk() {
    std::ifstream ifs("schemas_v1.json");
    if (!ifs.is_open()) return;

    try {
        std::stringstream ss;
        ss << ifs.rdbuf();
        TissDB::Json::JsonValue root = TissDB::Json::JsonValue::parse(ss.str());

        for (const auto& table_pair : root.as_object()) {
            TableSchema schema;
            schema.table_name = table_pair.first;
            auto table_obj = table_pair.second.as_object();

            auto cols_obj = table_obj.at("columns").as_object();
            for (const auto& col_pair : cols_obj) {
                DDL::ColumnDefinition col;
                col.name = col_pair.first;
                auto col_info = col_pair.second.as_object();
                col.type = static_cast<DDL::DataType>((int)col_info.at("type").as_number());

                auto constr_arr = col_info.at("constraints").as_array();
                for (const auto& c_val : constr_arr) {
                    col.constraints.push_back(static_cast<DDL::ColumnConstraint>((int)c_val.as_number()));
                }
                schema.columns[col.name] = col;
            }

            auto pk_arr = table_obj.at("primary_key").as_array();
            for (const auto& pk_val : pk_arr) {
                schema.primary_key_columns.push_back(pk_val.as_string());
            }

            schemas_[schema.table_name] = std::make_shared<TableSchema>(schema);
        }
    } catch (...) {
        std::cerr << "Error loading schemas from disk." << std::endl;
    }
}

void SchemaManager::saveSchemasToDisk() {
    TissDB::Json::JsonObject root_obj;
    for (const auto& pair : schemas_) {
        const auto& schema = *pair.second;
        TissDB::Json::JsonObject table_obj;

        TissDB::Json::JsonObject cols_obj;
        for (const auto& col_pair : schema.columns) {
            const auto& col = col_pair.second;
            TissDB::Json::JsonObject col_info;
            col_info["type"] = (double)static_cast<int>(col.type);

            TissDB::Json::JsonArray constr_arr;
            for (auto c : col.constraints) {
                constr_arr.push_back((double)static_cast<int>(c));
            }
            col_info["constraints"] = constr_arr;
            cols_obj[col.name] = col_info;
        }
        table_obj["columns"] = cols_obj;

        TissDB::Json::JsonArray pk_arr;
        for (const auto& pk : schema.primary_key_columns) {
            pk_arr.push_back(pk);
        }
        table_obj["primary_key"] = pk_arr;

        root_obj[schema.table_name] = table_obj;
    }

    std::ofstream ofs("schemas_v1.json");
    if (ofs.is_open()) {
        ofs << TissDB::Json::JsonValue(root_obj).serialize();
    }
}

}
}
