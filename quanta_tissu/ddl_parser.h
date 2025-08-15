#ifndef TISSDB_DDL_PARSER_H
#define TISSDB_DDL_PARSER_H

#include <string>
#include <memory>
#include <vector>

namespace TissDB {
namespace DDL {

// Forward declarations for AST nodes
class DDLStatement;
class CreateTableStatement;
class DropTableStatement;
class AlterTableStatement;
class ColumnDefinition;

enum class DataType {
    INT,
    STRING,
    BOOL,
    FLOAT,
    DATETIME
};

enum class ColumnConstraint {
    NONE,
    PRIMARY_KEY,
    UNIQUE,
    NOT_NULL
};

struct ColumnDefinition {
    std::string name;
    DataType type;
    std::vector<ColumnConstraint> constraints;
};

// Base class for all DDL statements
class DDLStatement {
public:
    enum class Type {
        CREATE_TABLE,
        DROP_TABLE,
        ALTER_TABLE
    };

    virtual ~DDLStatement() = default;
    virtual Type getType() const = 0;
};

class CreateTableStatement : public DDLStatement {
public:
    CreateTableStatement(std::string tableName, std::vector<ColumnDefinition> columns)
        : table_name_(std::move(tableName)), columns_(std::move(columns)) {}

    Type getType() const override { return Type::CREATE_TABLE; }
    const std::string& getTableName() const { return table_name_; }
    const std::vector<ColumnDefinition>& getColumns() const { return columns_; }

private:
    std::string table_name_;
    std::vector<ColumnDefinition> columns_;
};

class DropTableStatement : public DDLStatement {
public:
    DropTableStatement(std::string tableName) : table_name_(std::move(tableName)) {}

    Type getType() const override { return Type::DROP_TABLE; }
    const std::string& getTableName() const { return table_name_; }

private:
    std::string table_name_;
};

class AlterTableStatement : public DDLStatement {
public:
    enum class ActionType {
        ADD_COLUMN,
        DROP_COLUMN
    };

    AlterTableStatement(std::string tableName, ActionType actionType)
        : table_name_(std::move(tableName)), action_type_(actionType) {}

    Type getType() const override { return Type::ALTER_TABLE; }
    ActionType getActionType() const { return action_type_; }
    const std::string& getTableName() const { return table_name_; }

private:
    std::string table_name_;
    ActionType action_type_;
};

// Placeholder for the DDL Parser class
class DDLParser {
public:
    // Parses a DDL string and returns an AST
    std::unique_ptr<DDLStatement> parse(const std::string& ddl_string);

    // Placeholder for error handling
    bool hasErrors() const { return false; }
    std::string getErrorMessage() const { return ""; }
};

} // namespace DDL
} // namespace TissDB

#endif // TISSDB_DDL_PARSER_H
