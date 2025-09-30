#include "tissdb_client.h"
#include <iostream>
#include <iomanip>

using namespace TissDB;

void test_connection() {
    std::cout << "=== Testing TissDB Connection ===" << std::endl;
    
    try {
        // Connect to TissDB on localhost:9876
        TissDBClient client("127.0.0.1", 9876, "testdb_cpp");
        
        std::cout << "Testing connection..." << std::endl;
        if (client.test_connection()) {
            std::cout << "✓ Connection successful" << std::endl;
        } else {
            std::cout << "✗ Connection failed" << std::endl;
            return;
        }
        
        // Setup database and collections
        std::cout << "\nSetting up database and collections..." << std::endl;
        std::vector<std::string> collections = {"documents", "embeddings", "feedback"};
        
        if (client.ensure_db_setup(collections)) {
            std::cout << "✓ Database setup successful" << std::endl;
        }
        
        // Add a test document
        std::cout << "\nAdding test document..." << std::endl;
        Document doc;
        doc.set_field("title", "Test Document");
        doc.set_field("content", "This is a test document from C++");
        doc.set_field("type", "test");
        
        std::string doc_id = client.add_document("documents", doc);
        std::cout << "✓ Document added with ID: " << doc_id << std::endl;
        
        // Retrieve the document
        std::cout << "\nRetrieving document..." << std::endl;
        Document retrieved = client.get_document("documents", doc_id);
        std::cout << "✓ Document retrieved:" << std::endl;
        std::cout << "  Title: " << retrieved.get_field("title") << std::endl;
        std::cout << "  Content: " << retrieved.get_field("content") << std::endl;
        std::cout << "  Type: " << retrieved.get_field("type") << std::endl;
        
        // Add feedback
        std::cout << "\nAdding feedback..." << std::endl;
        Document feedback;
        feedback.set_field("rating", "5");
        feedback.set_field("comment", "Great system!");
        feedback.set_field("user", "cpp_tester");
        
        std::string feedback_id = client.add_feedback(feedback);
        std::cout << "✓ Feedback added with ID: " << feedback_id << std::endl;
        
        // Get stats
        std::cout << "\nGetting database stats..." << std::endl;
        auto stats = client.get_stats();
        std::cout << "✓ Stats retrieved:" << std::endl;
        for (const auto& pair : stats) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
        
        std::cout << "\n=== All tests passed! ===" << std::endl;
        
    } catch (const DatabaseException& e) {
        std::cerr << "✗ Database error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "TissDB C++ Client Test" << std::endl;
    std::cout << "======================" << std::endl << std::endl;
    
    test_connection();
    
    return 0;
}
