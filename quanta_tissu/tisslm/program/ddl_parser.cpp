#include "ddl_parser.h"
#include <iostream>
#include <regex>
#include <vector>
#include <algorithm>

namespace TissDB {
namespace DDL {

static std::string trim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

std::unique_ptr<DDLStatement> DDLParser::parse(const std::string& ddl_string) {
    error_message_.clear();

    std::regex create_re(R"(CREATE\s+TABLE\s+(\w+)\s*\((.*)\))", std::regex::icase);
    std::smatch match;
    if (std::regex_search(ddl_string, match, create_re)) {
        std::string table_name = match[1];
        std::string columns_part = match[2];
        std::vector<ColumnDefinition> columns;

        std::stringstream ss(columns_part);
        std::string col_def_str;
        while (std::getline(ss, col_def_str, ',')) {
            col_def_str = trim(col_def_str);
            if (col_def_str.empty()) continue;

            std::stringstream col_ss(col_def_str);
            std::string name, type_str;
            col_ss >> name >> type_str;
            std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::toupper);

            ColumnDefinition col;
            col.name = name;
            if (type_str == "INT") col.type = DataType::INT;
            else if (type_str == "STRING") col.type = DataType::STRING;
            else if (type_str == "BOOL") col.type = DataType::BOOL;
            else if (type_str == "FLOAT") col.type = DataType::FLOAT;
            else if (type_str == "DATETIME") col.type = DataType::DATETIME;
            else {
                error_message_ = "Unknown data type: " + type_str;
                return nullptr;
            }

            std::string constraint;
            while (col_ss >> constraint) {
                std::transform(constraint.begin(), constraint.end(), constraint.begin(), ::toupper);
                if (constraint == "PRIMARY") {
                    std::string key; col_ss >> key;
                    col.constraints.push_back(ColumnConstraint::PRIMARY_KEY);
                } else if (constraint == "NOT") {
                    std::string n; col_ss >> n;
                    col.constraints.push_back(ColumnConstraint::NOT_NULL);
                } else if (constraint == "UNIQUE") {
                    col.constraints.push_back(ColumnConstraint::UNIQUE);
                }
            }
            columns.push_back(col);
        }
        return std::make_unique<CreateTableStatement>(table_name, columns);
    }

    std::regex drop_re(R"(DROP\s+TABLE\s+(\w+))", std::regex::icase);
    if (std::regex_search(ddl_string, match, drop_re)) {
        return std::make_unique<DropTableStatement>(match[1]);
    }

    error_message_ = "Failed to parse DDL statement.";
    return nullptr;
}

}
}
