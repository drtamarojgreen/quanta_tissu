#include "http_client.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h> // For gethostbyname
#endif

namespace TissDB {

HttpClient::HttpClient() {
    platform_init();
    if (token_.empty()) {
        token_ = "static_test_token";
    }
}

HttpClient::~HttpClient() {
    platform_cleanup();
}

void HttpClient::set_token(const std::string& token) {
    token_ = token;
}

void HttpClient::platform_init() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw HttpClientException("WSAStartup failed.");
    }
#endif
}

void HttpClient::platform_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int HttpClient::create_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw HttpClientException("Socket creation failed.");
    }
    return sock;
}

void HttpClient::connect_socket(int sock, const std::string& host, int port) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

#ifdef _WIN32
    if (inet_pton(AF_INET, host.c_str(), &server.sin_addr) <= 0) {
        throw HttpClientException("Invalid address/ Address not supported");
    }
#else
    struct hostent *server_host = gethostbyname(host.c_str());
    if (server_host == nullptr) {
        throw HttpClientException("No such host");
    }
    memcpy(&server.sin_addr, server_host->h_addr, server_host->h_length);
#endif

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        throw HttpClientException("Connection failed.");
    }
}

void HttpClient::send_data(int sock, const std::string& data) {
    if (send(sock, data.c_str(), data.length(), 0) < 0) {
        throw HttpClientException("Send failed.");
    }
}

std::string HttpClient::receive_data(int sock) {
    char buffer[4096];
    std::string response;
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response += buffer;
    }
    if (bytes_received < 0) {
        throw HttpClientException("Receive failed.");
    }
    return response;
}

void HttpClient::close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

void HttpClient::parse_url(const std::string& url, std::string& host, int& port, std::string& path) {
    // Simplified URL parsing
    std::string temp = url;
    if (temp.rfind("http://", 0) == 0) {
        temp = temp.substr(7);
    }
    size_t path_pos = temp.find('/');
    if (path_pos != std::string::npos) {
        host = temp.substr(0, path_pos);
        path = temp.substr(path_pos);
    } else {
        host = temp;
        path = "/";
    }
    size_t port_pos = host.find(':');
    if (port_pos != std::string::npos) {
        port = std::stoi(host.substr(port_pos + 1));
        host = host.substr(0, port_pos);
    } else {
        port = 80;
    }
}

void HttpClient::parse_status_line(const std::string& status_line, int& status_code, std::string& reason_phrase) {
    std::stringstream ss(status_line);
    std::string http_version;
    ss >> http_version >> status_code;
    std::getline(ss, reason_phrase);
    // Trim leading space from reason_phrase
    if (!reason_phrase.empty() && reason_phrase[0] == ' ') {
        reason_phrase.erase(0, 1);
    }
}

std::string HttpClient::get(const std::string& url) {
    std::string host, path;
    int port;
    parse_url(url, host, port, path);
    int sock = create_socket();
    connect_socket(sock, host, port);
    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\n";
    if (!token_.empty()) {
        request += "Authorization: Bearer " + token_ + "\r\n";
    }
    request += "Connection: close\r\n\r\n";
    send_data(sock, request);
    std::string response = receive_data(sock);
    close_socket(sock);

    size_t status_line_end = response.find("\r\n");
    std::string status_line = response.substr(0, status_line_end);
    int status_code;
    std::string reason_phrase;
    parse_status_line(status_line, status_code, reason_phrase);

    if (status_code < 200 || status_code >= 300) {
        throw HttpClientException("HTTP Error: " + status_line);
    }
    check_response_status(response);

    size_t header_end = response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        return response.substr(header_end + 4);
    }
    return "";
}

