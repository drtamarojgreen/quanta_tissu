#include "test_framework.h"
#include "http_client.h"
#include <string>
#include <iostream>

// Assuming TissDB runs on localhost:8080
const std::string TISSDB_HOST = "localhost";
const int TISSDB_PORT = 8080;

TEST_CASE(TissDB_CRUD_BDD_Scenario) {
    TissDB::Test::HttpClient client(TISSDB_HOST, TISSDB_PORT);
    std::string collection_name = "/test_collection";
    std::string document_id = "doc1";
    std::string document_path = collection_name + "/" + document_id;

    // Given: A clean state (ensure document doesn't exist from previous runs)
    // Attempt to delete if it exists, ignore if not found
    try {
        client.del(document_path);
    } catch (const std::exception& e) {
        // Ignore if document not found, otherwise rethrow
        if (std::string(e.what()).find("Error connecting to server") == std::string::npos) {
            // If it's not a connection error, it might be a 404, which is fine for cleanup
            // For a robust test, we'd check status code, but for basic, this is okay.
        } else {
            // If TissDB is not running, we can't proceed with tests.
            FAIL("TissDB server not running or accessible at " + TISSDB_HOST + ":" + std::to_string(TISSDB_PORT));
        }
    }


    // When: I create a document
    std::string create_body = "{\"name\": \"Alice\", \"age\": 30}";
    TissDB::Test::HttpResponse post_response = client.post(document_path, create_body);
    ASSERT_EQ(201, post_response.status_code); // 201 Created

    // Then: The document should be retrievable
    TissDB::Test::HttpResponse get_response = client.get(document_path);
    ASSERT_EQ(200, get_response.status_code);
    ASSERT_TRUE(get_response.body.find("\"name\":\"Alice\"") != std::string::npos);
    ASSERT_TRUE(get_response.body.find("\"age\":30") != std::string::npos);

    // When: I update the document
    std::string update_body = "{\"name\": \"Alicia\", \"city\": \"New York\"}";
    TissDB::Test::HttpResponse put_response = client.put(document_path, update_body);
    ASSERT_EQ(200, put_response.status_code);

    // Then: The document should reflect the update
    TissDB::Test::HttpResponse get_updated_response = client.get(document_path);
    ASSERT_EQ(200, get_updated_response.status_code);
    ASSERT_TRUE(get_updated_response.body.find("\"name\":\"Alicia\"") != std::string::npos);
    ASSERT_TRUE(get_updated_response.body.find("\"city\":\"New York\"") != std::string::npos);
    ASSERT_TRUE(get_updated_response.body.find("\"age\":30") == std::string::npos); // Age should be replaced by PUT

    // When: I delete the document
    TissDB::Test::HttpResponse delete_response = client.del(document_path);
    ASSERT_EQ(200, delete_response.status_code);

    // Then: The document should no longer be retrievable
    TissDB::Test::HttpResponse get_deleted_response = client.get(document_path);
    ASSERT_EQ(404, get_deleted_response.status_code);
}
