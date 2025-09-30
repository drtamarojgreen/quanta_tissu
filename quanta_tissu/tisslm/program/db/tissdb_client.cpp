#include "tissdb_client.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <curl/curl.h>

namespace TissDB {

// Simple HTTP client using libcurl
class HttpClient {
public:
    HttpClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    ~HttpClient() {
        curl_global_cleanup();
    }
    
    struct Response {
        long status_code;
        std::string body;
        bool success;
    };
    
    Response put(const std::string& url, const std::string& data = "") {
        CURL* curl = curl_easy_init();
        Response response;
        
        if (!curl) {
            response.success = false;
            response.status_code = 0;
            return response;
        }
        
        std::string response_body;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        
        struct curl_slist* headers = nullptr;
        if (!data.empty()) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            response.body = response_body;
            response.success = true;
        } else {
            response.success = false;
            response.status_code = 0;
        }
        
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return response;
    }
    
    Response get(const std::string& url) {
        CURL* curl = curl_easy_init();
        Response response;
        
        if (!curl) {
            response.success = false;
            response.status_code = 0;
            return response;
        }
        
        std::string response_body;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            response.body = response_body;
            response.success = true;
        } else {
            response.success = false;
            response.status_code = 0;
        }
        
        curl_easy_cleanup(curl);
        
        return response;
    }

    Response post(const std::string& url, const std::string& data = "") {
        CURL* curl = curl_easy_init();
        Response response;
        
        if (!curl) {
            response.success = false;
            response.status_code = 0;
            return response;
        }
        
        std::string response_body;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        
        struct curl_slist* headers = nullptr;
        if (!data.empty()) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            response.body = response_body;
            response.success = true;
        } else {
            response.success = false;
            response.status_code = 0;
        }
        
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return response;
    }

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};

