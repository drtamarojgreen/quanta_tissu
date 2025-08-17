#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/common/document.h"

TEST_CASE(LSMTreeCreateDropCollection) {
    TissDB::Storage::LSMTree db;

    ASSERT_TRUE(db.create_collection("users"));
    ASSERT_FALSE(db.create_collection("users")); // Should fail, already exists

    ASSERT_TRUE(db.get_collection("users") != nullptr);
    ASSERT_TRUE(db.get_collection("non_existent") == nullptr);

    ASSERT_TRUE(db.drop_collection("users"));
    ASSERT_FALSE(db.drop_collection("users")); // Should fail, already dropped

    ASSERT_TRUE(db.get_collection("users") == nullptr);
}

TEST_CASE(LSMTreeDataOperations) {
    TissDB::Storage::LSMTree db;
    db.create_collection("products");

    TissDB::Document doc1;
    doc1.id = "prod1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Laptop");
    doc1.elements.push_back(elem1);

    db.put("products", "prod1", doc1);

    auto retrieved_doc_opt = db.get("products", "prod1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() != nullptr);
    ASSERT_EQ("Laptop", std::get<std::string>(retrieved_doc_opt.value()->elements[0].value));

    db.del("products", "prod1");
    retrieved_doc_opt = db.get("products", "prod1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() == nullptr); // Tombstone
}

TEST_CASE(LSMTreeDataIsolation) {
    TissDB::Storage::LSMTree db;
    db.create_collection("col1");
    db.create_collection("col2");

    TissDB::Document doc1;
    doc1.id = "doc1";
    db.put("col1", "doc1", doc1);

    // doc1 should be in col1, but not in col2
    ASSERT_TRUE(db.get("col1", "doc1").has_value());
    ASSERT_FALSE(db.get("col2", "doc1").has_value());
}

TEST_CASE(LSMTreeOperationsOnNonExistentCollection) {
    TissDB::Storage::LSMTree db;

    TissDB::Document doc1;
    doc1.id = "doc1";

    // These operations should not cause any errors
    db.put("non_existent", "doc1", doc1);
    auto retrieved_doc_opt = db.get("non_existent", "doc1");
    ASSERT_FALSE(retrieved_doc_opt.has_value());
    db.del("non_existent", "doc1");
}
