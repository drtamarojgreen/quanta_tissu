#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

namespace TissDB {

// Forward declarations
class HttpClient;

/**
 * Exception thrown when database operations fail
 */
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message) 
        : std::runtime_error(message) {}
};

/**
 * Represents a document in TissDB
 */
struct Document {
    std::string id;
    std::map<std::string, std::string> fields;
    
    Document() = default;
    Document(const std::string& doc_id) : id(doc_id) {}
    
    void set_field(const std::string& key, const std::string& value) {
        fields[key] = value;
    }
    
    std::string get_field(const std::string& key) const {
        auto it = fields.find(key);
        return (it != fields.end()) ? it->second : "";
    }
    
    // Convert to JSON string
    std::string to_json() const;
    
    // Parse from JSON string
    static Document from_json(const std::string& json_str);
};

/**
 * Client for interacting with TissDB HTTP API
 */
class TissDBClient {
public:
    /**
     * Constructor
     * @param host Database host (default: "127.0.0.1")
     * @param port Database port (default: 9876)
     * @param db_name Database name (default: "testdb")
     */
    TissDBClient(const std::string& host = "127.0.0.1", 
                 int port = 9876,
                 const std::string& db_name = "testdb");
    
    ~TissDBClient();
    
    /**
     * Ensure database and collections exist
     * @param collections List of collection names to create
     * @return true if successful
     */
    bool ensure_db_setup(const std::vector<std::string>& collections);
    
    /**
     * Add a document to a collection
     * @param collection Collection name
     * @param document Document to add
     * @param doc_id Document ID (generated if empty)
     * @return Document ID
     */
    std::string add_document(const std::string& collection, 
                            const Document& document,
                            const std::string& doc_id = "");
    
    /**
     * Get a document by ID
     * @param collection Collection name
     * @param doc_id Document ID
     * @return Document
     */
    Document get_document(const std::string& collection, const std::string& doc_id);
    
    /**
     * Get database statistics
     * @return Statistics as a map
     */
    std::map<std::string, std::string> get_stats();
    
    /**
     * Add feedback data
     * @param feedback_data Feedback document
     * @return Feedback ID
     */
    std::string add_feedback(const Document& feedback_data);
    
    /**
     * Test connection to database
     * @return true if connection is successful
     */
    bool test_connection();

    /**
     * Search documents in a collection based on a query.
     * @param collection Collection name.
     * @param query_json JSON string representing the query.
     * @return A vector of matching Documents.
     */
    std::vector<Document> search_documents(const std::string& collection, const std::string& query_json);

private:
    std::string host_;
    int port_;
    std::string db_name_;
    std::string base_url_;
    std::string db_url_;
    std::unique_ptr<HttpClient> http_client_;
    
    // Generate a unique document ID
    std::string generate_doc_id() const;
    
    // Helper to build collection URL
    std::string get_collection_url(const std::string& collection) const;
};

} // namespace TissDB