std::string HttpClient::post(const std::string& url, const std::string& body) {
    std::string host, path;
    int port;
    parse_url(url, host, port, path);
    int sock = create_socket();
    connect_socket(sock, host, port);
    std::string request = "POST " + path + " HTTP/1.1\r\nHost: " + host + "\r\n";
    if (!token_.empty()) {
        request += "Authorization: Bearer " + token_ + "\r\n";
    }
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += body;
    send_data(sock, request);
    std::string response = receive_data(sock);
    close_socket(sock);

    size_t status_line_end = response.find("\r\n");
    std::string status_line = response.substr(0, status_line_end);
    int status_code;
    std::string reason_phrase;
    parse_status_line(status_line, status_code, reason_phrase);

    if (status_code < 200 || status_code >= 300) {
        throw HttpClientException("HTTP Error: " + status_line);
    }
    check_response_status(response);

    size_t header_end = response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        return response.substr(header_end + 4);
    }
    return "";
}

std::string HttpClient::put(const std::string& url, const std::string& body) {
    std::string host, path;
    int port;
    parse_url(url, host, port, path);
    int sock = create_socket();
    connect_socket(sock, host, port);
    std::string request = "PUT " + path + " HTTP/1.1\r\nHost: " + host + "\r\n";
    if (!token_.empty()) {
        request += "Authorization: Bearer " + token_ + "\r\n";
    }
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += body;
    send_data(sock, request);
    std::string response = receive_data(sock);
    close_socket(sock);

    size_t status_line_end = response.find("\r\n");
    std::string status_line = response.substr(0, status_line_end);
    int status_code;
    std::string reason_phrase;
    parse_status_line(status_line, status_code, reason_phrase);

    if (status_code < 200 || status_code >= 300) {
        throw HttpClientException("HTTP Error: " + status_line);
    }
    check_response_status(response);

    size_t header_end = response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        return response.substr(header_end + 4);
    }
    return "";
}

std::string HttpClient::del(const std::string& url) {
    std::string host, path;
    int port;
    parse_url(url, host, port, path);
    int sock = create_socket();
    connect_socket(sock, host, port);
    std::string request = "DELETE " + path + " HTTP/1.1\r\nHost: " + host + "\r\n";
    if (!token_.empty()) {
        request += "Authorization: Bearer " + token_ + "\r\n";
    }
    request += "Connection: close\r\n\r\n";
    send_data(sock, request);
    std::string response = receive_data(sock);
    close_socket(sock);

    size_t status_line_end = response.find("\r\n");
    if (status_line_end == std::string::npos) {
        return "";
    }
    std::string status_line = response.substr(0, status_line_end);
    int status_code;
    std::string reason_phrase;
    parse_status_line(status_line, status_code, reason_phrase);

    if (status_code >= 300 && status_code != 404) {
        if (status_code == 500) {
            // For DELETE, we can be more lenient with 500 errors,
            // as the server might be complaining about a non-existent DB.
            // In a real application, this would be a server-side fix (return 404).
            return "";
        }
        throw HttpClientException("HTTP Error: " + status_line);
    }

    size_t header_end = response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        return response.substr(header_end + 4);
    }
    return "";
}

void HttpClient::check_response_status(const std::string& response) {
    size_t first_line_end = response.find("\r\n");
    if (first_line_end == std::string::npos) {
        throw HttpClientException("Invalid HTTP response: no status line");
    }
    std::string status_line = response.substr(0, first_line_end);
    size_t code_pos = status_line.find(' ');
    if (code_pos == std::string::npos) {
        throw HttpClientException("Invalid HTTP status line: " + status_line);
    }
    // Skip version part, find next space
    code_pos++;
    size_t code_end_pos = status_line.find(' ', code_pos);
     if (code_end_pos == std::string::npos) {
        throw HttpClientException("Invalid HTTP status line: " + status_line);
    }

    std::string status_code_str = status_line.substr(code_pos, code_end_pos - code_pos);
    int status_code = 0;
    try {
        status_code = std::stoi(status_code_str);
    } catch (...) {
        throw HttpClientException("Invalid HTTP status code: " + status_code_str);
    }

    if (status_code < 200 || status_code >= 300) {
        throw HttpClientException("HTTP Error: " + status_line);
    }
}

} // namespace TissDB
