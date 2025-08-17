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
Document create_doc_simple(const std::string& id, const std::string& name, int value) {
    Document doc;
    doc.id = id;
    doc.elements.push_back({"name", name});
    doc.elements.push_back({"value", (double)value}); // Store as double for consistency
    return doc;
}

struct UpdateDeleteTestFixture {
    const std::string test_dir = "./test_update_delete_data";
    std::unique_ptr<Storage::LSMTree> storage;
    std::unique_ptr<Query::Executor> executor;

    UpdateDeleteTestFixture() {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
        storage = std::make_unique<Storage::LSMTree>(test_dir);
        executor = std::make_unique<Query::Executor>(*storage);

        // Setup
        storage->create_collection("test_coll", Schema());
        storage->put("test_coll", "1", create_doc_simple("1", "doc1", 10));
        storage->put("test_coll", "2", create_doc_simple("2", "doc2", 20));
        storage->put("test_coll", "3", create_doc_simple("3", "doc3", 30));
    }

    ~UpdateDeleteTestFixture() {
        storage->shutdown();
        std::filesystem::remove_all(test_dir);
    }
};

TEST_CASE(ParserHandlesUpdateQuery) {
    Query::Parser parser;
    std::cout << "\nTesting UPDATE query parsing..." << std::endl;
    Query::AST ast = parser.parse("UPDATE test_coll SET value = 40 WHERE name = 'doc1'");

    ASSERT_EQ(ast.type, Query::AST::StatementType::UPDATE);
    ASSERT_EQ(ast.collection_name, "test_coll");
    ASSERT_EQ(ast.update_data.size(), 1);
    ASSERT_EQ(ast.update_data[0].first, "value");
    ASSERT_EQ(std::get<double>(ast.update_data[0].second), 40.0);
    ASSERT_TRUE(ast.where_clause.has_value());
}

TEST_CASE(ExecutorHandlesUpdateQuery) {
    UpdateDeleteTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting UPDATE query execution..." << std::endl;
    Query::AST ast = parser.parse("UPDATE test_coll SET value = 100 WHERE name = 'doc2'");
    fixture.executor->execute(ast);

    auto updated_doc_variant = fixture.storage->get("test_coll", "2");
    ASSERT_TRUE(std::holds_alternative<Document>(updated_doc_variant));
    Document updated_doc = std::get<Document>(updated_doc_variant);

    bool found = false;
    for(const auto& el : updated_doc.elements) {
        if (el.key == "value") {
            ASSERT_EQ(std::get<double>(el.value), 100.0);
            found = true;
        }
    }
    ASSERT_TRUE(found);
}

TEST_CASE(ParserHandlesDeleteQuery) {
    Query::Parser parser;
    std::cout << "\nTesting DELETE query parsing..." << std::endl;
    Query::AST ast = parser.parse("DELETE FROM test_coll WHERE value > 15");

    ASSERT_EQ(ast.type, Query::AST::StatementType::DELETE);
    ASSERT_EQ(ast.collection_name, "test_coll");
    ASSERT_TRUE(ast.where_clause.has_value());
}

TEST_CASE(ExecutorHandlesDeleteQuery) {
    UpdateDeleteTestFixture fixture;
    Query::Parser parser;

    std::cout << "\nTesting DELETE query execution..." << std::endl;
    Query::AST ast = parser.parse("DELETE FROM test_coll WHERE value > 25");
    fixture.executor->execute(ast);

    auto deleted_doc_variant = fixture.storage->get("test_coll", "3");
    ASSERT_TRUE(std::holds_alternative<std::monostate>(deleted_doc_variant)); // Should not exist

    auto existing_doc_variant = fixture.storage->get("test_coll", "1");
    ASSERT_TRUE(std::holds_alternative<Document>(existing_doc_variant)); // Should still exist
}
