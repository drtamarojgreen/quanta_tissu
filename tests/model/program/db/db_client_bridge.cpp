#include "../../../../quanta_tissu/tisslm/program/db/tissdb_client.h"
#include "../../../../quanta_tissu/tisslm/program/db/tissdb_lite_client.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace TissDB;

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <type:tissdb|tissdblite> <host> <port> <command> [args...]" << std::endl;
        return 1;
    }

    std::string type = argv[1];
    std::string host = argv[2];
    int port = std::stoi(argv[3]);
    std::string command = argv[4];

    try {
        if (type == "tissdb") {
            // Default DB name for the bridge, can be overridden if needed
            std::string db_name = "testdb";
            if (argc > 5 && command == "set_db") {
                db_name = argv[5];
                // For set_db we just return success
                std::cout << "DB set to " << db_name << std::endl;
                return 0;
            }

            // In a real bridge, we might want to persist the client or db_name.
            // For this CLI bridge, we'll take db_name as an optional 5th arg or use default.
            TissDBClient client(host, port, db_name, "static_test_token");

            if (command == "create_database") {
                client.create_database();
                std::cout << "success" << std::endl;
            } else if (command == "delete_database") {
                client.delete_database();
                std::cout << "success" << std::endl;
            } else if (command == "ensure_db_setup") {
                std::vector<std::string> cols;
                for (int i = 5; i < argc; ++i) cols.push_back(argv[i]);
                client.ensure_db_setup(cols);
                std::cout << "success" << std::endl;
            } else if (command == "add_document") {
                std::string col = argv[5];
                std::string title = argv[6];
                std::string doc_id = (argc > 7) ? argv[7] : "";
                Document doc;
                doc.elements.push_back({"title", title});
                std::string id = client.add_document(col, doc, doc_id);
                std::cout << id << std::endl;
            } else if (command == "get_document") {
                std::string col = argv[5];
                std::string doc_id = argv[6];
                Document doc = client.get_document(col, doc_id);
                // Simple output for the bridge to verify
                for (const auto& elem : doc.elements) {
                    if (elem.key == "title") {
                        std::cout << "title:" << std::get<std::string>(elem.value) << std::endl;
                    }
                }
            } else if (command == "query") {
                std::string col = argv[5];
                std::string q = argv[6];
                std::cout << client.query(col, q) << std::endl;
            } else if (command == "add_feedback") {
                double rating = std::stod(argv[5]);
                std::string comment = argv[6];
                Document f;
                f.elements.push_back({"rating", rating});
                f.elements.push_back({"comment", comment});
                std::cout << client.add_feedback(f) << std::endl;
            } else if (command == "test_connection") {
                std::cout << (client.test_connection() ? "success" : "failed") << std::endl;
            } else {
                std::cerr << "Unknown tissdb command: " << command << std::endl;
                return 1;
            }
        } else if (type == "tissdblite") {
            TissDBLiteClient client(host, port);
            if (command == "sendCommand") {
                std::cout << client.sendCommand(argv[5]) << std::endl;
            } else {
                std::cerr << "Unknown tissdblite command: " << command << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Unknown type: " << type << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
