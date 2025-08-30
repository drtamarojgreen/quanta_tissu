#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <map>

namespace TissDB {
namespace Test {

struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpClient {
public:
    HttpClient(const std::string& host, int port);
    ~HttpClient();

    void set_header(const std::string& key, const std::string& value);

    HttpResponse get(const std::string& path);
    HttpResponse post(const std::string& path, const std::string& body, const std::string& content_type = "application/json");
    HttpResponse put(const std::string& path, const std::string& body, const std::string& content_type = "application/json");
    HttpResponse del(const std::string& path); // 'delete' is a reserved keyword

private:
    std::string host_;
    int port_;
    std::map<std::string, std::string> headers_;

    int connect_to_server();
    HttpResponse send_request(const std::string& method, const std::string& path, const std::string& body = "", const std::string& content_type = "");
    HttpResponse parse_response(const std::string& raw_response);
};

} // namespace Test
} // namespace TissDB

#endif // HTTP_CLIENT_H
