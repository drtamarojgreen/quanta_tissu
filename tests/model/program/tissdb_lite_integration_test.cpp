#include "../../../quanta_tissu/tisslm/program/db/tissdb_client.h"
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

// Helper function to set a field in a document
void set_field(TissDB::Document& doc, const std::string& key, const std::string& value) {
    for (auto& element : doc.elements) {
        if (element.key == key) {
            element.value = value;
            return;
        }
    }
    doc.elements.push_back({key, value});
}

void run_tissdb_lite_integration_evaluation() {
    std::cout << "=== Running TissDB-Lite Integration Evaluation (C++) ===" << std::endl;

    // --- Setup TissDBClient ---
    TissDBClient client("127.0.0.1", 9876, "test_tissdb_lite_db", "static_test_token");

    std::cout << "  TissDBClient initialized." << std::endl;

    try {
        // Ensure DB setup
        client.ensure_db_setup({"myCollection"});
        std::cout << "  TissDB setup and collection created." << std::endl;

        // Insert items
        Document doc1;
        set_field(doc1, "name", "Test Item 1");
        set_field(doc1, "value", "10");
        std::string id1 = client.add_document("myCollection", doc1, "item1");
        std::cout << "  Inserted item 1 with ID: " << id1 << std::endl;

        Document doc2;
        set_field(doc2, "name", "Test Item 2");
        set_field(doc2, "value", "20");
        std::string id2 = client.add_document("myCollection", doc2, "item2");
        std::cout << "  Inserted item 2 with ID: " << id2 << std::endl;

        // Get item by ID
        Document retrieved_doc1 = client.get_document("myCollection", "item1");
        if (get_field(retrieved_doc1, "name") == "Test Item 1") {
            std::cout << "  Retrieved item 1 successfully." << std::endl;
        } else {
            throw std::runtime_error("Failed to retrieve item 1.");
        }

        // Search for items (using the new search_documents method)
        // This assumes TissDB's search endpoint can handle simple queries like this
        std::string query_json = "{\"query\": \"value > 15\"}";
        std::vector<Document> found_items = client.search_documents("myCollection", query_json);
        std::cout << "  Found " << found_items.size() << " items with value > 15." << std::endl;
        if (found_items.size() == 1 && get_field(found_items[0], "name") == "Test Item 2") {
            std::cout << "  Search for value > 15 Passed." << std::endl;
        } else {
            throw std::runtime_error("Search for value > 15 failed.");
        }

        // Update item
        set_field(doc1, "value", "15");
        client.add_document("myCollection", doc1, "item1"); // add_document also acts as update if ID exists
        Document updated_doc1 = client.get_document("myCollection", "item1");
        if (get_field(updated_doc1, "value") == "15") {
            std::cout << "  Updated item 1 successfully." << std::endl;
        } else {
            throw std::runtime_error("Failed to update item 1.");
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
