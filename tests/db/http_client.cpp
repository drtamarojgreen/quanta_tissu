#include "http_client.h"
#include <iostream>
#include <stdexcept>
#include <cstring> // For memset, strlen
#include <sstream> // For stringstream

// For socket programming
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h> // For close
    #include <netdb.h> // For gethostbyname
#endif

namespace TissDB {
namespace Test {

HttpClient::HttpClient(const std::string& host, int port) : host_(host), port_(port) {
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
    }
#endif
}

HttpClient::~HttpClient() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int HttpClient::connect_to_server() {
    int sock = -1;
#ifdef _WIN32
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == static_cast<int>(INVALID_SOCKET)) {
        throw std::runtime_error("Error creating socket: " + std::to_string(WSAGetLastError()));
    }
#else
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Error creating socket");
    }
#endif

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);

    // Resolve hostname
    struct hostent* server = gethostbyname(host_.c_str());
    if (server == NULL) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        throw std::runtime_error("Error: No such host " + host_);
    }
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        throw std::runtime_error("Error connecting to server");
    }
    return sock;
}

HttpResponse HttpClient::send_request(const std::string& method, const std::string& path, const std::string& body, const std::string& content_type) {
    int sock = connect_to_server();
    std::stringstream request_ss;

    request_ss << method << " " << path << " HTTP/1.1\r\n";
    request_ss << "Host: " << host_ << ":" << port_ << "\r\n";
    request_ss << "User-Agent: TissDB-BDD-Client/1.0\r\n";
    request_ss << "Accept: */*\r\n";

    if (!body.empty()) {
        request_ss << "Content-Type: " << content_type << "\r\n";
        request_ss << "Content-Length: " << body.length() << "\r\n";
    }
    request_ss << "Connection: close\r\n"; // Close connection after response
    request_ss << "\r\n"; // End of headers

    if (!body.empty()) {
        request_ss << body;
    }

    std::string request_str = request_ss.str();

    if (send(sock, request_str.c_str(), request_str.length(), 0) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        throw std::runtime_error("Error sending request");
    }

    // Read response
    std::string raw_response;
    char buffer[4096];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        raw_response.append(buffer);
    }

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    if (bytes_received < 0) {
        throw std::runtime_error("Error receiving response");
    }

    return parse_response(raw_response);
}

HttpResponse HttpClient::parse_response(const std::string& raw_response) {
    HttpResponse response;
    std::istringstream iss(raw_response);
    std::string line;

    // Parse status line
    std::getline(iss, line);
    std::istringstream status_line_ss(line);
    std::string http_version;
    status_line_ss >> http_version >> response.status_code;

    // Parse headers
    while (std::getline(iss, line) && line != "\r") { // Headers end with an empty line
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 2); // +2 to skip ": "
            if (!value.empty() && value.back() == '\r') {
                value.pop_back(); // Remove carriage return
            }
            response.headers[key] = value;
        }
    }

    // Parse body
    std::string body_line;
    while (std::getline(iss, body_line)) {
        response.body += body_line;
        if (iss.peek() != EOF) { // Add newline back if not the last line
            response.body += "\n";
        }
    }
    if (!response.body.empty() && response.body.back() == '\n') {
        response.body.pop_back(); // Remove trailing newline if present
    }


    return response;
}

HttpResponse HttpClient::get(const std::string& path) {
    return send_request("GET", path);
}

HttpResponse HttpClient::post(const std::string& path, const std::string& body, const std::string& content_type) {
    return send_request("POST", path, body, content_type);
}

HttpResponse HttpClient::put(const std::string& path, const std::string& body, const std::string& content_type) {
    return send_request("PUT", path, body, content_type);
}

HttpResponse HttpClient::del(const std::string& path) {
    return send_request("DELETE", path);
}

} // namespace Test
} // namespace TissDB
