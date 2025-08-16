#include "test_framework.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/query/parser.h"
#include "../../tissdb/storage/lsm_tree.h"
#include <filesystem>
#include <cmath>
#include <iostream>

// Mock LSMTree for testing executor in isolation
class MockLSMTreeForStdDev : public TissDB::Storage::LSMTree {
public:
    MockLSMTreeForStdDev() : TissDB::Storage::LSMTree("mock_data_stddev") {}

    void put(const std::string& collection_name, const std::string& key, const TissDB::Document& doc) override {
        mock_data_[collection_name][key] = doc;
    }

    std::optional<TissDB::Document> get(const std::string& collection_name, const std::string& key) override {
        if (mock_data_.count(collection_name) && mock_data_[collection_name].count(key)) {
            return mock_data_[collection_name][key];
        }
        return std::nullopt;
    }

    std::vector<TissDB::Document> scan(const std::string& collection_name) override {
        std::vector<TissDB::Document> docs;
        if (mock_data_.count(collection_name)) {
            for (const auto& pair : mock_data_[collection_name]) {
                docs.push_back(pair.second);
            }
        }
        return docs;
    }

    std::map<std::string, std::map<std::string, TissDB::Document>> mock_data_;
};

TEST_CASE(ExecutorAggregateStdDev) {
    MockLSMTreeForStdDev mock_lsm_tree;
    mock_lsm_tree.create_collection("data");

    // Setup initial data
    mock_lsm_tree.put("data", "1", TissDB::Document{"1", {{"value", 10.0}}});
    mock_lsm_tree.put("data", "2", TissDB::Document{"2", {{"value", 20.0}}});
    mock_lsm_tree.put("data", "3", TissDB::Document{"3", {{"value", 30.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // Execute the query
    TissDB::Query::AST ast = parser.parse("SELECT STDDEV(value) FROM data");
    TissDB::Query::QueryResult result = executor.execute(ast);

    // Verify the results
    ASSERT_EQ(1, result.documents.size());
    const auto& doc = result.documents[0];
    ASSERT_EQ(1, doc.elements.size());
    const auto& elem = doc.elements[0];
    ASSERT_EQ("stddev", elem.key);

    // The population standard deviation of {10, 20, 30} is sqrt(((10-20)^2 + (20-20)^2 + (30-20)^2)/3) = sqrt(200/3) = 8.16496...
    double expected_stddev = 8.16496580927726;
    double actual_stddev = std::get<double>(elem.value);
    double tolerance = 1e-9;

    std::cout << "Expected STDDEV: " << expected_stddev << std::endl;
    std::cout << "Actual STDDEV: " << actual_stddev << std::endl;

    ASSERT_TRUE(std::abs(expected_stddev - actual_stddev) < tolerance);

    std::filesystem::remove_all("mock_data_stddev");
}
