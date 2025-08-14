

#include "test_framework.h"
#include "../../../tissdb/storage/bpp_tree.h"
#include <filesystem>

TEST_CASE(BppTreeInsertAndFind) {
    bpp::btree<std::string, std::string> btree;

    btree.insert("key1", "doc1");
    btree.insert("key2", "doc2");
    btree.insert("key3", "doc3");

    auto result1 = btree.find("key1");
    ASSERT_TRUE(result1);
    ASSERT_EQ("doc1", *result1);

    auto result2 = btree.find("key2");
    ASSERT_TRUE(result2);
    ASSERT_EQ("doc2", *result2);

    auto result3 = btree.find("key3");
    ASSERT_TRUE(result3);
    ASSERT_EQ("doc3", *result3);

    auto result4 = btree.find("non_existent");
    ASSERT_FALSE(result4);
}

TEST_CASE(BppTreeRemove) {
    bpp::btree<std::string, std::string> btree;

    btree.insert("key1", "doc1");
    btree.insert("key2", "doc2");
    btree.insert("key3", "doc3");

    auto result1 = btree.find("key2");
    ASSERT_TRUE(result1);

    btree.erase("key2");
    auto result2 = btree.find("key2");
    ASSERT_FALSE(result2);

    auto result3 = btree.find("key1");
    ASSERT_TRUE(result3);
    auto result4 = btree.find("key3");
    ASSERT_TRUE(result4);

    // Remove non-existent key
    btree.erase("non_existent");
    auto result5 = btree.find("key1");
    ASSERT_TRUE(result5);
}

TEST_CASE(BppTreeSerialization) {
    std::string file_path = "test_bpp_tree.bin";
    if (std::filesystem::exists(file_path)) {
        std::filesystem::remove(file_path);
    }

    // Create and serialize a B-Tree
    {
        bpp::btree<std::string, std::string> btree;
        btree.insert("apple", "doc_apple");
        btree.insert("banana", "doc_banana");
        btree.insert("cherry", "doc_cherry");

        std::ofstream ofs(file_path, std::ios::binary);
        btree.dump(ofs);
    }

    // Deserialize and verify
    {
        bpp::btree<std::string, std::string> btree;
        std::ifstream ifs(file_path, std::ios::binary);
        btree.load(ifs);

        auto result1 = btree.find("apple");
        ASSERT_TRUE(result1);
        ASSERT_EQ("doc_apple", *result1);

        auto result2 = btree.find("banana");
        ASSERT_TRUE(result2);
        ASSERT_EQ("doc_banana", *result2);

        auto result3 = btree.find("cherry");
        ASSERT_TRUE(result3);
        ASSERT_EQ("doc_cherry", *result3);

        auto result4 = btree.find("grape");
        ASSERT_FALSE(result4);
    }

    std::filesystem::remove(file_path);
}

