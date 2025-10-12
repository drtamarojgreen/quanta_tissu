#include "test_framework.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/query/parser.h"
#include "../../tissdb/storage/lsm_tree.h"
#include <filesystem>
#include <cmath>
#include <iostream>
#include "../../tissdb/storage/transaction_manager.h"

// Mock LSMTree for testing executor in isolation
class MockLSMTreeForStdDev : public TissDB::Storage::LSMTree {
public:
    MockLSMTreeForStdDev() : TissDB::Storage::LSMTree() {}

    void create_collection(const std::string& name, const TissDB::Schema& schema, bool is_recovery = false) override {
        // Mock implementation, can be empty if not needed for the test logic
        (void)name;
        (void)schema;
        (void)is_recovery;
    }

    void put(const std::string& collection_name, const std::string& key, const TissDB::Document& doc, TissDB::Transactions::TransactionID tid = -1, bool is_recovery = false) override {
        (void)tid; // Unused in mock
        (void)is_recovery; // Unused in mock
        mock_data_[collection_name][key] = doc;
    }

        std::optional<std::shared_ptr<TissDB::Document>> get(const std::string& collection_name, const std::string& key, TissDB::Transactions::TransactionID tid = -1) override {
        (void)tid; // Unused in mock
        if (mock_data_.count(collection_name) && mock_data_[collection_name].count(key)) {
            return std::make_shared<TissDB::Document>(mock_data_[collection_name][key]);
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
    mock_lsm_tree.create_collection("data", {});

    // Setup initial data
    mock_lsm_tree.put("data", "1", TissDB::Document{"1", {TissDB::Element{"value", 10.0}}});
    mock_lsm_tree.put("data", "2", TissDB::Document{"2", {TissDB::Element{"value", 20.0}}});
    mock_lsm_tree.put("data", "3", TissDB::Document{"3", {TissDB::Element{"value", 30.0}}});

    TissDB::Query::Parser parser;
    TissDB::Query::Executor executor(mock_lsm_tree);

    // Execute the query
    TissDB::Query::AST ast = parser.parse("SELECT STDDEV(value) FROM data");
    TissDB::Query::QueryResult result = executor.execute(ast, {});

    // Verify the results
    ASSERT_EQ(1, result.size());
    const auto& doc = result[0];
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