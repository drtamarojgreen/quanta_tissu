#include "tissdb_client.h"
#include "../../../../tissdb/json/json.h"
#include "http_client.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>      // For exit(), stoi()
#include <cstdint>      // For uint16_t, uint32_t
#include <vector> // Added for std::vector
#include <map>    // Added for std::map

namespace TissDB {

TissDB::Document from_json(const std::string& json_str) {
    TissDB::Json::JsonValue json = TissDB::Json::JsonValue::parse(json_str);
    TissDB::Document doc;
    doc.id = json.as_object().at("id").as_string();
    const auto& elements = json.as_object().at("elements").as_object();
    for (const auto& pair : elements) {
        TissDB::Element elem;
        elem.key = pair.first;
        const auto& val = pair.second;
        if (val.is_null()) {
            elem.value = nullptr;
        } else if (val.is_string()) {
            elem.value = val.as_string();
        } else if (val.is_number()) {
            elem.value = val.as_number();
        } else if (val.is_bool()) {
            elem.value = val.as_bool();
        }
        doc.elements.push_back(elem);
    }
    return doc;
}

std::string to_json(const TissDB::Document& doc) {
    TissDB::Json::JsonObject obj;
    obj["id"] = doc.id;
    TissDB::Json::JsonObject elements;
    for (const auto& elem : doc.elements) {
        elements[elem.key] = std::visit([](auto&& arg) -> TissDB::Json::JsonValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return TissDB::Json::JsonValue(nullptr);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return TissDB::Json::JsonValue(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                return TissDB::Json::JsonValue(arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                return TissDB::Json::JsonValue(arg);
            } else {
                // Handle other types as needed
                return TissDB::Json::JsonValue(nullptr);
            }
        }, elem.value);
    }
    obj["elements"] = elements;
    return TissDB::Json::JsonValue(obj).serialize();
}

TissDBClient::TissDBClient(const std::string& host, int port, const std::string& db_name, const std::string& token)
    : host_(host), port_(port), db_name_(db_name) {
    base_url_ = "http://" + host_ + ":" + std::to_string(port_);
    db_url_ = base_url_ + "/" + db_name_; // Corrected DB URL
    http_client_ = std::make_unique<HttpClient>();
    if (!token.empty()) {
        http_client_->set_token(token);
    }
}

TissDBClient::~TissDBClient() = default;

bool TissDBClient::ensure_db_setup(const std::vector<std::string>& collections) {
    for (const auto& collection_name : collections) {
        std::string url = db_url_ + "/" + collection_name;
        http_client_->put(url, "");
    }
    return true;
}

std::string TissDBClient::add_document(const std::string& collection, const TissDB::Document& document, const std::string& doc_id) {
    std::string url = db_url_ + "/" + collection;
    std::string doc_json = to_json(document);
    std::string response_json;

    if (!doc_id.empty()) {
        // PUT to /<db>/<collection>/<doc_id> for creation/update with specific ID
        url += "/" + doc_id;
        response_json = http_client_->put(url, doc_json);
    } else {
        // POST to /<db>/<collection> for creation with generated ID
        response_json = http_client_->post(url, doc_json);
    }

    TissDB::Json::JsonValue json = TissDB::Json::JsonValue::parse(response_json);
    if (json.is_object() && json.as_object().count("id")) {
        return json.as_object().at("id").as_string();
    }
    // PUT might return the full document, not an ID object.
    // If an ID was provided, we can assume it was successful on a 2xx response.
    if (!doc_id.empty()) {
        return doc_id;
    }
    return "";
}

TissDB::Document TissDBClient::get_document(const std::string& collection, const std::string& doc_id) {
    std::string url = db_url_ + "/" + collection + "/" + doc_id;
    std::string response_json = http_client_->get(url);
    return from_json(response_json);
}

std::map<std::string, std::string> TissDBClient::get_stats() {
    return {}; // Dummy stats
}

void TissDBClient::delete_database() {
    http_client_->del(db_url_);
}

void TissDBClient::create_database() {
    http_client_->put(db_url_, "");
}

std::string TissDBClient::add_feedback(const TissDB::Document& feedback_data) {
    // Corrected to post to a collection within the database, e.g., /<db_name>/feedback
    std::string url = db_url_ + "/feedback";
    std::string feedback_json = to_json(feedback_data);
    std::string response_json = http_client_->post(url, feedback_json);
    TissDB::Json::JsonValue json = TissDB::Json::JsonValue::parse(response_json);
    if (json.as_object().count("id")) {
        return json.as_object().at("id").as_string();
    }
    return "";
}

bool TissDBClient::test_connection() {
    try {
        // Make a GET request to the base URL.
        // If the request is successful, the connection is considered valid.
        http_client_->get(base_url_);
        return true;
    } catch (const HttpClientException& e) {
        // If an exception is thrown, the connection is considered invalid.
        std::cerr << "Connection test failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<TissDB::Document> TissDBClient::search_documents(const std::string& collection, const std::string& query_json) {
    std::string url = db_url_ + "/" + collection + "/_search";
    std::string response_json = http_client_->post(url, query_json);
    TissDB::Json::JsonValue json = TissDB::Json::JsonValue::parse(response_json);
    std::vector<TissDB::Document> docs;
    for (const auto& val : json.as_array()) {
        docs.push_back(from_json(val.serialize()));
    }
    return docs;
}

std::string TissDBClient::query(const std::string& collection, const std::string& query_string) {
    std::string url = db_url_;
    if (!collection.empty()) {
        url += "/" + collection;
    }
    url += "/_query";
    TissDB::Json::JsonObject data;
    data["query"] = TissDB::Json::JsonValue(query_string);
    std::string query_json = TissDB::Json::JsonValue(data).serialize();
    return http_client_->post(url, query_json);
}

} // namespace TissDB
