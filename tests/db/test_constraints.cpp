#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/storage/collection.h"
#include "../../tissdb/common/document.h"
#include "../../tissdb/common/schema.h"
#include <filesystem>
#include <stdexcept>

TEST_CASE(CollectionUniqueConstraint) {
    std::string collection_path = "test_collection_unique";
    if (std::filesystem::exists(collection_path)) {
        std::filesystem::remove_all(collection_path);
    }

    TissDB::Storage::LSMTree lsm_tree;
    TissDB::Storage::Collection collection(&lsm_tree, collection_path);

    // 1. Define Schema
    TissDB::Schema schema;
    schema.add_field("email", TissDB::FieldType::String, true, true); // required, unique
    collection.set_schema(schema);

    // 2. Create Index
    collection.create_index({"email"});

    // 3. Insert first document (should succeed)
    TissDB::Document doc1;
    doc1.id = "user1";
    TissDB::Element elem1;
    elem1.key = "email";
    elem1.value = std::string("alice@example.com");
    doc1.elements.push_back(elem1);

    bool success = false;
    try {
        collection.put("user1", doc1);
        success = true;
    } catch (const std::exception& e) {
        // No exception should be thrown here
    }
    ASSERT_TRUE(success);

    // 4. Insert second document with duplicate email (should fail)
    TissDB::Document doc2;
    doc2.id = "user2";
    TissDB::Element elem2;
    elem2.key = "email";
    elem2.value = std::string("alice@example.com");
    doc2.elements.push_back(elem2);

    bool exception_thrown = false;
    try {
        collection.put("user2", doc2);
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        std::string error_msg = e.what();
        ASSERT_TRUE(error_msg.find("uniqueness constraint violated") != std::string::npos);
    }
    ASSERT_TRUE(exception_thrown);

    // 5. Insert third document with a different email (should succeed)
    TissDB::Document doc3;
    doc3.id = "user3";
    TissDB::Element elem3;
    elem3.key = "email";
    elem3.value = std::string("bob@example.com");
    doc3.elements.push_back(elem3);

    success = false;
    try {
        collection.put("user3", doc3);
        success = true;
    } catch (const std::exception& e) {
        // No exception should be thrown here
    }
    ASSERT_TRUE(success);

    // The `shutdown` method is on the `LSMTree` object, not the `Collection` object.
    // lsm_tree.shutdown();
    std::filesystem::remove_all(collection_path);
}