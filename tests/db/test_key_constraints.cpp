#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/common/schema.h"
#include <stdexcept>
#include <filesystem>

// Helper to create a simple document
TissDB::Document create_doc(const std::string& id, const std::string& key, const std::string& value) {
    TissDB::Document doc;
    doc.id = id;
    TissDB::Element elem;
    elem.key = key;
    elem.value = value;
    doc.elements.push_back(elem);
    return doc;
}

TEST_CASE(PrimaryKeyConstraint) {
    TissDB::Storage::LSMTree db;

    // 1. Define Schema with Primary Key
    TissDB::Schema user_schema;
    user_schema.add_field("user_id", TissDB::FieldType::String, true, true);
    user_schema.set_primary_key("user_id");
    db.create_collection("users", user_schema);

    // 2. Test successful insertion
    TissDB::Document doc1 = create_doc("doc1", "user_id", "user1");
    bool success = false;
    try {
        db.put("users", "doc1", doc1);
        success = true;
    } catch (const std::exception& e) {
        // Should not throw
    }
    ASSERT_TRUE(success);

    // 3. Test insertion with duplicate primary key (should fail)
    TissDB::Document doc2 = create_doc("doc2", "user_id", "user1");
    bool exception_thrown = false;
    try {
        db.put("users", "doc2", doc2);
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        std::string error_msg = e.what();
        ASSERT_TRUE(error_msg.find("Primary key constraint violated") != std::string::npos);
    }
    ASSERT_TRUE(exception_thrown);

    // 4. Test insertion with missing primary key (should fail)
    TissDB::Document doc3;
    doc3.id = "doc3";
    exception_thrown = false;
    try {
        db.put("users", "doc3", doc3);
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        std::string error_msg = e.what();
        ASSERT_TRUE(error_msg.find("Primary key field 'user_id' is missing") != std::string::npos);
    }
    ASSERT_TRUE(exception_thrown);
}

TEST_CASE(ForeignKeyConstraint) {
    TissDB::Storage::LSMTree db;

    // 1. Create referenced collection (users)
    TissDB::Schema user_schema;
    user_schema.add_field("id", TissDB::FieldType::String, true, true);
    user_schema.set_primary_key("id");
    db.create_collection("users", user_schema);

    // 2. Create referencing collection (orders)
    TissDB::Schema order_schema;
    order_schema.add_field("order_id", TissDB::FieldType::String, true, true);
    order_schema.set_primary_key("order_id");
    order_schema.add_field("user_id", TissDB::FieldType::String, false, false);
    order_schema.add_foreign_key("user_id", "users", "id");
    db.create_collection("orders", order_schema);

    // 3. Insert a user
    TissDB::Document user_doc = create_doc("u1", "id", "user123");
    db.put("users", "u1", user_doc);

    // 4. Test FK success: Insert an order with a valid user_id
    TissDB::Document order_doc1 = create_doc("o1", "user_id", "user123");
    bool success = false;
    try {
        db.put("orders", "o1", order_doc1);
        success = true;
    } catch (const std::exception& e) {
        // Should not throw
    }
    ASSERT_TRUE(success);

    // 5. Test FK failure: Insert an order with an invalid user_id
    TissDB::Document order_doc2 = create_doc("o2", "user_id", "user456_invalid");
    bool exception_thrown = false;
    try {
        db.put("orders", "o2", order_doc2);
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        std::string error_msg = e.what();
        ASSERT_TRUE(error_msg.find("Foreign key constraint violated") != std::string::npos);
    }
    ASSERT_TRUE(exception_thrown);

    // 6. Test nullable FK: Insert an order with no user_id (should succeed)
    TissDB::Document order_doc3;
    order_doc3.id = "o3";
    success = false;
    try {
        db.put("orders", "o3", order_doc3);
        success = true;
    } catch (const std::exception& e) {
        // should not throw
    }
    ASSERT_TRUE(success);
}
