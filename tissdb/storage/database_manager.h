#pragma once

#include <string>
#include <map>
#include <memory>
#include "lsm_tree.h"

namespace TissDB {
namespace Storage {

class DatabaseManager {
public:
    DatabaseManager(const std::string& base_path);
    ~DatabaseManager();

    // Creates a new database.
    // Throws a runtime_error if the database already exists.
    void create_database(const std::string& db_name);

    // Deletes an existing database.
    // Throws a runtime_error if the database does not exist.
    void delete_database(const std::string& db_name);

    // Retrieves a reference to a database.
    // Throws a runtime_error if the database does not exist.
    LSMTree& get_database(const std::string& db_name);

    // Checks if a database exists.
    bool database_exists(const std::string& db_name) const;

private:
    std::string base_data_path_;
    std::map<std::string, std::unique_ptr<LSMTree>> databases_;
};

} // namespace Storage
} // namespace TissDB
