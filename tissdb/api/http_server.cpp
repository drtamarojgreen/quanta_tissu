#include "http_server.h"
#include "../common/log.h"
#include "../common/schema.h"
#include "../storage/database_manager.h"
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
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #undef ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#ifdef _WIN32
    using socklen_t = int;
    #define close(s) closesocket(s)
    #define SHUT_RDWR SD_BOTH
#endif

namespace TissDB {
namespace API {

namespace {

JsonValue value_to_json(const Value& value); // Forward declaration

Json::JsonObject document_to_json(const Document& doc) {
    Json::JsonObject obj;
    obj["_id"] = Json::JsonValue(doc.id);
    for (const auto& elem : doc.elements) {
        obj[elem.key] = value_to_json(elem.value);
    }
    return obj;
}

JsonValue value_to_json(const Value& value) {
    if (std::holds_alternative<std::nullptr_t>(value)) {
        return Json::JsonValue(nullptr);
    } else if (const auto* str_val = std::get_if<std::string>(&value)) {
        return Json::JsonValue(*str_val);
    } else if (const auto* num_val = std::get_if<double>(&value)) {
        return Json::JsonValue(*num_val);
    } else if (const auto* bool_val = std::get_if<bool>(&value)) {
        return Json::JsonValue(*bool_val);
    } else if (const auto* vec_val = std::get_if<std::vector<Value>>(&value)) {
        Json::JsonArray arr;
        for (const auto& v : *vec_val) {
            arr.push_back(value_to_json(v));
        }
        return Json::JsonValue(arr);
    } else if (const auto* map_val = std::get_if<std::map<std::string, Value>>(&value)) {
        Json::JsonObject obj;
        for (const auto& [k, v] : *map_val) {
            obj[k] = value_to_json(v);
        }
        return Json::JsonValue(obj);
    } else if (const auto* element_vec_val = std::get_if<std::vector<Element>>(&value)) {
        Json::JsonArray arr;
        for (const auto& element : *element_vec_val) {
            Json::JsonObject obj;
            obj[element.key] = value_to_json(element.value);
            arr.push_back(obj);
        }
        return Json::JsonValue(arr);
    }
    // Fallback for other types not fully handled here
    throw std::runtime_error("Unsupported value type in value_to_json");
}


Value json_to_value(const Json::JsonValue& json_val); // Forward declaration

Document json_to_document(const Json::JsonObject& obj) {
    Document doc;
    for (const auto& pair : obj) {
        if (pair.first == "_id") continue;
        Element elem;
        elem.key = pair.first;
        elem.value = json_to_value(pair.second);
        doc.elements.push_back(elem);
    }
    return doc;
}

Value json_to_value(const Json::JsonValue& json_val) {
    if (json_val.is_null()) {
        return nullptr;
    } else if (json_val.is_string()) {
        return json_val.as_string();
    } else if (json_val.is_number()) {
        return json_val.as_number();
    } else if (json_val.is_bool()) {
        return json_val.as_bool();
    } else if (json_val.is_array()) {
        std::vector<Value> vec;
        for (const auto& v : json_val.as_array()) {
            vec.push_back(json_to_value(v));
        }
        return vec;
    } else if (json_val.is_object()) {
        std::map<std::string, Value> map;
        for (const auto& [k, v] : json_val.as_object()) {
            map[k] = json_to_value(v);
        }
        return map;
    }
    return nullptr;
}

struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};
} // anonymous namespace

class HttpServer::Impl {
public:
    Impl(Storage::DatabaseManager& manager, int port);
    ~Impl();
    void start();
    void stop();
private:
    void server_loop();
    void handle_client(int client_socket);
    void send_response(int sock, const std::string& code, const std::string& ctype, const std::string& body);

    Storage::DatabaseManager& db_manager_;
    int server_fd = -1;
    int server_port;
    std::atomic<bool> is_running{false};
    std::thread server_thread;
};

HttpServer::Impl::Impl(Storage::DatabaseManager& manager, int port) : db_manager_(manager), server_port(port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) throw std::runtime_error("WSAStartup failed");
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

    // Parse request line
    std::string request_line;
    std::getline(request_ss, request_line);
    if (!request_line.empty() && request_line.back() == '\r') {
        request_line.pop_back();
    }
    std::stringstream request_line_ss(request_line);
    request_line_ss >> req.method >> req.path;

    LOG_INFO("Incoming request: " + req.method + " " + req.path);

