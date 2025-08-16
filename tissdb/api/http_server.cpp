#include "http_server.h"
#include "../common/schema.h"
#include "../storage/lsm_tree.h"
#include "../json/json.h"
#include "../common/document.h"
#include "../query/parser.h"
#include "../query/ast.h"
#include "../query/executor.h"
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
#include <arpa/inet.h>
#include <unistd.h> // For close()

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
    std::map<int, int> client_transactions_;
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

    Transactions::TransactionID transaction_id = -1;
    if (client_transactions_.count(client_socket)) {
        transaction_id = client_transactions_[client_socket];
    }

    try {
        if (req.method == "POST" && path_parts.size() == 1 && path_parts[0] == "_begin") {
            transaction_id = storage_engine.begin_transaction();
            client_transactions_[client_socket] = transaction_id;
            send_response(client_socket, "200 OK", "text/plain", "Transaction started with ID: " + std::to_string(transaction_id));
        } else if (req.method == "POST" && path_parts.size() == 1 && path_parts[0] == "_commit") {
            if (transaction_id != -1) {
                storage_engine.commit_transaction(transaction_id);
                client_transactions_.erase(client_socket);
                send_response(client_socket, "200 OK", "text/plain", "Transaction committed.");
            } else {
                send_response(client_socket, "400 Bad Request", "text/plain", "No active transaction.");
            }
        } else if (req.method == "POST" && path_parts.size() == 1 && path_parts[0] == "_rollback") {
            if (transaction_id != -1) {
                storage_engine.rollback_transaction(transaction_id);
                client_transactions_.erase(client_socket);
                send_response(client_socket, "200 OK", "text/plain", "Transaction rolled back.");
            } else {
                send_response(client_socket, "400 Bad Request", "text/plain", "No active transaction.");
            }
        } else if (path_parts.empty()) {
            send_response(client_socket, "400 Bad Request", "text/plain", "Collection name missing from URL.");
            close(client_socket);
            return;
        }

        std::string collection_name = path_parts[0];

        if (req.method == "GET" && path_parts.size() == 2) {
            // GET /<collection>/<id>
            std::string doc_id = path_parts[1];
            auto doc_opt = storage_engine.get(collection_name, doc_id, transaction_id);
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
                storage_engine.put(collection_name, id, doc, transaction_id);
                send_response(client_socket, "201 Created", "text/plain", "Document created with ID: " + id);
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
            }
        } else if (req.method == "PUT" && path_parts.size() == 2) {
            // PUT /<collection>/<id>
            try {
                Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                Document doc = json_to_document(parsed_body.as_object());
                doc.id = path_parts[1];
                storage_engine.put(collection_name, path_parts[1], doc, transaction_id);
                send_response(client_socket, "200 OK", "application/json", parsed_body.serialize());
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
            }
        } else if (req.method == "DELETE" && path_parts.size() == 2) {
            // DELETE /<collection>/<id>
            std::string doc_id = path_parts[1];
            storage_engine.del(collection_name, doc_id, transaction_id);
            send_response(client_socket, "204 No Content", "text/plain", "");
        } else if (req.method == "POST" && path_parts.size() == 2 && path_parts[1] == "_query") {
            // POST /<collection>/_query
            try {
                const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                std::string query_string = parsed_body.as_object().at("query").as_string();

                Query::Parser parser;
                Query::AST ast = parser.parse(query_string);

                Query::Executor executor(storage_engine);
                Query::QueryResult result = executor.execute(ast);

                Json::JsonArray result_array;
                for (const auto& doc : result) {
                    result_array.push_back(Json::JsonValue(document_to_json(doc)));
                }

                send_response(client_socket, "200 OK", "application/json", Json::JsonValue(result_array).serialize());
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid query: " + std::string(e.what()));
            }
        } else if (req.method == "POST" && path_parts.size() == 2 && path_parts[1] == "_index") {
            // POST /<collection>/_index
            try {
                const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                std::vector<std::string> field_names;
                if (parsed_body.as_object().count("field")) {
                    field_names.push_back(parsed_body.as_object().at("field").as_string());
                } else if (parsed_body.as_object().count("fields")) {
                    for (const auto& field : parsed_body.as_object().at("fields").as_array()) {
                        field_names.push_back(field.as_string());
                    }
                }

                storage_engine.create_index(collection_name, field_names);

                send_response(client_socket, "200 OK", "text/plain", "Index created on fields: " + parsed_body.serialize());
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid index request: " + std::string(e.what()));
            }
        } else if (req.method == "PUT" && path_parts.size() == 1) {
            // PUT /<collection> (Create collection)
            try {
                storage_engine.create_collection(collection_name, TissDB::Schema());
                send_response(client_socket, "201 Created", "text/plain", "Collection '" + collection_name + "' created.");
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Failed to create collection: " + std::string(e.what()));
            }
        } else if (req.method == "PUT" && path_parts.size() == 2 && path_parts[1] == "_schema") {
            // PUT /<collection>/_schema
            try {
                const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                const Json::JsonObject& schema_json = parsed_body.as_object();
                
                TissDB::Schema schema;
                if (schema_json.count("fields")) {
                    const Json::JsonArray& fields = schema_json.at("fields").as_array();
                    for (const auto& field_val : fields) {
                        const Json::JsonObject& field_obj = field_val.as_object();
                        std::string name = field_obj.at("name").as_string();
                        std::string type_str = field_obj.at("type").as_string();
                        bool required = field_obj.count("required") ? field_obj.at("required").as_bool() : false;
                        
                        TissDB::FieldType type;
                        if (type_str == "String") type = TissDB::FieldType::String;
                        else if (type_str == "Number") type = TissDB::FieldType::Number;
                        else if (type_str == "Boolean") type = TissDB::FieldType::Boolean;
                        else if (type_str == "DateTime") type = TissDB::FieldType::DateTime;
                        else if (type_str == "Binary") type = TissDB::FieldType::Binary;
                        else if (type_str == "Object") type = TissDB::FieldType::Object;
                        else if (type_str == "Array") type = TissDB::FieldType::Array;
                        else {
                            throw std::runtime_error("Unknown field type: " + type_str);
                        }
                        
                        schema.add_field(name, type, required);
                    }
                }
                
                storage_engine.get_collection(collection_name).set_schema(schema);
                send_response(client_socket, "200 OK", "text/plain", "Schema updated for collection '" + collection_name + "'.");

            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid schema format: " + std::string(e.what()));
            }
        } else if (req.method == "DELETE" && path_parts.size() == 1) {
            // DELETE /<collection> (Delete collection)
            try {
                storage_engine.delete_collection(collection_name);
                send_response(client_socket, "204 No Content", "text/plain", "");
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Failed to delete collection: " + std::string(e.what()));
            }
        } else {
            send_response(client_socket, "404 Not Found", "text/plain", "Endpoint not found.");
        }
    } catch (const std::exception& e) {
        send_response(client_socket, "500 Internal Server Error", "text/plain", "Server error: " + std::string(e.what()));
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
