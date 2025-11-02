#ifndef TISSLM_TISSDB_LITE_CLIENT_H
#define TISSLM_TISSDB_LITE_CLIENT_H

#include <string>
#include <vector>
#include <memory>

#include "../../../../tissdb/common/document.h"

namespace TissDB {

class HttpClient;

class TissDBLiteClient {
public:
    TissDBLiteClient(const std::string& host = "127.0.0.1", int port = 9877);
    ~TissDBLiteClient();

    std::string sendCommand(const std::string& command_json);

private:
    std::string base_url_;
    std::unique_ptr<HttpClient> http_client_;
};

} // namespace TissDB

#endif // TISSLM_TISSDB_LITE_CLIENT_H