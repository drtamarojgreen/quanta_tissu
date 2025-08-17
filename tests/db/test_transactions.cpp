#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include <filesystem>

TEST_CASE(TransactionCommit) {
    std::string data_dir = "transaction_commit_test_data";
    std::filesystem::remove_all(data_dir); // Clean up previous runs
    
    // 1. Initialize DB and create collection
    TissDB::Storage::LSMTree db(data_dir);
    db.create_collection("users", {});
    db.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}}});
    db.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}}});

    // 2. Start a transaction
    auto tid = db.begin_transaction();

    // 3. Perform operations within the transaction
    db.put("users", "user3", TissDB::Document{"user3", {{"name", std::string("Charlie")}}}, tid);
    db.del("users", "user1", tid);

    // 4. Verify changes are NOT visible before commit
    auto res1 = db.get("users", "user3");
    ASSERT_FALSE(res1.has_value());
    auto res2 = db.get("users", "user1");
    ASSERT_TRUE(res2.has_value());

    // 5. Commit the transaction
    db.commit_transaction(tid);

    // 6. Verify changes ARE visible after commit
    auto res3 = db.get("users", "user3");
    ASSERT_TRUE(res3.has_value());
            ASSERT_EQ(std::get<std::string>((*res2)->elements[0].value), std::string("Alice"));



    auto res4 = db.get("users", "user1");
    ASSERT_FALSE(res4.has_value());

    std::filesystem::remove_all(data_dir);
}

TEST_CASE(TransactionRollback) {
    std::string data_dir = "transaction_rollback_test_data";
    std::filesystem::remove_all(data_dir);

    // 1. Initialize DB and create collection
    TissDB::Storage::LSMTree db(data_dir);
    db.create_collection("users", {});
    db.put("users", "user1", TissDB::Document{"user1", {{"name", std::string("Alice")}}});

    // 2. Start a transaction
    auto tid = db.begin_transaction();

    // 3. Perform operations
    db.put("users", "user2", TissDB::Document{"user2", {{"name", std::string("Bob")}}}, tid);
    db.del("users", "user1", tid);

    // 4. Rollback the transaction
    db.rollback_transaction(tid);

    // 5. Verify the database state is unchanged
    auto res1 = db.get("users", "user2");
    ASSERT_FALSE(res1.has_value()); // Bob should not exist

    auto res2 = db.get("users", "user1");
    ASSERT_TRUE(res2.has_value()); // Alice should still exist
    ASSERT_EQ(std::get<std::string>(res2->elements[0].value), std::string("Alice"));

    std::filesystem::remove_all(data_dir);
}
