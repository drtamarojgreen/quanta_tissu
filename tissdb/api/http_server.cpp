#include "http_server.h"
#include "../common/log.h"
#include "../common/schema.h"
#include "../storage/database_manager.h"
#include "../json/json.h"
#include "../common/document.h"
#include "../query/parser.h"
#include "../query/ast.h"
#include "../query/executor.h"
#include "../auth/token_manager.h"
#include "../auth/rbac.h"
#include "../audit/audit_logger.h"
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
#include <algorithm>
#include <cctype>
#include <iomanip> // for std::put_time

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

void to_lower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
}

Json::JsonValue value_to_json(const Value& value); // Forward declaration

Json::JsonObject document_to_json(const Document& doc) {
    Json::JsonObject obj;
    obj["_id"] = Json::JsonValue(doc.id);
    for (const auto& elem : doc.elements) {
        obj[elem.key] = value_to_json(elem.value);
    }
    return obj;
}

Json::JsonValue value_to_json(const Value& value) {
    if (std::holds_alternative<std::nullptr_t>(value)) {
        return Json::JsonValue(nullptr);
    } else if (const auto* str_val = std::get_if<std::string>(&value)) {
        return Json::JsonValue(*str_val);
    } else if (const auto* num_val = std::get_if<double>(&value)) {
        return Json::JsonValue(*num_val);
    } else if (const auto* bool_val = std::get_if<bool>(&value)) {
        return Json::JsonValue(*bool_val);
    } else if (const auto* date_val = std::get_if<Date>(&value)) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << date_val->year << "-"
           << std::setw(2) << static_cast<int>(date_val->month) << "-"
           << std::setw(2) << static_cast<int>(date_val->day);
        return Json::JsonValue(ss.str());
    } else if (const auto* time_val = std::get_if<Time>(&value)) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << static_cast<int>(time_val->hour) << ":"
           << std::setw(2) << static_cast<int>(time_val->minute) << ":"
           << std::setw(2) << static_cast<int>(time_val->second);
        return Json::JsonValue(ss.str());
    } else if (const auto* dt_val = std::get_if<DateTime>(&value)) {
        std::time_t time = std::chrono::system_clock::to_time_t(*dt_val);
        std::tm tm = *std::gmtime(&time);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return Json::JsonValue(ss.str());
    } else if (const auto* arr_ptr = std::get_if<std::shared_ptr<Array>>(&value)) {
        Json::JsonArray arr;
        if (arr_ptr && *arr_ptr) {
            for (const auto& v : (*arr_ptr)->values) {
                arr.push_back(value_to_json(v));
            }
        }
        return Json::JsonValue(arr);
    } else if (const auto* obj_ptr = std::get_if<std::shared_ptr<Object>>(&value)) {
        Json::JsonObject obj;
        if (obj_ptr && *obj_ptr) {
            for (const auto& [k, v] : (*obj_ptr)->values) {
                obj[k] = value_to_json(v);
            }
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
        if (pair.first == "_id") {
            doc.id = pair.second.as_string();
            continue;
        }
        Element elem;
        elem.key = pair.first;
        elem.value = json_to_value(pair.second);
        doc.elements.push_back(std::move(elem));
    }
    return doc;
}

