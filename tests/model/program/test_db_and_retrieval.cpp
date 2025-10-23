#include "../../../quanta_tissu/tisslm/program/db/tissdb_client.h"
#include "../../../quanta_tissu/tisslm/program/retrieval/retrieval_strategy.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace TissDB;
using namespace Retrieval;

// Test result tracking
struct TestResults {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;
    
    void record_pass(const std::string& test_name) {
        passed++;
        std::cout << "  ✓ " << test_name << std::endl;
    }
    
    void record_fail(const std::string& test_name, const std::string& reason) {
        failed++;
        failures.push_back(test_name + ": " + reason);
        std::cout << "  ✗ " << test_name << " - " << reason << std::endl;
    }
    
    void print_summary() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Test Summary" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        
        if (failed > 0) {
            std::cout << "\nFailed Tests:" << std::endl;
            for (const auto& failure : failures) {
                std::cout << "  - " << failure << std::endl;
            }
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

TestResults results;

// Helper function to compare floats
bool float_equals(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

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


// ============================================================================ 
// Database Tests
// ============================================================================ 

void test_db_connection() {
    std::cout << "\n=== Testing Database Connection ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        
        if (client.test_connection()) {
            results.record_pass("Database connection");
        } else {
            results.record_fail("Database connection", "Connection failed");
        }
    } catch (const std::exception& e) {
        results.record_fail("Database connection", e.what());
    }
}

void test_db_setup() {
    std::cout << "\n=== Testing Database Setup ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::vector<std::string> collections = {"test_docs", "test_embeddings", "test_feedback"};
        
        if (client.ensure_db_setup(collections)) {
            results.record_pass("Database and collection creation");
        } else {
            results.record_fail("Database and collection creation", "Setup failed");
        }
    } catch (const std::exception& e) {
        results.record_fail("Database and collection creation", e.what());
    }
}

void test_document_crud() {
    std::cout << "\n=== Testing Document CRUD Operations ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::string collection = "test_docs";
        client.ensure_db_setup({collection});
        
        // Clean up collection before test to ensure isolation
        try {
            client.query(collection, "DELETE FROM " + collection + ";");
        } catch (const std::exception& e) {
            // Log this but don't fail the test itself, as the main test might still pass
            std::cerr << "  (Note: Initial cleanup query failed, proceeding with test...)" << std::endl;
        }

        // Create
        std::string insert_query = "INSERT INTO " + collection + " (_id, title, content, author) VALUES ('doc1', 'Test Document', 'This is test content', 'Unit Test');";
        try {
            client.query(collection, insert_query);
            results.record_pass("Document creation");
        } catch (const std::exception& e) {
            results.record_fail("Document creation", e.what());
            return;
        }
        
        // Read
        std::string select_query = "SELECT title, content, author FROM " + collection + " WHERE _id = 'doc1';";
        try {
            std::string response = client.query(collection, select_query);
            // NOTE: The C++ JSON parser is not available here.
            // We will do a simple string search for now.
            if (response.find("Test Document") != std::string::npos &&
                response.find("This is test content") != std::string::npos &&
                response.find("Unit Test") != std::string::npos) {
                results.record_pass("Document retrieval");
            } else {
                results.record_fail("Document retrieval", "Retrieved data doesn't match");
            }
        } catch (const std::exception& e) {
            results.record_fail("Document retrieval", e.what());
        }

        // Update
        std::string update_query = "UPDATE " + collection + " SET content = 'This is updated content' WHERE _id = 'doc1';";
        try {
            client.query(collection, update_query);
            results.record_pass("Document update");
        } catch (const std::exception& e) {
            results.record_fail("Document update", e.what());
            return;
        }

        // Verify Update
        std::string verify_update_query = "SELECT content FROM " + collection + " WHERE _id = 'doc1';";
        try {
            std::string response = client.query(collection, verify_update_query);
            if (response.find("This is updated content") != std::string::npos) {
                results.record_pass("Document update verification");
            } else {
                results.record_fail("Document update verification", "Updated data not found");
            }
        } catch (const std::exception& e) {
            results.record_fail("Document update verification", e.what());
        }

        // Delete
        std::string delete_query = "DELETE FROM " + collection + " WHERE _id = 'doc1';";
        try {
            client.query(collection, delete_query);
            results.record_pass("Document deletion");
        } catch (const std::exception& e) {
            results.record_fail("Document deletion", e.what());
            return;
        }

        // Verify Delete
        std::string verify_delete_query = "SELECT _id FROM " + collection + " WHERE _id = 'doc1';";
        try {
            std::string response = client.query(collection, verify_delete_query);
            if (response == "[]") {
                results.record_pass("Document deletion verification");
            } else {
                results.record_fail("Document deletion verification", "Document not deleted");
            }
        } catch (const std::exception& e) {
            results.record_fail("Document deletion verification", e.what());
        }

    } catch (const std::exception& e) {
        results.record_fail("Document CRUD", e.what());
    }
}