// Document implementation
std::string Document::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& pair : fields) {
        if (!first) oss << ",";
        oss << "\"" << pair.first << "\":\"" << pair.second << "\"";
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

Document Document::from_json(const std::string& json_str) {
    Document doc;
    // Simple JSON parsing (for production, use a proper JSON library)
    // This is a simplified implementation
    size_t pos = 0;
    while (pos < json_str.length()) {
        size_t key_start = json_str.find("\"", pos);
        if (key_start == std::string::npos) break;
        
        size_t key_end = json_str.find("\"", key_start + 1);
        if (key_end == std::string::npos) break;
        
        std::string key = json_str.substr(key_start + 1, key_end - key_start - 1);
        
        size_t value_start = json_str.find("\"", key_end + 1);
        if (value_start == std::string::npos) break;
        
        size_t value_end = json_str.find("\"", value_start + 1);
        if (value_end == std::string::npos) break;
        
        std::string value = json_str.substr(value_start + 1, value_end - value_start - 1);
        
        doc.set_field(key, value);
        pos = value_end + 1;
    }
    
    return doc;
}

// TissDBClient implementation
TissDBClient::TissDBClient(const std::string& host, int port, const std::string& db_name)
    : host_(host), port_(port), db_name_(db_name) {
    
    base_url_ = "http://" + host_ + ":" + std::to_string(port_);
    db_url_ = base_url_ + "/" + db_name_;
    http_client_ = std::make_unique<HttpClient>();
}

TissDBClient::~TissDBClient() = default;

bool TissDBClient::ensure_db_setup(const std::vector<std::string>& collections) {
    try {
        // Create database
        auto db_response = http_client_->put(db_url_);
        if (!db_response.success) {
            throw DatabaseException("Failed to connect to TissDB");
        }
        
        // Accept 200, 201, 409 (already exists), or 500 with "already exists"
        if (db_response.status_code != 200 && 
            db_response.status_code != 201 && 
            db_response.status_code != 409) {
            if (db_response.status_code == 500 && 
                db_response.body.find("already exists") != std::string::npos) {
                // Database already exists, continue
            } else {
                throw DatabaseException("Failed to create database: " + std::to_string(db_response.status_code));
            }
        }
        
        // Create collections
        for (const auto& collection : collections) {
            std::string coll_url = db_url_ + "/" + collection;
            auto coll_response = http_client_->put(coll_url);
            
            if (!coll_response.success) {
                throw DatabaseException("Failed to create collection: " + collection);
            }
            
            if (coll_response.status_code != 200 && 
                coll_response.status_code != 201 && 
                coll_response.status_code != 409) {
                throw DatabaseException("Failed to create collection " + collection + 
                                      ": " + std::to_string(coll_response.status_code));
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        throw DatabaseException(std::string("Database setup failed: ") + e.what());
    }
}

std::string TissDBClient::add_document(const std::string& collection, 
                                       const Document& document,
                                       const std::string& doc_id) {
    std::string id = doc_id.empty() ? generate_doc_id() : doc_id;
    std::string url = db_url_ + "/" + collection + "/" + id;
    std::string json_data = document.to_json();
    
    auto response = http_client_->put(url, json_data);
    
    if (!response.success || (response.status_code != 200 && response.status_code != 201)) {
        throw DatabaseException("Failed to add document: " + std::to_string(response.status_code));
    }
    
    return id;
}

Document TissDBClient::get_document(const std::string& collection, const std::string& doc_id) {
    std::string url = db_url_ + "/" + collection + "/" + doc_id;
    auto response = http_client_->get(url);
    
    if (!response.success || response.status_code != 200) {
        throw DatabaseException("Failed to get document: " + std::to_string(response.status_code));
    }
    
    Document doc = Document::from_json(response.body);
    doc.id = doc_id;
    return doc;
}

std::map<std::string, std::string> TissDBClient::get_stats() {
    std::string url = db_url_ + "/_stats";
    auto response = http_client_->get(url);
    
    std::map<std::string, std::string> stats;
    
    if (!response.success || response.status_code != 200) {
        throw DatabaseException("Failed to get stats: " + std::to_string(response.status_code));
    }
    
    // Parse response body (simplified)
    stats["response"] = response.body;
    return stats;
}

std::string TissDBClient::add_feedback(const Document& feedback_data) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    std::string feedback_id = "feedback_" + std::to_string(timestamp) + "_" + generate_doc_id();
    return add_document("feedback", feedback_data, feedback_id);
}

bool TissDBClient::test_connection() {
    try {
        auto response = http_client_->get(base_url_);
        return response.success;
    } catch (...) {
        return false;
    }
}

std::string TissDBClient::generate_doc_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            oss << "-";
        }
        oss << std::hex << dis(gen);
    }
    
    return oss.str();
}

std::string TissDBClient::get_collection_url(const std::string& collection) const {
    return db_url_ + "/" + collection;
}

std::vector<Document> TissDBClient::search_documents(const std::string& collection, const std::string& query_json) {
    std::string url = db_url_ + "/" + collection + "/_search";
    auto response = http_client_->post(url, query_json);

    if (!response.success || response.status_code != 200) {
        throw DatabaseException("Failed to search documents: " + std::to_string(response.status_code) + ": " + response.body);
    }

    std::vector<Document> results;
    // This is a placeholder for robust JSON array parsing.
    // In a real scenario, you would use a proper JSON library to parse the response body
    // which is expected to be a JSON array of document objects.
    // For now, we'll assume a very simple format or just return an empty list.
    
    // Example of very basic parsing for a list of documents (needs improvement)
    std::string response_body = response.body;
    size_t start = response_body.find("[");
    size_t end = response_body.rfind("]");

    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string docs_array_str = response_body.substr(start + 1, end - start - 1);
        
        // Split by '},{' to get individual document JSON strings (very fragile)
        std::string delimiter = "},{ ";
        size_t pos = 0;
        std::string token;
        while ((pos = docs_array_str.find(delimiter)) != std::string::npos) {
            token = docs_array_str.substr(0, pos);
            results.push_back(Document::from_json("{" + token + "}"));
            docs_array_str.erase(0, pos + delimiter.length());
        }
        if (!docs_array_str.empty()) {
            results.push_back(Document::from_json("{" + docs_array_str + "}"));
        }
    }

    return results;
}

} // namespace TissDB
