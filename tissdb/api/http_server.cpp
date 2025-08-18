#include "http_server.h"
#include "../common/log.h"
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

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h> // For inet_pton, etc.
    #pragma comment(lib, "ws2_32.lib") // Link with Winsock library

    // Undefine the ERROR macro which is defined in winsock2.h and conflicts with LogLevel::ERROR.
    #undef ERROR
#else
    // POSIX Socket headers
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> // For close()
#endif

#ifdef _WIN32
    // Type redefinitions
    using socklen_t = int; // Winsock uses int for socklen_t

    // Function redefinitions
    #define close(s) closesocket(s)
    #define SHUT_RDWR SD_BOTH // For shutdown()
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND

    // Winsock specific initialization/cleanup
    // These will be called in constructor/destructor
#endif

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
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
    }
#endif
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) throw std::runtime_error("Socket creation failed.");
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
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
#ifdef _WIN32
    WSACleanup();
#endif
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

    LOG_INFO("Incoming request: " + req.method + " " + req.path);

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
            LOG_INFO("Sending response: 200 OK");
            send_response(client_socket, "200 OK", "text/plain", "Transaction started with ID: " + std::to_string(transaction_id));
        } else if (req.method == "POST" && path_parts.size() == 1 && path_parts[0] == "_commit") {
            if (transaction_id != -1) {
                storage_engine.commit_transaction(transaction_id);
                client_transactions_.erase(client_socket);
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "text/plain", "Transaction committed.");
            } else {
                LOG_WARNING("Sending response: 400 Bad Request (No active transaction)");
                send_response(client_socket, "400 Bad Request", "text/plain", "No active transaction.");
            }
        } else if (req.method == "POST" && path_parts.size() == 1 && path_parts[0] == "_rollback") {
            if (transaction_id != -1) {
                storage_engine.rollback_transaction(transaction_id);
                client_transactions_.erase(client_socket);
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "text/plain", "Transaction rolled back.");
            } else {
                LOG_WARNING("Sending response: 400 Bad Request (No active transaction)");
                send_response(client_socket, "400 Bad Request", "text/plain", "No active transaction.");
            }
        } else if (req.method == "POST" && path_parts.size() == 2 && path_parts[0] == "_bi" && path_parts[1] == "dashboards") {
            // POST /_bi/dashboards
            try {
                // Ensure the _bi_dashboards collection exists
                bool collection_exists = false;
                auto collections = storage_engine.list_collections();
                for (const auto& name : collections) {
                    if (name == "_bi_dashboards") {
                        collection_exists = true;
                        break;
                    }
                }
                if (!collection_exists) {
                    storage_engine.create_collection("_bi_dashboards", TissDB::Schema());
                }

                Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                Document doc = json_to_document(parsed_body.as_object());
                // Generate a simple ID
                std::string id = "_bi_dashboard_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                doc.id = id;
                storage_engine.put("_bi_dashboards", id, doc, transaction_id);
                LOG_INFO("Sending response: 201 Created");
                send_response(client_socket, "201 Created", "application/json", "{\"id\": \"" + id + "\"}");
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid JSON body)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
            }
        } else if (req.method == "GET" && path_parts.size() == 3 && path_parts[0] == "_bi" && path_parts[1] == "dashboards") {
            // GET /_bi/dashboards/<id>
            std::string doc_id = path_parts[2];
            auto doc_opt = storage_engine.get("_bi_dashboards", doc_id, transaction_id);
            if (doc_opt) {
                Json::JsonValue json_doc(document_to_json(*(*doc_opt)));
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "application/json", json_doc.serialize());
            } else {
                LOG_INFO("Sending response: 404 Not Found");
                send_response(client_socket, "404 Not Found", "text/plain", "Dashboard not found.");
            }
        } else if (req.method == "PUT" && path_parts.size() == 3 && path_parts[0] == "_bi" && path_parts[1] == "dashboards") {
            // PUT /_bi/dashboards/<id>
            try {
                Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                Document doc = json_to_document(parsed_body.as_object());
                doc.id = path_parts[2];
                storage_engine.put("_bi_dashboards", path_parts[2], doc, transaction_id);
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "application/json", parsed_body.serialize());
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid JSON body)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
            }
        } else if (req.method == "GET" && path_parts.size() == 1 && path_parts[0] == "_health") {
            LOG_INFO("Sending response: 200 OK");
            send_response(client_socket, "200 OK", "text/plain", "OK");
            close(client_socket);
            return;
        } else if (req.method == "GET" && path_parts.size() == 1 && path_parts[0] == "_collections") {
            // This is a placeholder. A real implementation would query the storage engine.
            Json::JsonArray collections_array;
            // collections_array.push_back(Json::JsonValue("my_test_collection"));
            // collections_array.push_back(Json::JsonValue("documents_collection"));
            LOG_INFO("Sending response: 200 OK");
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(collections_array).serialize());
            close(client_socket);
            return;
        } else if (path_parts.empty()) {
            LOG_WARNING("Sending response: 400 Bad Request (Collection name missing)");
            send_response(client_socket, "400 Bad Request", "text/plain", "Collection name missing from URL.");
            close(client_socket);
            return;
        }

        std::string collection_name = path_parts[0];

        if (req.method == "GET" && path_parts.size() == 2 && path_parts[1] == "_all") {
            // This is a placeholder. A real implementation would query the storage engine.
            Json::JsonArray docs_array;
            // docs_array.push_back(Json::JsonValue("doc_a"));
            // docs_array.push_back(Json::JsonValue("doc_b"));
            LOG_INFO("Sending response: 200 OK");
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(docs_array).serialize());
            close(client_socket);
            return;
        }

        if (req.method == "GET" && path_parts.size() == 2 && path_parts[1] != "_all") {
            // GET /<collection>/<id>
            std::string doc_id = path_parts[1];
            auto doc_opt = storage_engine.get(collection_name, doc_id, transaction_id);
            if (doc_opt) {
                Json::JsonValue json_doc(document_to_json(*(*doc_opt)));
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "application/json", json_doc.serialize());
            } else {
                LOG_INFO("Sending response: 404 Not Found");
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
                LOG_INFO("Sending response: 201 Created");
                send_response(client_socket, "201 Created", "text/plain", "Document created with ID: " + id);
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid JSON body)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
            }
        } else if (req.method == "PUT" && path_parts.size() == 2) {
            // PUT /<collection>/<id>
            try {
                Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                Document doc = json_to_document(parsed_body.as_object());
                doc.id = path_parts[1];
                storage_engine.put(collection_name, path_parts[1], doc, transaction_id);
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "application/json", parsed_body.serialize());
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid JSON body)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body: " + std::string(e.what()));
            }
        } else if (req.method == "DELETE" && path_parts.size() == 2) {
            // DELETE /<collection>/<id>
            std::string doc_id = path_parts[1];
            storage_engine.del(collection_name, doc_id, transaction_id);
            LOG_INFO("Sending response: 204 No Content");
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

                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "application/json", Json::JsonValue(result_array).serialize());
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid query)");
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

                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "text/plain", "Index created on fields: " + parsed_body.serialize());
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid index request)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid index request: " + std::string(e.what()));
            }
        } else if (req.method == "PUT" && path_parts.size() == 1) {
            // PUT /<collection> (Create collection)
            try {
                storage_engine.create_collection(collection_name, TissDB::Schema());
                LOG_INFO("Sending response: 201 Created");
                send_response(client_socket, "201 Created", "text/plain", "Collection '" + collection_name + "' created.");
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Failed to create collection)");
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
                LOG_INFO("Sending response: 200 OK");
                send_response(client_socket, "200 OK", "text/plain", "Schema updated for collection '" + collection_name + "'.");

            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Invalid schema format)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid schema format: " + std::string(e.what()));
            }
        } else if (req.method == "DELETE" && path_parts.size() == 1) {
            // DELETE /<collection> (Delete collection)
            try {
                storage_engine.delete_collection(collection_name);
                LOG_INFO("Sending response: 204 No Content");
                send_response(client_socket, "204 No Content", "text/plain", "");
            } catch (const std::exception& e) {
                LOG_ERROR("Sending response: 400 Bad Request (Failed to delete collection)");
                send_response(client_socket, "400 Bad Request", "text/plain", "Failed to delete collection: " + std::string(e.what()));
            }
        } else {
            LOG_WARNING("Sending response: 404 Not Found (Endpoint not found)");
            send_response(client_socket, "404 Not Found", "text/plain", "Endpoint not found.");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Sending response: 500 Internal Server Error: " + std::string(e.what()));
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
