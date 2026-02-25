#ifndef TISSLM_TISSDB_CLIENT_H
#define TISSLM_TISSDB_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "../../../../tissdb/common/document.h"

namespace TissDB {

class HttpClient;

class TissDBClient {
public:
    TissDBClient(const std::string& host = "127.0.0.1", int port = 9876, const std::string& db_name = "testdb", const std::string& token = "");
    ~TissDBClient();

    bool ensure_db_setup(const std::vector<std::string>& collections);
    std::string add_document(const std::string& collection, const Document& document, const std::string& doc_id = "");
    Document get_document(const std::string& collection, const std::string& doc_id);
    std::vector<Document> search_documents(const std::string& collection, const std::string& query_json);
    std::string add_feedback(const Document& feedback_data);
    bool test_connection();
    std::map<std::string, std::string> get_stats();
    void delete_database();
    void create_database();
    std::string query(const std::string& collection, const std::string& query_string);

private:
    std::string host_;
    int port_;
    std::string db_name_;
    std::string base_url_;
    std::string db_url_;
    std::unique_ptr<HttpClient> http_client_;

    std::string generate_doc_id() const;
    std::string get_collection_url(const std::string& collection) const;
};

} // namespace TissDB

#endif // TISSLM_TISSDB_CLIENT_H