Value json_to_value(const Json::JsonValue& json_val) {
    if (json_val.is_null()) {
        return nullptr;
    } else if (json_val.is_string()) {
        // This is where we could try to auto-detect date/time formats from strings
        // when inserting JSON, but for now we'll just treat them as strings.
        // The TissQL parser is the primary way to insert typed date/time values.
        return json_val.as_string();
    } else if (json_val.is_number()) {
        return json_val.as_number();
    } else if (json_val.is_bool()) {
        return json_val.as_bool();
    } else if (json_val.is_array()) {
        auto arr = std::make_shared<Array>();
        for (const auto& v : json_val.as_array()) {
            arr->values.push_back(json_to_value(v));
        }
        return arr;
    } else if (json_val.is_object()) {
        auto obj = std::make_shared<Object>();
        for (const auto& [k, v] : json_val.as_object()) {
            obj->values[k] = json_to_value(v);
        }
        return obj;
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

    Auth::TokenManager token_manager_;
    Auth::RBACManager rbac_manager_;
    Audit::AuditLogger audit_logger_;
    Storage::DatabaseManager& db_manager_;
    int server_fd = -1;
    int server_port;
    std::atomic<bool> is_running{false};
    std::thread server_thread;
};

HttpServer::Impl::Impl(Storage::DatabaseManager& manager, int port)
    : db_manager_(manager),
      server_port(port),
      audit_logger_("tissdb_audit.log") {
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
    // Note: In a real server, we'd get the client's IP address from the socket.
    // This is a placeholder.
    std::string source_ip = "127.0.0.1";

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

    audit_logger_.log({std::chrono::system_clock::now(), "", source_ip,
        Audit::EventType::RequestBegin, req.method + " " + req.path, true, "Request received."});

    // Parse headers
    std::string header_line;
    while (std::getline(request_ss, header_line) && !header_line.empty() && header_line != "\r") {
        auto colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = header_line.substr(0, colon_pos);
            to_lower(key); // Make header key case-insensitive
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

    // --- Authentication Check ---
    if (req.path != "/_health") {
        if (req.headers.find("authorization") == req.headers.end()) {
            audit_logger_.log({std::chrono::system_clock::now(), "", source_ip, Audit::EventType::AuthFailure,
                req.method + " " + req.path, false, "Authorization header missing."});
            send_response(client_socket, "401 Unauthorized", "text/plain", "Authorization header missing.");
            close(client_socket);
            return;
        }

        std::string auth_header = req.headers.at("authorization");
        std::string token;
        std::stringstream ss(auth_header);
        std::string bearer;
        ss >> bearer >> token;

        if (bearer != "Bearer" || !token_manager_.validate_token(token)) {
            audit_logger_.log({std::chrono::system_clock::now(), token, source_ip, Audit::EventType::AuthFailure,
                req.method + " " + req.path, false, "Invalid or missing bearer token."});
            send_response(client_socket, "401 Unauthorized", "text/plain", "Invalid or missing bearer token.");
            close(client_socket);
            return;
        }
        audit_logger_.log({std::chrono::system_clock::now(), token, source_ip, Audit::EventType::AuthSuccess,
            req.method + " " + req.path, true, "Authentication successful."});
    }
    // --- End Authentication Check ---

    // --- RBAC Check ---
    // TODO: The role should be retrieved from the token's metadata.
    // For now, we hardcode the role based on the static token for demonstration.
    std::string token_val;
    if (req.headers.count("authorization")) {
        std::stringstream ss(req.headers.at("authorization"));
        std::string bearer;
        ss >> bearer >> token_val;
    }
    Auth::Role user_role;
    if (token_val == "static_test_token") {
        user_role = Auth::Role::Admin;
    } else if (token_val == "read_only_token") {
        user_role = Auth::Role::ReadOnly;
    } else {
        user_role = Auth::Role::NoAccess;
    }

    std::vector<std::string> path_parts_for_rbac;
    std::stringstream path_ss_for_rbac(req.path);
    std::string segment_for_rbac;
    while(std::getline(path_ss_for_rbac, segment_for_rbac, '/')) {
        if(!segment_for_rbac.empty()) path_parts_for_rbac.push_back(segment_for_rbac);
    }

    // Example of checking a specific permission for a critical action
    if (req.method == "DELETE" && path_parts_for_rbac.size() == 1) { // e.g., DELETE /my_database
        if (!rbac_manager_.has_permission(user_role, Auth::Permission::DbDelete)) {
            audit_logger_.log({std::chrono::system_clock::now(), token_val, source_ip, Audit::EventType::PermissionCheckFailure,
                req.method + " " + req.path, false, "User does not have DbDelete permission."});
            send_response(client_socket, "403 Forbidden", "text/plain", "You do not have permission to delete a database.");
            close(client_socket);
            return;
        }
    }
    // --- End RBAC Check ---

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
            audit_logger_.log({std::chrono::system_clock::now(), token_val, source_ip, Audit::EventType::DbDelete,
                req.path, true, "Database deleted successfully."});
            send_response(client_socket, "204 No Content", "text/plain", "");
            close(client_socket);
            return;
        }

        std::string db_name = path_parts[0];
        auto& storage_engine = db_manager_.get_database(db_name);
        std::vector<std::string> sub_path_parts(path_parts.begin() + 1, path_parts.end());

        Transactions::TransactionID transaction_id = -1;
        if (req.headers.count("x-transaction-id")) {
            try {
                transaction_id = std::stoi(req.headers.at("x-transaction-id"));
            } catch (const std::exception& e) {
                LOG_WARNING("Could not parse X-Transaction-ID header: " + std::string(e.what()));
                // Invalid header, proceed without transaction context
                transaction_id = -1;
            }
        }

        if (sub_path_parts.empty()) {
            send_response(client_socket, "400 Bad Request", "text/plain", "Collection name missing from URL.");
        } else if (sub_path_parts[0] == "_begin" && req.method == "POST") {
            Transactions::TransactionID new_transaction_id = storage_engine.begin_transaction();
            Json::JsonObject response_obj;
            response_obj["transaction_id"] = Json::JsonValue(static_cast<double>(new_transaction_id));
            send_response(client_socket, "200 OK", "application/json", Json::JsonValue(response_obj).serialize());
        } else if (sub_path_parts[0] == "_commit" && req.method == "POST") {
            try {
                const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
                if (!parsed_body.is_object() || !parsed_body.as_object().count("transaction_id")) {
                    send_response(client_socket, "400 Bad Request", "text/plain", "Missing transaction_id in request body.");
                    close(client_socket);
                    return;
                }
                Transactions::TransactionID tid_to_commit = static_cast<Transactions::TransactionID>(parsed_body.as_object().at("transaction_id").as_number());
                bool success = storage_engine.commit_transaction(tid_to_commit);
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
                Transactions::TransactionID tid_to_rollback = static_cast<Transactions::TransactionID>(parsed_body.as_object().at("transaction_id").as_number());
                bool success = storage_engine.rollback_transaction(tid_to_rollback);
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
        } else if (sub_path_parts[0] == "_query" && req.method == "POST") {
             const Json::JsonValue parsed_body = Json::JsonValue::parse(req.body);
             std::string query_str = parsed_body.as_object().at("query").as_string();
             Query::Parser parser;
             Query::AST ast = parser.parse(query_str);
             Query::Executor executor(storage_engine);
             auto result_docs = executor.execute(ast, {});
             Json::JsonArray result_array;
             for (const auto& doc : result_docs) {
                 result_array.push_back(Json::JsonValue(document_to_json(doc)));
             }
             send_response(client_socket, "200 OK", "application/json", Json::JsonValue(result_array).serialize());
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
                if (doc_path_parts[0] == "_index") {
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
                Json::JsonObject response_obj;
                response_obj["id"] = Json::JsonValue(id);
                send_response(client_socket, "201 Created", "application/json", Json::JsonValue(response_obj).serialize());
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
                try {
                    storage_engine.delete_collection(collection_name);
                    send_response(client_socket, "204 No Content", "text/plain", "");
                } catch (const std::runtime_error& e) {
                    // This is likely "Collection not found"
                    send_response(client_socket, "404 Not Found", "text/plain", e.what());
                }
            } else if (req.method == "GET" && path_parts.size() >= 2 && path_parts[0] == "_admin" && path_parts[1] == "audit_log") {
                if (!rbac_manager_.has_permission(user_role, Auth::Permission::AdminRead)) {
                    audit_logger_.log({std::chrono::system_clock::now(), token_val, source_ip, Audit::EventType::PermissionCheckFailure,
                        req.path, false, "User does not have AdminRead permission."});
                    send_response(client_socket, "403 Forbidden", "text/plain", "You do not have permission to access the audit log.");
                } else {
                    // In a real implementation, this would return a JSON array of log entries.
                    // The get_logs function is a placeholder and will throw an exception.
                    try {
                        // TODO: Add time range filtering from query params
                        auto logs = audit_logger_.get_logs(std::chrono::system_clock::now() - std::chrono::hours(24),
                                                         std::chrono::system_clock::now());
                        // This part will not be reached due to the placeholder exception.
                        send_response(client_socket, "200 OK", "application/json", "[]");
                    } catch (const std::logic_error& e) {
                        send_response(client_socket, "501 Not Implemented", "text/plain", e.what());
                    }
                }
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
