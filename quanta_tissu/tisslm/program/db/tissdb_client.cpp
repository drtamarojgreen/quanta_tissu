#include "tissdb_client.h"
#include "http_client.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>      // For exit(), stoi()
#include <cstdint>      // For uint16_t, uint32_t
#include <vector> // Added for std::vector
#include <map>    // Added for std::map

namespace TissDB {

// Document implementation (from tissdb_client.h)
std::string Document::to_json() const {
    std::string json = "{";
    json += "\"id\":\"" + id + "\",";
    json += "\"fields\":{";
    bool first = true;
    for (const auto& pair : fields) {
        if (!first) {
            json += ",";
        }
        json += "\"" + pair.first + "\":\"" + pair.second + "\"";
        first = false;
    }
    json += "}}";
    return json;
}

Document Document::from_json(const std::string& json_str) {
    Document doc;
    size_t id_pos = json_str.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json_str.find('"', id_pos + 5) + 1;
        size_t end = json_str.find('"', start);
        doc.id = json_str.substr(start, end - start);
    }

    size_t fields_pos = json_str.find("\"fields\":{");
    if (fields_pos != std::string::npos) {
        size_t start = fields_pos + 10;
        size_t end = json_str.rfind('}');
        std::string fields_str = json_str.substr(start, end - start);

        size_t current_pos = 0;
        while (current_pos < fields_str.length()) {
            size_t key_start = fields_str.find('"', current_pos) + 1;
            size_t key_end = fields_str.find('"', key_start);
            std::string key = fields_str.substr(key_start, key_end - key_start);

            size_t value_start = fields_str.find('"', key_end + 2) + 1;
            size_t value_end = fields_str.find('"', value_start);
            std::string value = fields_str.substr(value_start, value_end - value_start);

            doc.fields[key] = value;

            current_pos = value_end + 1;
            if (current_pos < fields_str.length() && fields_str[current_pos] == ',') {
                current_pos++;
            }
        }
    }

    return doc;
}

void Document::set_field(const std::string& key, const std::string& value) {
    fields[key] = value;
}

std::string Document::get_field(const std::string& key) const {
    auto it = fields.find(key);
    if (it != fields.end()) {
        return it->second;
    }
    return "";
}

TissDBClient::TissDBClient(const std::string& host, int port, const std::string& db_name)
    : host_(host), port_(port), db_name_(db_name) {
    base_url_ = "http://" + host_ + ":" + std::to_string(port_);
    db_url_ = base_url_ + "/db/" + db_name_;
    http_client_ = std::make_unique<HttpClient>();
}

TissDBClient::~TissDBClient() = default;

bool TissDBClient::ensure_db_setup(const std::vector<std::string>& collections) {
    // Simplified: In a real scenario, this would make API calls to create DB/collections
    // For now, just return true
    return true;
}

std::string TissDBClient::add_document(const std::string& collection, const TissDB::Document& document, const std::string& doc_id) {
    std::string url = db_url_ + "/collection/" + collection + "/document";
    std::string doc_json = document.to_json();
    // In a real scenario, http_client_->post(url, doc_json) would be called
    // For now, just return a dummy ID
    return "dummy_id";
}

TissDB::Document TissDBClient::get_document(const std::string& collection, const std::string& doc_id) {
    std::string url = db_url_ + "/collection/" + collection + "/document/" + doc_id;
    // In a real scenario, http_client_->get(url) would be called
    // For now, return a dummy document
    TissDB::Document doc(doc_id);
    doc.set_field("content", "Dummy content for " + doc_id);
    return doc;
}

std::map<std::string, std::string> TissDBClient::get_stats() {
    return {}; // Dummy stats
}

std::string TissDBClient::add_feedback(const TissDB::Document& feedback_data) {
    return "dummy_feedback_id";
}

bool TissDBClient::test_connection() {
    return true; // Dummy connection test
}

std::vector<TissDB::Document> TissDBClient::search_documents(const std::string& collection, const std::string& query_json) {
    return {}; // Dummy search results
}

} // namespace TissDB
