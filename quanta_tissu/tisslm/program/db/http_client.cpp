#include "http_client.h"
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // For gethostbyname
#endif

namespace TissDB {

HttpClient::HttpClient() {
    platform_init();
}

HttpClient::~HttpClient() {
    platform_cleanup();
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

std::string HttpClient::get(const std::string& url) {
    std::string host, path;
    int port;
    parse_url(url, host, port, path);

    int sock = create_socket();
    connect_socket(sock, host, port);

    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    send_data(sock, request);

    std::string response = receive_data(sock);
    close_socket(sock);

    return response;
}

std::string HttpClient::post(const std::string& url, const std::string& body) {
    std::string host, path;
    int port;
    parse_url(url, host, port, path);

    int sock = create_socket();
    connect_socket(sock, host, port);

    std::string request = "POST " + path + " HTTP/1.1\r\nHost: " + host + "\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.length()) + "\r\nConnection: close\r\n\r\n" + body;
    send_data(sock, request);

    std::string response = receive_data(sock);
    close_socket(sock);

    return response;
}

} // namespace TissDB
