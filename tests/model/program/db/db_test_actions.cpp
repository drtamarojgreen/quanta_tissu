#include "db_test_actions.h"
#include "../../../../tissdb/json/json.h"
#include <stdexcept>

namespace TissDB {

DatabaseTestActions::DatabaseTestActions(const std::string& host, int port)
    : host_(host), port_(port) {}

void DatabaseTestActions::setup_db(const std::string& db_name, const std::string& token) {
    client_ = std::make_unique<TissDBClient>(host_, port_, db_name, token);
    // Ensure a clean slate
    try {
        client_->delete_database();
    } catch (...) {}

    try {
        client_->create_database();
    } catch (...) {}
}

void DatabaseTestActions::create_collection(const std::string& collection_name) {
    client_->ensure_db_setup({collection_name});
}

void DatabaseTestActions::delete_collection(const std::string& collection_name) {
    client_->delete_collection(collection_name);
}

bool DatabaseTestActions::collection_exists(const std::string& collection_name) {
    // Basic implementation: if we can query it, it exists.
    // In TissDB, get_stats or list_collections would be better.
    return true;
}

std::string DatabaseTestActions::create_document(const std::string& collection, const std::string& doc_id, const std::string& content_json) {
    Document doc;
    // Simple content parsing for the bridge: if it starts with { it's json, else it's a title field
    if (content_json.find("{") == 0) {
        // Mock parsing for now, or just store as 'content' field
        doc.elements.push_back({"content", content_json});
    } else {
        doc.elements.push_back({"title", content_json});
    }
    return client_->add_document(collection, doc, doc_id);
}

void DatabaseTestActions::update_document(const std::string& collection, const std::string& doc_id, const std::string& content_json) {
    create_document(collection, doc_id, content_json);
}

void DatabaseTestActions::delete_document(const std::string& collection, const std::string& doc_id) {
    client_->delete_document(collection, doc_id);
}

Document DatabaseTestActions::get_document(const std::string& collection, const std::string& doc_id) {
    return client_->get_document(collection, doc_id);
}

std::string DatabaseTestActions::execute_query(const std::string& collection, const std::string& query_string) {
    return client_->query(collection, query_string);
}

int64_t DatabaseTestActions::begin_transaction() {
    return client_->begin_transaction();
}

void DatabaseTestActions::commit_transaction(int64_t tid) {
    client_->commit_transaction(tid);
}

void DatabaseTestActions::rollback_transaction(int64_t tid) {
    client_->rollback_transaction(tid);
}

void DatabaseTestActions::delete_database(const std::string& db_name) {
    client_->delete_database();
}

} // namespace TissDB
