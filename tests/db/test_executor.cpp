
#include "test_framework.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/query/parser.h"
#include "../../tissdb/storage/lsm_tree.h"
#include <filesystem>

// Mock LSMTree for testing executor in isolation
class MockLSMTree : public TissDB::Storage::LSMTree {
public:
    MockLSMTree() : TissDB::Storage::LSMTree("mock_data") {}

    // Override put to store documents in memory for testing
    void put(const std::string& collection_name, const std::string& key, const TissDB::Document& doc) override {
        mock_data_[collection_name][key] = doc;
    }

    // Override get to retrieve from mock data
    std::optional<TissDB::Document> get(const std::string& collection_name, const std::string& key) override {
        if (mock_data_.count(collection_name) && mock_data_[collection_name].count(key)) {
            return mock_data_[collection_name][key];
        }
        return std::nullopt;
    }

    // Override scan to return all docs from mock data
    std::vector<TissDB::Document> scan(const std::string& collection_name) override {
        std::vector<TissDB::Document> docs;
        if (mock_data_.count(collection_name)) {
            for (const auto& pair : mock_data_[collection_name]) {
                docs.push_back(pair.second);
            }
        }
        return docs;
    }

    // Override create_index to just record that an index was created
    void create_index(const std::string& collection_name, const std::string& field_name) override {
        mock_indexes_[collection_name].insert(field_name);
    }

    // Override find_by_index to simulate index lookup
    std::vector<std::string> find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) override {
        std::vector<std::string> result_ids;
        if (mock_indexes_.count(collection_name) && mock_indexes_[collection_name].count(field_name)) {
            // Simulate index lookup: find documents in mock_data that match the criteria
            if (mock_data_.count(collection_name)) {
                for (const auto& pair : mock_data_[collection_name]) {
                    for (const auto& elem : pair.second.elements) {
                        if (elem.key == field_name) {
                            if (auto* str_val = std::get_if<std::string>(&elem.value)) {
                                if (*str_val == value) {
                                    result_ids.push_back(pair.first);
                                }
                            }
                        }
                    }
                }
            }
        }
        return result_ids;
    }

    // Mock data storage
    std::map<std::string, std::map<std::string, TissDB::Document>> mock_data_;
    std::map<std::string, std::set<std::string>> mock_indexes_;
};

TEST_CASE(ExecutorSelectAll) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users");

    TissDB::Document doc1;
    doc1.id = "user1";
    TissDB::Element e1; e1.key = "name"; e1.value = std::string("Alice");
    doc1.elements.push_back(e1);
    mock_lsm_tree.put("users", "user1", doc1);

    TissDB::Document doc2;
    doc2.id = "user2";
    TissDB::Element e2; e2.key = "name"; e2.value = std::string("Bob");
    doc2.elements.push_back(e2);
    mock_lsm_tree.put("users", "user2", doc2);

    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM users");

    TissDB::Query::Executor executor(mock_lsm_tree);
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(2, result.documents.size());
    // Check content (order might vary)
    bool found_user1 = false;
    bool found_user2 = false;
    for (const auto& doc : result.documents) {
        if (doc.id == "user1") found_user1 = true;
        if (doc.id == "user2") found_user2 = true;
    }
    ASSERT_TRUE(found_user1);
    ASSERT_TRUE(found_user2);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithWhere) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("products");

    TissDB::Document doc1;
    doc1.id = "prod1";
    TissDB::Element e1; e1.key = "price"; e1.value = 150.0;
    doc1.elements.push_back(e1);
    mock_lsm_tree.put("products", "prod1", doc1);

    TissDB::Document doc2;
    doc2.id = "prod2";
    TissDB::Element e2; e2.key = "price"; e2.value = 50.0;
    doc2.elements.push_back(e2);
    mock_lsm_tree.put("products", "prod2", doc2);

    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM products WHERE price > 100");

    TissDB::Query::Executor executor(mock_lsm_tree);
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.documents.size());
    ASSERT_EQ("prod1", result.documents[0].id);

    std::filesystem::remove_all("mock_data");
}

TEST_CASE(ExecutorSelectWithIndex) {
    MockLSMTree mock_lsm_tree;
    mock_lsm_tree.create_collection("users");
    mock_lsm_tree.create_index("users", "name");

    TissDB::Document doc1;
    doc1.id = "user1";
    TissDB::Element e1; e1.key = "name"; e1.value = std::string("Alice");
    doc1.elements.push_back(e1);
    mock_lsm_tree.put("users", "user1", doc1);

    TissDB::Document doc2;
    doc2.id = "user2";
    TissDB::Element e2; e2.key = "name"; e2.value = std::string("Bob");
    doc2.elements.push_back(e2);
    mock_lsm_tree.put("users", "user2", doc2);

    TissDB::Query::Parser parser;
    TissDB::Query::AST ast = parser.parse("SELECT * FROM users WHERE name = 'Alice'");

    TissDB::Query::Executor executor(mock_lsm_tree);
    TissDB::Query::QueryResult result = executor.execute(ast);

    ASSERT_EQ(1, result.documents.size());
    ASSERT_EQ("user1", result.documents[0].id);

    std::filesystem::remove_all("mock_data");
}
