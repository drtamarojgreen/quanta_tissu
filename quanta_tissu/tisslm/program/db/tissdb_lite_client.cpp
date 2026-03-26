#include "tissdb_lite_client.h"
#include "http_client.h"
#include "tissdb/json/json.h"

namespace TissDB {

TissDBLiteClient::TissDBLiteClient(const std::string& host, int port) {
    base_url_ = "http://" + host + ":" + std::to_string(port);
    http_client_ = std::make_unique<HttpClient>();
}

TissDBLiteClient::~TissDBLiteClient() = default;

std::string TissDBLiteClient::sendCommand(const std::string& command_json) {
    std::string url = base_url_ + "/command";
    return http_client_->post(url, command_json);
}

std::map<std::string, std::string> TissDBLiteClient::get_stats() {
    std::map<std::string, std::string> stats;
    try {
        std::string response = http_client_->get(base_url_ + "/_stats");
        auto root = TissDB::Json::JsonValue::parse(response);
        if (root.is_object()) {
            for (const auto& pair : root.as_object()) {
                if (pair.second.is_string()) stats[pair.first] = pair.second.as_string();
                else if (pair.second.is_number()) stats[pair.first] = std::to_string(pair.second.as_number());
            }
        }
    } catch (...) { stats["status"] = "error"; }
    return stats;
}

}
