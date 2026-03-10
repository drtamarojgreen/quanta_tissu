#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/query/parser.h"
#include "../../tissdb/query/executor.h"
#include "../../tissdb/common/document.h"
#include <filesystem>
#include <vector>
#include <string>

using namespace TissDB;

// Helper function to create a document
Document create_doc(const std::string& id, const std::string& brand, const std::string& type, double price) {
    Document doc;
    doc.id = id;
    doc.elements.push_back({"brand", brand});
    doc.elements.push_back({"type", type});
    doc.elements.push_back({"price", price});
    return doc;
}

struct ExecutorTestFixture {
    const std::string test_dir = "./test_executor_data";
    std::unique_ptr<Storage::LSMTree> storage;
    std::unique_ptr<Query::Executor> executor;

    ExecutorTestFixture() {
        std::filesystem::remove_all(test_dir);
        storage = std::make_unique<Storage::LSMTree>(test_dir);
        executor = std::make_unique<Query::Executor>(*storage);

        // Setup
        storage->create_collection("products", Schema());
        storage->put("products", "1", create_doc("1", "AudioPhonic", "headphones", 200.0));
        storage->put("products", "2", create_doc("2", "AudioPhonic", "speakers", 500.0));
        storage->put("products", "3", create_doc("3", "TechGear", "headphones", 150.0));

        // Create Indexes
        storage->create_index("products", {"brand"});
        storage->create_index("products", {"brand", "type"});
    }

    ~ExecutorTestFixture() {
        storage->shutdown();
        std::filesystem::remove_all(test_dir);
    }
};

TEST_CASE(ExecutorUsesSingleIndex) {
    ExecutorTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting single index usage..." << std::endl;
    Query::AST ast = parser.parse("SELECT * FROM products WHERE brand = 'TechGear'");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("3", result[0].id);
}

TEST_CASE(ExecutorWithParameters) {
    ExecutorTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting query with parameters..." << std::endl;
    Query::AST ast = parser.parse("SELECT * FROM products WHERE brand = ? AND price > ?");
    std::vector<Query::Literal> params = {"AudioPhonic", 100.0};
    Query::QueryResult result = fixture.executor->execute(ast, params);

    ASSERT_EQ(2, result.size());
    // Sort results by ID to have a deterministic order for checking
    std::sort(result.begin(), result.end(), [](const Document& a, const Document& b){
        return a.id < b.id;
    });
    ASSERT_EQ("1", result[0].id);
    ASSERT_EQ("2", result[1].id);
}

TEST_CASE(ExecutorParameterCountMismatch) {
    ExecutorTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting parameter count mismatch..." << std::endl;
    Query::AST ast = parser.parse("SELECT * FROM products WHERE brand = ? AND price > ?");
    std::vector<Query::Literal> params = {"AudioPhonic"}; // Only one parameter

    ASSERT_THROW(fixture.executor->execute(ast, params), std::runtime_error);
}

TEST_CASE(ExecutorUsesCompoundIndex) {
    ExecutorTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting compound index usage..." << std::endl;
    Query::AST ast = parser.parse("SELECT * FROM products WHERE brand = 'AudioPhonic' AND type = 'headphones'");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("1", result[0].id);
}

TEST_CASE(ExecutorFallsBackToSingleIndex) {
    ExecutorTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting fallback to single index..." << std::endl;
    // The compound index is (brand, type), but the query is on (brand, price).
    // It should use the single-field index on 'brand'.
    Query::AST ast = parser.parse("SELECT * FROM products WHERE brand = 'AudioPhonic' AND price = 500");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    // The index on 'brand' will return docs 1 and 2. The filter will then remove doc 1.
    ASSERT_EQ(1, result.size());
    ASSERT_EQ("2", result[0].id);
}

TEST_CASE(ExecutorPerformsFullScan) {
    ExecutorTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting full scan..." << std::endl;
    // No index on 'price'
    Query::AST ast = parser.parse("SELECT * FROM products WHERE price = 150");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("3", result[0].id);
}


struct TemporalExecutorFixture {
    const std::string test_dir = "./test_temporal_executor_data";
    std::unique_ptr<Storage::LSMTree> storage;
    std::unique_ptr<Query::Executor> executor;

    TemporalExecutorFixture() {
        std::filesystem::remove_all(test_dir);
        storage = std::make_unique<Storage::LSMTree>(test_dir);
        executor = std::make_unique<Query::Executor>(*storage);
        storage->create_collection("events", Schema());

        Document d1;
        d1.id = "e1";
        d1.elements.push_back({"name", std::string("A")});
        d1.elements.push_back({"ts", Timestamp{1722074400000000LL}}); // 2024-07-27T10:00:00Z
        d1.elements.push_back({"event_date", Date{2024, 7, 27}});
        d1.elements.push_back({"event_time", Time{9, 15, 0}});

        Document d2;
        d2.id = "e2";
        d2.elements.push_back({"name", std::string("B")});
        d2.elements.push_back({"ts", Timestamp{1722074700000000LL}}); // 2024-07-27T10:05:00Z
        d2.elements.push_back({"event_date", Date{2024, 7, 28}});
        d2.elements.push_back({"event_time", Time{10, 30, 0}});

        storage->put("events", d1.id, d1);
        storage->put("events", d2.id, d2);
    }

    ~TemporalExecutorFixture() {
        storage->shutdown();
        std::filesystem::remove_all(test_dir);
    }
};

TEST_CASE(ExecutorTimestampBetweenFilter) {
    TemporalExecutorFixture fixture;
    Query::Parser parser;

    Query::AST ast = parser.parse(
        "SELECT * FROM events WHERE ts BETWEEN TIMESTAMP '2024-07-27T10:01:00Z' AND TIMESTAMP '2024-07-27T10:09:00Z'");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("e2", result[0].id);
}

TEST_CASE(ExecutorDateAndTimeLiteralsFilter) {
    TemporalExecutorFixture fixture;
    Query::Parser parser;

    Query::AST ast = parser.parse(
        "SELECT * FROM events WHERE event_date = DATE '2024-07-27' AND event_time = TIME '09:15:00'");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("e1", result[0].id);
}

TEST_CASE(ExecutorDateTimeFunctionsExtractAndNow) {
    TemporalExecutorFixture fixture;
    Query::Parser parser;

    Query::AST extract_ast = parser.parse(
        "SELECT * FROM events WHERE EXTRACT(YEAR FROM ts) = 2024 AND DATE(ts) = DATE '2024-07-27' AND TIME(ts) > TIME '10:00:00'");
    Query::QueryResult extract_result = fixture.executor->execute(extract_ast, {});

    ASSERT_EQ(1, extract_result.size());
    ASSERT_EQ("e2", extract_result[0].id);

    Query::AST now_ast = parser.parse("SELECT * FROM events WHERE ts <= NOW()");
    Query::QueryResult now_result = fixture.executor->execute(now_ast, {});
    ASSERT_EQ(2, now_result.size());
}

TEST_CASE(ExecutorTimestampIntervalArithmetic) {
    TemporalExecutorFixture fixture;
    Query::Parser parser;

    Query::AST ast = parser.parse(
        "SELECT * FROM events WHERE ts + INTERVAL 5 MINUTES > TIMESTAMP '2024-07-27T10:06:00Z'");
    Query::QueryResult result = fixture.executor->execute(ast, {});

    ASSERT_EQ(1, result.size());
    ASSERT_EQ("e2", result[0].id);
}
