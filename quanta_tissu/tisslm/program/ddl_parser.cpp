#include "ddl_parser.h"
#include <iostream>
#include <regex>
#include <vector>

namespace TissDB {
namespace DDL {

std::unique_ptr<DDLStatement> DDLParser::parse(const std::string& ddl_string) {
    std::cout << "DDLParser: Parsing query: " << ddl_string << std::endl;

    // 1. Check for CREATE TABLE
    std::regex create_re(R"(CREATE\s+TABLE\s+(\w+)\s*\((.*)\))", std::regex::icase);
    std::smatch match;
    if (std::regex_search(ddl_string, match, create_re)) {
        std::string table_name = match[1];
        std::string columns_part = match[2];

        std::vector<ColumnDefinition> columns;
        // Simple column parsing: "id INT, name STRING"
        std::regex col_re(R"((\w+)\s+(\w+))");
        auto words_begin = std::sregex_iterator(columns_part.begin(), columns_part.end(), col_re);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch col_match = *i;
            ColumnDefinition col;
            col.name = col_match[1];
            std::string type_str = col_match[2];
            if (type_str == "INT") col.type = DataType::INT;
            else if (type_str == "STRING") col.type = DataType::STRING;
            else col.type = DataType::STRING; // Default
            columns.push_back(col);
        }

        return std::make_unique<CreateTableStatement>(table_name, columns);
    }

    // 2. Check for DROP TABLE
    std::regex drop_re(R"(DROP\s+TABLE\s+(\w+))", std::regex::icase);
    if (std::regex_search(ddl_string, match, drop_re)) {
        return std::make_unique<DropTableStatement>(match[1]);
    }

    return nullptr;
}

} // namespace DDL
} // namespace TissDB
