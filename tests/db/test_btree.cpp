
#include "test_main.cpp"
#include "../../../tissdb/storage/btree.h"
#include <filesystem>

TEST_CASE(BTreeInsertAndFind) {
    TissDB::Storage::BTree btree;

    btree.insert("key1", "doc1");
    btree.insert("key2", "doc2");
    btree.insert("key3", "doc3");

    ASSERT_EQ(1, btree.find("key1").size());
    ASSERT_EQ("doc1", btree.find("key1")[0]);

    ASSERT_EQ(1, btree.find("key2").size());
    ASSERT_EQ("doc2", btree.find("key2")[0]);

    ASSERT_EQ(1, btree.find("key3").size());
    ASSERT_EQ("doc3", btree.find("key3")[0]);

    ASSERT_EQ(0, btree.find("non_existent").size());
}

TEST_CASE(BTreeRemove) {
    TissDB::Storage::BTree btree;

    btree.insert("key1", "doc1");
    btree.insert("key2", "doc2");
    btree.insert("key3", "doc3");

    ASSERT_EQ(1, btree.find("key2").size());

    btree.remove("key2");
    ASSERT_EQ(0, btree.find("key2").size());

    ASSERT_EQ(1, btree.find("key1").size());
    ASSERT_EQ(1, btree.find("key3").size());

    // Remove non-existent key
    btree.remove("non_existent");
    ASSERT_EQ(1, btree.find("key1").size());
}

TEST_CASE(BTreeSerialization) {
    std::string file_path = "test_btree.bin";
    if (std::filesystem::exists(file_path)) {
        std::filesystem::remove(file_path);
    }

    // Create and serialize a B-Tree
    {
        TissDB::Storage::BTree btree;
        btree.insert("apple", "doc_apple");
        btree.insert("banana", "doc_banana");
        btree.insert("cherry", "doc_cherry");

        std::ofstream ofs(file_path, std::ios::binary);
        btree.serialize(ofs);
    }

    // Deserialize and verify
    {
        TissDB::Storage::BTree btree;
        std::ifstream ifs(file_path, std::ios::binary);
        btree.deserialize(ifs);

        ASSERT_EQ(1, btree.find("apple").size());
        ASSERT_EQ("doc_apple", btree.find("apple")[0]);
        ASSERT_EQ(1, btree.find("banana").size());
        ASSERT_EQ("doc_banana", btree.find("banana")[0]);
        ASSERT_EQ(1, btree.find("cherry").size());
        ASSERT_EQ("doc_cherry", btree.find("cherry")[0]);
        ASSERT_EQ(0, btree.find("grape").size());
    }

    std::filesystem::remove(file_path);
}