    // Parse headers
    std::string header_line;
    while (std::getline(request_ss, header_line) && !header_line.empty() && header_line != "\r") {
        auto colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = header_line.substr(0, colon_pos);
            std::string value = header_line.substr(colon_pos + 1);
            // Trim leading whitespace from value
            value.erase(0, value.find_first_not_of(" \t"));
            // Trim trailing carriage return
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            req.headers[key] = value;
        }
    }

    size_t body_start = request_str.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        req.body = request_str.substr(body_start + 4);
    }

    std::vector<std::string> path_parts;
    std::stringstream path_ss(req.path);
    std::string segment;
    while(std::getline(path_ss, segment, '/')) {
       if(!segment.empty()) path_parts.push_back(segment);
    }

    try {
        if (path_parts.empty() || (req.method == "GET" && path_parts.size() == 1 && path_parts[0] == "_health")) {
            send_response(client_socket, "200 OK", "text/plain", "OK");
            close(client_socket);
            return;
        }

        if (req.method == "GET" && path_parts.size() == 1 && path_parts[0] == "_databases") {
            Json::JsonArray db_array;
            for (const auto& name : db_manager_.list_databases()) {
                db_array.push_back(Json::JsonValue(name));
            }
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(db_array).serialize());
            close(client_socket);
            return;
        }

        if (req.method == "PUT" && path_parts.size() == 1) {
            db_manager_.create_database(path_parts[0]);
            send_response(client_socket, "201 Created", "text/plain", "Database '" + path_parts[0] + "' created.");
            close(client_socket);
            return;
        }

        if (req.method == "DELETE" && path_parts.size() == 1) {
            db_manager_.delete_database(path_parts[0]);
            send_response(client_socket, "204 No Content", "text/plain", "");
            close(client_socket);
            return;
        }

        std::string db_name = path_parts[0];
        auto& storage_engine = db_manager_.get_database(db_name);
        std::vector<std::string> sub_path_parts(path_parts.begin() + 1, path_parts.end());

        Transactions::TransactionID transaction_id = -1;
        if (req.headers.count("X-Transaction-ID")) {
            try {
                transaction_id = std::stoi(req.headers.at("X-Transaction-ID"));
            } catch (const std::exception& e) {
                LOG_WARNING("Could not parse X-Transaction-ID header: " + std::string(e.what()));
                // Invalid header, proceed without transaction context
                transaction_id = -1;
            }
        }

        if (sub_path_parts.empty()) {
            send_response(client_socket, "400 Bad Request", "text/plain", "Collection name missing from URL.");
        } else if (sub_path_parts[0] == "_begin" && req.method == "POST") {
            Transactions::TransactionID transaction_id = storage_engine.begin_transaction();
            Json::JsonObject response_obj;
            response_obj["transaction_id"] = Json::JsonValue(static_cast<double>(transaction_id));
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(response_obj).serialize());
        } else if (sub_path_parts[0] == "_commit" && req.method == "POST") {
            try {
                const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                if (!parsed_body.is_object() || !parsed_body.as_object().count("transaction_id")) {
                    send_response(client_socket, "400 Bad Request", "text/plain", "Missing transaction_id in request body.");
                    close(client_socket);
                    return;
                }
                Transactions::TransactionID transaction_id = static_cast<Transactions::TransactionID>(parsed_body.as_object().at("transaction_id").as_number());
                bool success = storage_engine.commit_transaction(transaction_id);
                Json::JsonObject response_obj;
                response_obj["success"] = Json::JsonValue(success);
                send_response(client_socket, "200 OK", "application/json", Json::JsonValue(response_obj).serialize());
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body.");
            }
        } else if (sub_path_parts[0] == "_rollback" && req.method == "POST") {
            try {
                const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                if (!parsed_body.is_object() || !parsed_body.as_object().count("transaction_id")) {
                    send_response(client_socket, "400 Bad Request", "text/plain", "Missing transaction_id in request body.");
                    close(client_socket);
                    return;
                }
                Transactions::TransactionID transaction_id = static_cast<Transactions::TransactionID>(parsed_body.as_object().at("transaction_id").as_number());
                bool success = storage_engine.rollback_transaction(transaction_id);
                Json::JsonObject response_obj;
                response_obj["success"] = Json::JsonValue(success);
                send_response(client_socket, "200 OK", "application/json", Json::JsonValue(response_obj).serialize());
            } catch (const std::exception& e) {
                send_response(client_socket, "400 Bad Request", "text/plain", "Invalid JSON body.");
            }
        } else if (sub_path_parts[0] == "_stats" && req.method == "GET") {
            Json::JsonObject stats_obj;
            stats_obj["total_docs"] = Json::JsonValue(static_cast<double>(storage_engine.scan("knowledge").size()));
            stats_obj["feedback_entries"] = Json::JsonValue(static_cast<double>(storage_engine.scan("knowledge_feedback").size()));
            stats_obj["total_accesses"] = Json::JsonValue(0.0); // Placeholder
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(stats_obj).serialize());
        } else if (sub_path_parts[0] == "_feedback" && req.method == "POST") {
            Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
            Document doc = json_to_document(parsed_body.as_object());
            std::string id = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            doc.id = id;
            storage_engine.put("knowledge_feedback", id, doc, transaction_id);
            send_response(client_socket, "201 Created", "text/plain", "Feedback created with ID: " + id);
        } else if (sub_path_parts[0] == "_collections" && req.method == "GET") {
            Json::JsonArray collections_array;
            for (const auto& name : storage_engine.list_collections()) {
                collections_array.push_back(Json::JsonValue(name));
            }
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(collections_array).serialize());
        } else {
            std::string collection_name = sub_path_parts[0];
            std::vector<std::string> doc_path_parts(sub_path_parts.begin() + 1, sub_path_parts.end());

            if (!doc_path_parts.empty() && req.method == "POST") {
                if (doc_path_parts[0] == "_query") {
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
                } else if (doc_path_parts[0] == "_index") {
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
                    send_response(client_socket, "200 OK", "text/plain", "Index creation initiated.");
                } else {
                    send_response(client_socket, "404 Not Found", "text/plain", "Endpoint not found.");
                }
            } else if (req.method == "POST" && doc_path_parts.empty()) {
                Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                Document doc = json_to_document(parsed_body.as_object());
                std::string id = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                doc.id = id;
                storage_engine.put(collection_name, id, doc, transaction_id);
                send_response(client_socket, "201 Created", "text/plain", "Document created with ID: " + id);
            } else if (req.method == "GET" && doc_path_parts.size() == 1) {
                 auto doc_opt = storage_engine.get(collection_name, doc_path_parts[0], transaction_id);
                 if (doc_opt && *doc_opt) {
                     // Optional has a value and the shared_ptr is not null
                     send_response(client_socket, "200 OK", "application/json", Json::JsonValue(document_to_json(**doc_opt)).serialize());
                 } else {
                     // Optional is empty OR contains a tombstone (nullptr)
                     send_response(client_socket, "404 Not Found", "text/plain", "Document not found.");
                 }
            } else if (req.method == "PUT" && doc_path_parts.size() == 1) {
                Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                Document doc = json_to_document(parsed_body.as_object());
                doc.id = doc_path_parts[0];
                storage_engine.put(collection_name, doc.id, doc, transaction_id);
                send_response(client_socket, "200 OK", "application/json", parsed_body.serialize());
            } else if (req.method == "DELETE" && doc_path_parts.size() == 1) {
                if (storage_engine.del(collection_name, doc_path_parts[0], transaction_id)) {
                    send_response(client_socket, "204 No Content", "text/plain", "");
                } else {
                    send_response(client_socket, "404 Not Found", "text/plain", "Document not found.");
                }
            } else if (req.method == "PUT" && doc_path_parts.empty()) {
                auto collections = storage_engine.list_collections();
                bool exists = false;
                for (const auto& c : collections) {
                    if (c == collection_name) {
                        exists = true;
                        break;
                    }
                }
                if (exists) {
                    send_response(client_socket, "200 OK", "text/plain", "Collection '" + collection_name + "' already exists.");
                } else {
                    storage_engine.create_collection(collection_name, TissDB::Schema());
                    send_response(client_socket, "201 Created", "text/plain", "Collection '" + collection_name + "' created.");
                }
            } else if (req.method == "DELETE" && doc_path_parts.empty()) {
                storage_engine.delete_collection(collection_name);
                send_response(client_socket, "204 No Content", "text/plain", "");
            } else {
                 send_response(client_socket, "404 Not Found", "text/plain", "Endpoint not found.");
            }
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Sending response: 500 Internal Server Error: " + std::string(e.what()));
        send_response(client_socket, "500 Internal Server Error", "text/plain", "Server error: " + std::string(e.what()));
    }

    close(client_socket);
}

HttpServer::HttpServer(Storage::DatabaseManager& db_manager, int port) : pimpl(std::make_unique<Impl>(db_manager, port)) {}
HttpServer::~HttpServer() = default;
void HttpServer::start() { pimpl->start(); }
void HttpServer::stop() { pimpl->stop(); }

} // namespace API
} // namespace TissDB
