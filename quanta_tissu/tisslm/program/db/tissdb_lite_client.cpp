#include "tissdb_lite_client.h"
#include "http_client.h"

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

} // namespace TissDB