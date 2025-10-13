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

// ============================================================================
// Database Tests
// ============================================================================

void test_db_connection() {
    std::cout << "\n=== Testing Database Connection ===" << std::endl;

    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");

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
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");
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
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");
        std::vector<std::string> collections = {"test_docs"};
        client.ensure_db_setup(collections);

        // Create
        Document doc;
        doc.set_field("title", "Test Document");
        doc.set_field("content", "This is test content");
        doc.set_field("author", "Unit Test");

        std::string doc_id = client.add_document("test_docs", doc);

        if (!doc_id.empty()) {
            results.record_pass("Document creation");
        } else {
            results.record_fail("Document creation", "Empty document ID returned");
            return;
        }

        // Read
        try {
            Document retrieved = client.get_document("test_docs", doc_id);

            if (retrieved.get_field("title") == "Test Document" &&
                retrieved.get_field("content") == "This is test content" &&
                retrieved.get_field("author") == "Unit Test") {
                results.record_pass("Document retrieval");
            } else {
                results.record_fail("Document retrieval", "Retrieved data doesn't match");
            }
        } catch (const std::exception& e) {
            results.record_fail("Document retrieval", e.what());
        }

    } catch (const std::exception& e) {
        results.record_fail("Document CRUD", e.what());
    }
}

void test_feedback_collection() {
    std::cout << "\n=== Testing Feedback Collection ===" << std::endl;

    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");
        std::vector<std::string> collections = {"feedback"};
        client.ensure_db_setup(collections);

        Document feedback;
        feedback.set_field("rating", "5");
        feedback.set_field("comment", "Excellent system!");
        feedback.set_field("user", "test_user");
        feedback.set_field("feature", "retrieval");

        std::string feedback_id = client.add_feedback(feedback);

        if (!feedback_id.empty() && feedback_id.find("feedback_") == 0) {
            results.record_pass("Feedback collection");
        } else {
            results.record_fail("Feedback collection", "Invalid feedback ID");
        }
    } catch (const std::exception& e) {
        results.record_fail("Feedback collection", e.what());
    }
}

void test_multiple_documents() {
    std::cout << "\n=== Testing Multiple Document Operations ===" << std::endl;

    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");
        std::vector<std::string> collections = {"test_docs"};
        client.ensure_db_setup(collections);

        std::vector<std::string> doc_ids;

        // Add multiple documents
        for (int i = 0; i < 5; ++i) {
            Document doc;
            doc.set_field("title", "Document " + std::to_string(i));
            doc.set_field("content", "Content for document " + std::to_string(i));
            doc.set_field("index", std::to_string(i));

            std::string doc_id = client.add_document("test_docs", doc);
            doc_ids.push_back(doc_id);
        }

        if (doc_ids.size() == 5) {
            results.record_pass("Multiple document creation");
        } else {
            results.record_fail("Multiple document creation", "Not all documents created");
        }

        // Verify all documents
        int verified = 0;
        for (size_t i = 0; i < doc_ids.size(); ++i) {
            try {
                Document retrieved = client.get_document("test_docs", doc_ids[i]);
                if (retrieved.get_field("index") == std::to_string(i)) {
                    verified++;
                }
            } catch (...) {
                // Document not found
            }
        }

        if (verified == 5) {
            results.record_pass("Multiple document retrieval");
        } else {
            results.record_fail("Multiple document retrieval",
                              "Only " + std::to_string(verified) + "/5 documents verified");
        }

    } catch (const std::exception& e) {
        results.record_fail("Multiple documents", e.what());
    }
}

void test_document_search() {
    std::cout << "\n=== Testing Document Search ===" << std::endl;

    try {
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");
        std::vector<std::string> collections = {"search_docs"};
        client.ensure_db_setup(collections);

        // Add sample documents
        Document doc1;
        doc1.set_field("title", "Mars Mission Overview");
        doc1.set_field("content", "The first manned mission to Mars, named 'Ares 1', is scheduled for 2035.");
        client.add_document("search_docs", doc1, "doc_mars");

        Document doc2;
        doc2.set_field("title", "Moon Landing History");
        doc2.set_field("content", "The Apollo 11 mission landed humans on the Moon in 1969.");
        client.add_document("search_docs", doc2, "doc_moon");

        Document doc3;
        doc3.set_field("title", "Future Space Exploration");
        doc3.set_field("content", "Plans for future space exploration include missions to Jupiter's moons.");
        client.add_document("search_docs", doc3, "doc_jupiter");

        // Search for documents containing "Mars"
        std::string query_json = "{\"query\": \"Mars\"}";
        std::vector<Document> search_results = client.search_documents("search_docs", query_json);

        if (search_results.size() == 1 && search_results[0].get_field("title") == "Mars Mission Overview") {
            results.record_pass("Document search for 'Mars'");
        } else {
            results.record_fail("Document search for 'Mars'", "Unexpected search results or count");
        }

        // Search for documents containing "Moon"
        query_json = "{\"query\": \"Moon\"}";
        search_results = client.search_documents("search_docs", query_json);

        if (search_results.size() == 1 && search_results[0].get_field("title") == "Moon Landing History") {
            results.record_pass("Document search for 'Moon'");
        } else {
            results.record_fail("Document search for 'Moon'", "Unexpected search results or count for 'Moon'");
        }

        // Search for documents containing "exploration" (should find two)
        query_json = "{\"query\": \"exploration\"}";
        search_results = client.search_documents("search_docs", query_json);

        if (search_results.size() == 2) { // Assuming both Mars and Jupiter docs contain "exploration" implicitly or explicitly
            results.record_pass("Document search for 'exploration'");
        } else {
            results.record_fail("Document search for 'exploration'", "Expected 2 documents, got " + std::to_string(search_results.size()));
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
        TissDBClient client("127.0.0.1", 9876, "test_cpp_db");
        std::vector<std::string> collections = {"embeddings"};
        client.ensure_db_setup(collections);

        // Store documents with embedding metadata
        std::vector<std::string> doc_ids;
        for (int i = 0; i < 3; ++i) {
            Document doc;
            doc.set_field("text", "Document " + std::to_string(i));
            doc.set_field("embedding_dim", "3");
            doc.set_field("has_embedding", "true");

            std::string doc_id = client.add_document("embeddings", doc);
            doc_ids.push_back(doc_id);
        }

        if (doc_ids.size() == 3) {
            results.record_pass("Database with embedding metadata");
        } else {
            results.record_fail("Database with embedding metadata", "Failed to store documents");
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

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "TissLM C++ Database and Retrieval Test Suite" << std::endl;
    std::cout << "Testing against TissDB on 127.0.0.1:9876" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

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

    // Print summary
    results.print_summary();

    return (results.failed == 0) ? 0 : 1;
}