void test_feedback_collection() {
    std::cout << "\n=== Testing Feedback Collection ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::string collection = "feedback";
        client.ensure_db_setup({collection});
        
        std::string insert_query = "INSERT INTO " + collection + " (rating, comment, user, feature) VALUES (5, 'Excellent system!', 'test_user', 'retrieval');";
        try {
            client.query(collection, insert_query);
            results.record_pass("Feedback collection");
        } catch (const std::exception& e) {
            results.record_fail("Feedback collection", e.what());
        }
    } catch (const std::exception& e) {
        results.record_fail("Feedback collection", e.what());
    }
}

void test_multiple_documents() {
    std::cout << "\n=== Testing Multiple Document Operations ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::string collection = "test_docs";
        client.ensure_db_setup({collection});
        
        // Clean up collection before test to ensure isolation
        try {
            client.query(collection, "DELETE FROM " + collection + ";");
        } catch (const std::exception& e) {
            results.record_fail("Multiple document retrieval", "Cleanup failed: " + std::string(e.what()));
            return;
        }

        // Add multiple documents
        int created_count = 0;
        for (int i = 0; i < 5; ++i) {
            std::string title = "Document " + std::to_string(i);
            std::string content = "Content for document " + std::to_string(i);
            std::string doc_id = "doc" + std::to_string(i);
            std::string insert_query = "INSERT INTO " + collection + " (_id, title, content, idx) VALUES ('" + doc_id + "', '" + title + "', '" + content + "', " + std::to_string(i) + ");";
            try {
                client.query(collection, insert_query);
                created_count++;
            } catch (const std::exception& e) {
                // fail silently
            }
        }
        
        if (created_count == 5) {
            results.record_pass("Multiple document creation");
        } else {
            results.record_fail("Multiple document creation", "Not all documents created");
        }
        
        // Clean up collection before test to ensure isolation
        try {
            client.query(collection, "DELETE FROM " + collection + ";");
        } catch (const std::exception& e) {
            results.record_fail("Multiple document retrieval", "Cleanup failed: " + std::string(e.what()));
            return;
        }
        // Verify all documents
        std::string select_query = "SELECT COUNT(*) FROM " + collection + ";";
        try {
            std::string response = client.query(collection, select_query);
            if (response.find("\"COUNT(*)\":5") != std::string::npos) {
                results.record_pass("Multiple document retrieval");
            } else {
                results.record_fail("Multiple document retrieval", "Verification query failed. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("Multiple document retrieval", e.what());
        }
        
    } catch (const std::exception& e) {
        results.record_fail("Multiple documents", e.what());
    }
}

void test_document_search() {
    std::cout << "\n=== Testing Document Search ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::string collection = "search_docs";
        client.ensure_db_setup({collection});
        
        // Add sample documents
        client.query(collection, "INSERT INTO " + collection + " (_id, title, content) VALUES ('doc_mars', 'Mars Mission Overview', 'The first manned mission to Mars, named ''Ares 1'', is scheduled for 2035.');");
        client.query(collection, "INSERT INTO " + collection + " (_id, title, content) VALUES ('doc_moon', 'Moon Landing History', 'The Apollo 11 mission landed humans on the Moon in 1969.');");
        client.query(collection, "INSERT INTO " + collection + " (_id, title, content) VALUES ('doc_jupiter', 'Future Space Exploration', 'Plans for future space exploration include missions to Jupiter''s moons.');");
        
        // Search for documents containing "Mars"
        std::string mars_query = "SELECT title FROM " + collection + " WHERE content LIKE '%Mars%';";
        try {
            std::string response = client.query(collection, mars_query);
            if (response.find("Mars Mission Overview") != std::string::npos) {
                results.record_pass("Document search for 'Mars'");
            } else {
                results.record_fail("Document search for 'Mars'", "Unexpected search results. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("Document search for 'Mars'", e.what());
        }

        // Search for documents containing "Moon"
        std::string moon_query = "SELECT title FROM " + collection + " WHERE content LIKE '%Moon%';";
        try {
            std::string response = client.query(collection, moon_query);
            if (response.find("Moon Landing History") != std::string::npos) {
                results.record_pass("Document search for 'Moon'");
            } else {
                results.record_fail("Document search for 'Moon'", "Unexpected search results for 'Moon'. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("Document search for 'Moon'", e.what());
        }

        // Search for documents containing "exploration" (should find one)
        std::string exploration_query = "SELECT COUNT(*) FROM " + collection + " WHERE content LIKE '%exploration%';";
        try {
            std::string response = client.query(collection, exploration_query);
            if (response.find("\"COUNT(*)\":1") != std::string::npos) {
                results.record_pass("Document search for 'exploration'");
            } else {
                results.record_fail("Document search for 'exploration'", "Expected 1 document, got different count. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("Document search for 'exploration'", e.what());
        }
        
    } catch (const std::exception& e) {
        results.record_fail("Document search", e.what());
    }
}

// ============================================================================ 
// Retrieval Strategy Tests
// ============================================================================ 

void test_cosine_similarity() {
    std::cout << "\n=== Testing Cosine Similarity Strategy ===" << std::endl;
    
    try {
        CosineSimilarityStrategy strategy;
        
        // Test with identical vectors
        std::vector<float> query = {1.0f, 0.0f, 0.0f};
        std::vector<std::vector<float>> docs = {
            {1.0f, 0.0f, 0.0f},  // Should be 1.0
            {0.0f, 1.0f, 0.0f},  // Should be 0.0
            {0.5f, 0.5f, 0.0f}   // Should be ~0.707
        };
        
        auto similarities = strategy.calculate_similarity(query, docs);
        
        if (similarities.size() == 3 &&
            float_equals(similarities[0], 1.0f) &&
            float_equals(similarities[1], 0.0f) &&
            float_equals(similarities[2], 0.707f, 0.01f)) {
            results.record_pass("Cosine similarity calculation");
        } else {
            results.record_fail("Cosine similarity calculation", 
                              "Unexpected similarity values");
        }
    } catch (const std::exception& e) {
        results.record_fail("Cosine similarity", e.what());
    }
}

void test_euclidean_distance() {
    std::cout << "\n=== Testing Euclidean Distance Strategy ===" << std::endl;
    
    try {
        EuclideanDistanceStrategy strategy;
        
        std::vector<float> query = {0.0f, 0.0f, 0.0f};
        std::vector<std::vector<float>> docs = {
            {0.0f, 0.0f, 0.0f},  // Distance 0, similarity 1.0
            {1.0f, 0.0f, 0.0f},  // Distance 1, similarity 0.5
            {3.0f, 4.0f, 0.0f}   // Distance 5, similarity 1/6
        };
        
        auto similarities = strategy.calculate_similarity(query, docs);
        
        if (similarities.size() == 3 &&
            float_equals(similarities[0], 1.0f) &&
            float_equals(similarities[1], 0.5f) &&
            similarities[2] < similarities[1]) {
            results.record_pass("Euclidean distance calculation");
        } else {
            results.record_fail("Euclidean distance calculation", 
                              "Unexpected similarity values");
        }
    } catch (const std::exception& e) {
        results.record_fail("Euclidean distance", e.what());
    }
}

void test_dot_product() {
    std::cout << "\n=== Testing Dot Product Strategy ===" << std::endl;
    
    try {
        DotProductStrategy strategy;
        
        std::vector<float> query = {1.0f, 2.0f, 3.0f};
        std::vector<std::vector<float>> docs = {
            {1.0f, 0.0f, 0.0f},  // Dot product = 1
            {0.0f, 1.0f, 0.0f},  // Dot product = 2
            {1.0f, 1.0f, 1.0f}   // Dot product = 6
        };
        
        auto similarities = strategy.calculate_similarity(query, docs);
        
        if (similarities.size() == 3 &&
            float_equals(similarities[0], 1.0f) &&
            float_equals(similarities[1], 2.0f) &&
            float_equals(similarities[2], 6.0f)) {
            results.record_pass("Dot product calculation");
        } else {
            results.record_fail("Dot product calculation", 
                              "Unexpected similarity values");
        }
    } catch (const std::exception& e) {
        results.record_fail("Dot product", e.what());
    }
}

void test_bm25() {
    std::cout << "\n=== Testing BM25 Strategy ===" << std::endl;
    
    try {
        std::vector<std::string> corpus = {
            "the quick brown fox",
            "the lazy dog",
            "quick brown dogs"
        };
        
        BM25RetrievalStrategy strategy(corpus);
        
        std::vector<float> dummy_embedding;
        std::vector<std::vector<float>> dummy_docs;
        std::map<std::string, std::string> kwargs;
        kwargs["query_text"] = "quick fox";
        
        auto scores = strategy.calculate_similarity(dummy_embedding, dummy_docs, kwargs);
        
        if (scores.size() == 3 && scores[0] > scores[1]) {
            results.record_pass("BM25 calculation");
        } else {
            results.record_fail("BM25 calculation", "Unexpected scores");
        }
    } catch (const std::exception& e) {
        results.record_fail("BM25", e.what());
    }
}

void test_hybrid_strategy() {
    std::cout << "\n=== Testing Hybrid Strategy ===" << std::endl;
    
    try {
        HybridStrategy hybrid;
        
        auto cosine = std::make_shared<CosineSimilarityStrategy>();
        auto euclidean = std::make_shared<EuclideanDistanceStrategy>();
        
        hybrid.add_strategy(cosine, 0.7f);
        hybrid.add_strategy(euclidean, 0.3f);
        
        std::vector<float> query = {1.0f, 0.0f, 0.0f};
        std::vector<std::vector<float>> docs = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f}
        };
        
        auto scores = hybrid.calculate_similarity(query, docs);
        
        if (scores.size() == 2 && scores[0] > scores[1]) {
            results.record_pass("Hybrid strategy combination");
        } else {
            results.record_fail("Hybrid strategy combination", "Unexpected scores");
        }
    } catch (const std::exception& e) {
        results.record_fail("Hybrid strategy", e.what());
    }
}

// ============================================================================ 
// Integration Tests
// ============================================================================ 

void test_db_with_embeddings() {
    std::cout << "\n=== Testing Database with Embeddings ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::string collection = "embeddings";
        client.ensure_db_setup({collection});

        // Clean up collection before test to ensure isolation
        try {
            client.query(collection, "DELETE FROM " + collection + ";");
        } catch (const std::exception& e) {
            results.record_fail("Verification of documents with embeddings", "Cleanup failed: " + std::string(e.what()));
            return;
        }
        
        // Store documents with embedding metadata
        int created_count = 0;
        for (int i = 0; i < 3; ++i) {
            std::string text = "Document " + std::to_string(i);
            std::string doc_id = "doc" + std::to_string(i);
            std::string insert_query = "INSERT INTO " + collection + " (_id, text, embedding_dim, has_embedding) VALUES ('" + doc_id + "', '" + text + "', 3, true);";
            try {
                client.query(collection, insert_query);
                created_count++;
            } catch (const std::exception& e) {
                // fail silently
            }
        }
        
        if (created_count == 3) {
            results.record_pass("Database with embedding metadata");
        } else {
            results.record_fail("Database with embedding metadata", "Failed to store documents");
        }

        // Verify the documents were stored
        std::string select_query = "SELECT COUNT(*) FROM " + collection + " WHERE has_embedding = true;";
        try {
            std::string response = client.query(collection, select_query);
            if (response.find("\"COUNT(*)\":3") != std::string::npos) {
                results.record_pass("Verification of documents with embeddings");
            } else {
                results.record_fail("Verification of documents with embeddings", "Verification query failed. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("Verification of documents with embeddings", e.what());
        }

    } catch (const std::exception& e) {
        results.record_fail("Database with embeddings", e.what());
    }
}

void test_retrieval_pipeline() {
    std::cout << "\n=== Testing Complete Retrieval Pipeline ===" << std::endl;
    
    try {
        // Simulate a complete retrieval pipeline
        std::vector<std::vector<float>> doc_embeddings = {
            {0.8f, 0.2f, 0.1f},
            {0.1f, 0.9f, 0.2f},
            {0.7f, 0.3f, 0.2f}
        };
        
        std::vector<float> query_embedding = {0.9f, 0.1f, 0.1f};
        
        CosineSimilarityStrategy strategy;
        auto similarities = strategy.calculate_similarity(query_embedding, doc_embeddings);
        
        // Find best match
        int best_idx = 0;
        float best_score = similarities[0];
        for (size_t i = 1; i < similarities.size(); ++i) {
            if (similarities[i] > best_score) {
                best_score = similarities[i];
                best_idx = i;
            }
        }
        
        if (best_idx == 0) {  // First document should be most similar
            results.record_pass("Complete retrieval pipeline");
        } else {
            results.record_fail("Complete retrieval pipeline", "Incorrect best match");
        }
    } catch (const std::exception& e) {
        results.record_fail("Retrieval pipeline", e.what());
    }
}

void test_advanced_queries() {
    std::cout << "\n=== Testing Advanced Queries ===" << std::endl;
    
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        std::string orders_collection = "orders";
        std::string customers_collection = "customers";
        client.ensure_db_setup({orders_collection, customers_collection});

        // Clean up collections before test to ensure isolation
        try {
            client.query(orders_collection, "DELETE FROM " + orders_collection + ";");
            client.query(customers_collection, "DELETE FROM " + customers_collection + ";");
        } catch (const std::exception& e) {
            std::cerr << "  (Note: Initial cleanup query failed in advanced queries, proceeding...)" << std::endl;
        }
        
        // Insert customers
        client.query(customers_collection, "INSERT INTO " + customers_collection + " (_id, name) VALUES ('cust1', 'Alice');");
        client.query(customers_collection, "INSERT INTO " + customers_collection + " (_id, name) VALUES ('cust2', 'Bob');");

        // Insert orders
        client.query(orders_collection, "INSERT INTO " + orders_collection + " (customer_id, item, amount) VALUES ('cust1', 'Laptop', 1200);");
        client.query(orders_collection, "INSERT INTO " + orders_collection + " (customer_id, item, amount) VALUES ('cust2', 'Mouse', 25);");
        client.query(orders_collection, "INSERT INTO " + orders_collection + " (customer_id, item, amount) VALUES ('cust1', 'Keyboard', 75);");

        // Test JOIN
        std::string join_query = "SELECT c.name, o.item, o.amount FROM " + orders_collection + " o JOIN " + customers_collection + " c ON o.customer_id = c._id;";
        try {
            std::string response = client.query(orders_collection, join_query);
            if (response.find("Alice") != std::string::npos && response.find("Laptop") != std::string::npos && response.find("Bob") != std::string::npos) {
                results.record_pass("JOIN query");
            } else {
                results.record_fail("JOIN query", "Unexpected result. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("JOIN query", e.what());
        }

        // Test GROUP BY
        std::string groupby_query = "SELECT customer_id, SUM(amount) FROM " + orders_collection + " GROUP BY customer_id;";
        try {
            std::string response = client.query(orders_collection, groupby_query);
            if (response.find("1275") != std::string::npos && response.find("25") != std::string::npos) {
                results.record_pass("GROUP BY query");
            } else {
                results.record_fail("GROUP BY query", "Unexpected result. Response: " + response);
            }
        } catch (const std::exception& e) {
            results.record_fail("GROUP BY query", e.what());
        }

    } catch (const std::exception& e) {
        results.record_fail("Advanced Queries", e.what());
    }
}

// ============================================================================ 
// Main Test Runner
// ============================================================================ 

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "TissLM C++ Database and Retrieval Test Suite" << std::endl;
    std::cout << "Testing against TissDB on 127.0.0.1:9876" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // Clean up and create database before running tests
    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db", "static_test_token");
        client.delete_database();
        client.create_database();
    } catch (const std::exception& e) {
        std::cerr << "Error during test setup: " << e.what() << std::endl;
    }
    
    // Database Tests
    test_db_connection();
    test_db_setup();
    test_document_crud();
    test_feedback_collection();
    test_multiple_documents();
    test_document_search();
    
    // Retrieval Strategy Tests
    test_cosine_similarity();
    test_euclidean_distance();
    test_dot_product();
    test_bm25();
    test_hybrid_strategy();
    
    // Integration Tests
    test_db_with_embeddings();
    test_retrieval_pipeline();
    test_advanced_queries();
    
    // Print summary
    results.print_summary();
    
    return (results.failed == 0) ? 0 : 1;
}
