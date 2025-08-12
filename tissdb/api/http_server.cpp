#include "http_server.h"
#include "../storage/lsm_tree.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cstring> // for strerror
#include <sstream>

// --- Platform-specific socket includes ---
// Assuming a POSIX-compliant environment (Linux, macOS)
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for close, read, write, shutdown

namespace TissDB {
namespace API {

// The PIMPL implementation class using low-level sockets.
class HttpServer::Impl {
public:
    Impl(Storage::LSMTree& storage, int port);
    ~Impl();

    void start();
    void stop();

private:
    void server_loop();
    void handle_client(int client_socket);
    void handle_request(const std::string& request_str, int client_socket);

    Storage::LSMTree& storage_engine;
    int server_fd = -1;
    int server_port;
    std::atomic<bool> is_running{false};
    std::thread server_thread;
};

// --- Implementation of HttpServer::Impl ---

HttpServer::Impl::Impl(Storage::LSMTree& storage, int port)
    : storage_engine(storage), server_port(port) {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }
}

HttpServer::Impl::~Impl() {
    stop();
    if (server_fd != -1) {
        close(server_fd);
    }
}

void HttpServer::Impl::start() {
    if (listen(server_fd, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
    is_running = true;
    server_thread = std::thread(&HttpServer::Impl::server_loop, this);
}

void HttpServer::Impl::stop() {
    if (is_running.exchange(false)) {
        if (server_fd != -1) {
            shutdown(server_fd, SHUT_RDWR);
        }
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }
}

void HttpServer::Impl::server_loop() {
    while (is_running) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            if (is_running) {
                 std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            }
            continue;
        }
        std::thread(&HttpServer::Impl::handle_client, this, client_socket).detach();
    }
}

void HttpServer::Impl::handle_client(int client_socket) {
    const int buffer_size = 2048;
    char buffer[buffer_size] = {0};

    if (recv(client_socket, buffer, buffer_size - 1, 0) > 0) {
        handle_request(std::string(buffer), client_socket);
    }

    close(client_socket);
}

void HttpServer::Impl::handle_request(const std::string& request_str, int client_socket) {
    std::string method, path;
    std::stringstream request_line_ss(request_str.substr(0, request_str.find("\r\n")));
    request_line_ss >> method >> path;

    std::string response_body;
    std::string status_code = "200 OK";

    if (method == "GET" && path == "/") {
        response_body = "Welcome to the library-free TissDB!";
    } else {
        response_body = "Not Found";
        status_code = "404 Not Found";
    }

    std::stringstream response_ss;
    response_ss << "HTTP/1.1 " << status_code << "\r\n";
    response_ss << "Content-Type: text/plain\r\n";
    response_ss << "Content-Length: " << response_body.length() << "\r\n";
    response_ss << "Connection: close\r\n";
    response_ss << "\r\n";
    response_ss << response_body;

    std::string response = response_ss.str();
    send(client_socket, response.c_str(), response.length(), 0);
}

// --- Public HttpServer methods that delegate to the PIMPL ---

HttpServer::HttpServer(Storage::LSMTree& storage_engine, int port)
    : pimpl(std::make_unique<Impl>(storage_engine, port)) {}

HttpServer::~HttpServer() = default;

void HttpServer::start() {
    pimpl->start();
}

void HttpServer::stop() {
    pimpl->stop();
}

} // namespace API
} // namespace TissDB
