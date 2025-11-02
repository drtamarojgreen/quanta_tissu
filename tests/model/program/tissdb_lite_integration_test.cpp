#include "../../../quanta_tissu/tisslm/program/db/tissdb_lite_client.h"
#include "../../../tissdb/json/json.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <variant>

using namespace TissDB;

// Helper function to get a field from a document
std::string get_field(const TissDB::Document& doc, const std::string& key) {
    for (const auto& element : doc.elements) {
        if (element.key == key) {
            if (std::holds_alternative<std::string>(element.value)) {
                return std::get<std::string>(element.value);
            }
        }
    }
    return "";
}

void run_tissdb_lite_integration_evaluation() {
    std::cout << "=== Running TissDB-Lite Integration Evaluation (C++) ===" << std::endl;

    // --- Setup TissDBLiteClient ---
    TissDBLiteClient client("127.0.0.1", 9877);

    std::cout << "  TissDBLiteClient initialized." << std::endl;

    try {
        // Clear the database
        client.sendCommand("{\"action\": \"deleteDb\"}");

        // Insert items
        client.sendCommand("{\"action\": \"insert\", \"collectionName\": \"myCollection\", \"item\": {\"name\": \"Test Item 1\", \"value\": 10}}");
        client.sendCommand("{\"action\": \"insert\", \"collectionName\": \"myCollection\", \"item\": {\"name\": \"Test Item 2\", \"value\": 20}}");

        // Search for items
        std::string response_json = client.sendCommand("{\"action\": \"find\", \"collectionName\": \"myCollection\", \"condition_string\": \"value > 15\"}");
        Json::JsonValue json = Json::JsonValue::parse(response_json);
        Json::JsonArray found_items = json.as_object().at("data").as_array();

        std::cout << "  Found " << found_items.size() << " items with value > 15." << std::endl;
        if (found_items.size() == 1 && found_items[0].as_object().at("name").as_string() == "Test Item 2") {
            std::cout << "  Search for value > 15 Passed." << std::endl;
        } else {
            throw std::runtime_error("Search for value > 15 failed.");
        }

        std::cout << "  TissDB-Lite Integration tests completed successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "  [ERROR] TissDB-Lite Integration test failed: " << e.what() << std::endl;
        throw;
    }

    std::cout << "\n=== TissDB-Lite Integration Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_tissdb_lite_integration_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "TissDB-Lite Integration Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
