#include "http_server.h"
#include "../storage/lsm_tree.h"
#include "../json/json.h"
#include "../common/document.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <map>

// POSIX Socket headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace TissDB {
namespace API {

// --- Helper Functions ---
namespace {

// Converts a TissDB Document to a JSON Object
Json::JsonObject document_to_json(const Document& doc) {
    Json::JsonObject obj;
    obj["id"] = Json::JsonValue(doc.id);
    for (const auto& elem : doc.elements) {
        // This is a simplification. A full implementation would handle
        // nested elements and different value types correctly.
        if (const auto* str_val = std::get_if<std::string>(&elem.value)) {
            obj[elem.key] = Json::JsonValue(*str_val);
        } else if (const auto* num_val = std::get_if<double>(&elem.value)) {
            obj[elem.key] = Json::JsonValue(*num_val);
        } else if (const auto* bool_val = std::get_if<bool>(&elem.value)) {
            obj[elem.key] = Json::JsonValue(*bool_val);
        }
    }
    return obj;
}

// Converts a JSON Object to a TissDB Document
Document json_to_document(const Json::JsonObject& obj) {
    Document doc;
    // The document ID is part of the URL, not the body, for PUT/POST.
    // So we only parse the elements.
    for (const auto& pair : obj) {
        if (pair.first == "id") continue; // Skip id field in body
        Element elem;
        elem.key = pair.first;
        if (pair.second.is_string()) {
            elem.value = pair.second.as_string();
        } else if (pair.second.is_number()) {
            elem.value = pair.second.as_number();
        } else if (pair.second.is_bool()) {
            elem.value = pair.second.as_bool();
        }
        // This simplification ignores nested objects and arrays.
        doc.elements.push_back(elem);
    }
    return doc;
}

// A simple struct to hold parsed request info
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};

} // anonymous namespace

class HttpServer::Impl {
public:
    Impl(Storage::LSMTree& storage, int port);
    ~Impl();
    void start();
    void stop();
private:
    void server_loop();
    void handle_client(int client_socket);
    void send_response(int sock, const std::string& code, const std::string& ctype, const std::string& body);

    Storage::LSMTree& storage_engine;
    int server_fd = -1;
    int server_port;
    std::atomic<bool> is_running{false};
    std::thread server_thread;
};

// --- Implementation ---

HttpServer::Impl::Impl(Storage::LSMTree& storage, int port) : storage_engine(storage), server_port(port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) throw std::runtime_error("Socket creation failed.");
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_fd);
        throw std::runtime_error("Socket bind failed.");
    }
}

HttpServer::Impl::~Impl() {
    stop();
    if (server_fd != -1) close(server_fd);
}

void HttpServer::Impl::start() {
    if (listen(server_fd, 10) < 0) throw std::runtime_error("Socket listen failed.");
    is_running = true;
    server_thread = std::thread(&HttpServer::Impl::server_loop, this);
}

void HttpServer::Impl::stop() {
    if (is_running.exchange(false)) {
        if (server_fd != -1) shutdown(server_fd, SHUT_RDWR);
        if (server_thread.joinable()) server_thread.join();
    }
}

void HttpServer::Impl::server_loop() {
    while (is_running) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) continue;
        std::thread(&HttpServer::Impl::handle_client, this, client_socket).detach();
    }
}

void HttpServer::Impl::send_response(int sock, const std::string& code, const std::string& ctype, const std::string& body) {
    std::stringstream ss;
    ss << "HTTP/1.1 " << code << "\r\n";
    ss << "Content-Type: " << ctype << "\r\n";
    ss << "Content-Length: " << body.length() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << body;
    std::string response = ss.str();
    send(sock, response.c_str(), response.length(), 0);
}

void HttpServer::Impl::handle_client(int client_socket) {
    char buffer[4096] = {0};
    if (recv(client_socket, buffer, 4095, 0) <= 0) {
        close(client_socket);
        return;
    }

    std::string request_str(buffer);
    std::stringstream request_ss(request_str);
    HttpRequest req;
    request_ss >> req.method >> req.path;

    // In a real server, we would parse headers and body properly.
    // This is a major simplification.
    size_t body_start = request_str.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        req.body = request_str.substr(body_start + 4);
    }

    // --- Routing ---
    std::vector<std::string> path_parts;
    std::stringstream path_ss(req.path);
    std::string segment;
    while(std::getline(path_ss, segment, '/')) {
       if(!segment.empty()) path_parts.push_back(segment);
    }

    if (req.method == "GET" && path_parts.size() == 2) {
        // GET /<collection>/<id>
        std::string key = path_parts[0] + "/" + path_parts[1];
        auto doc_opt = storage_engine.get(key);
        if (doc_opt) {
            Json::JsonValue json_doc(document_to_json(*doc_opt));
            send_response(client_socket, "200 OK", "application/json", json_doc.serialize());
        } else {
            send_response(client_socket, "404 Not Found", "text/plain", "Document not found.");
        }
    } else if (req.method == "POST" && path_parts.size() == 1) {
        // POST /<collection>
        try {
            Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
            Document doc = json_to_document(parsed_body.as_object());
            // Generate a simple ID
            std::string id = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            doc.id = id;
            std::string key = path_parts[0] + "/" + id;
            storage_engine.put(key, doc);
            send_response(client_socket, "201 Created", "text/plain", "Document created with ID: " + id);
        } catch (const std::exception& e) {
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
        }
    } else if (req.method == "DELETE" && path_parts.size() == 2) {
        // DELETE /<collection>/<id>
        std::string key = path_parts[0] + "/" + path_parts[1];
        storage_engine.del(key);
        send_response(client_socket, "204 No Content", "text/plain", "");
    }
    else {
        send_response(client_socket, "404 Not Found", "text/plain", "Endpoint not found.");
    }

    close(client_socket);
}

// --- Public Methods ---
HttpServer::HttpServer(Storage::LSMTree& storage_engine, int port) : pimpl(std::make_unique<Impl>(storage_engine, port)) {}
HttpServer::~HttpServer() = default;
void HttpServer::start() { pimpl->start(); }
void HttpServer::stop() { pimpl->stop(); }

} // namespace API
} // namespace TissDB
