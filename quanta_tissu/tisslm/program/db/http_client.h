#ifndef TISSLM_HTTP_CLIENT_H
#define TISSLM_HTTP_CLIENT_H

#include <string>
#include <stdexcept>

namespace TissDB {

class HttpClientException : public std::runtime_error {
public:
    explicit HttpClientException(const std::string& message)
        : std::runtime_error(message) {}
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    void set_token(const std::string& token);
    std::string get(const std::string& url);
    std::string post(const std::string& url, const std::string& body);
    std::string put(const std::string& url, const std::string& body);
    std::string del(const std::string& url);

private:
    std::string token_;
    // Platform-specific socket initialization/cleanup
    void platform_init();
    void platform_cleanup();

    // Platform-specific socket creation, connection, send, receive
    int create_socket();
    void connect_socket(int sock, const std::string& host, int port);
    void send_data(int sock, const std::string& data);
    std::string receive_data(int sock);
    void close_socket(int sock);

    // Helper to parse URL
    void parse_url(const std::string& url, std::string& host, int& port, std::string& path);
    // Helper to parse status line
    void parse_status_line(const std::string& status_line, int& status_code, std::string& reason_phrase);

    // Helper to check HTTP status
};

} // namespace TissDB

#endif // TISSLM_HTTP_CLIENT_H
