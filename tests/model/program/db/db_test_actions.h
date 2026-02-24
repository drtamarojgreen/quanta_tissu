#ifndef TISSLM_DB_TEST_ACTIONS_H
#define TISSLM_DB_TEST_ACTIONS_H

#include "../../../../quanta_tissu/tisslm/program/db/tissdb_client.h"
#include <string>
#include <vector>
#include <memory>

namespace TissDB {

class DatabaseTestActions {
public:
    DatabaseTestActions(const std::string& host = "127.0.0.1", int port = 9876);

    void setup_db(const std::string& db_name, const std::string& token = "static_test_token");
    void create_collection(const std::string& collection_name);
    void delete_collection(const std::string& collection_name);
    bool collection_exists(const std::string& collection_name);

    std::string create_document(const std::string& collection, const std::string& doc_id, const std::string& content_json);
    void update_document(const std::string& collection, const std::string& doc_id, const std::string& content_json);
    void delete_document(const std::string& collection, const std::string& doc_id);
    Document get_document(const std::string& collection, const std::string& doc_id);

    std::string execute_query(const std::string& collection, const std::string& query_string);

    int64_t begin_transaction();
    void commit_transaction(int64_t tid);
    void rollback_transaction(int64_t tid);

    void delete_database(const std::string& db_name);

private:
    std::string host_;
    int port_;
    std::unique_ptr<TissDBClient> client_;
};

} // namespace TissDB

#endif // TISSLM_DB_TEST_ACTIONS_H
