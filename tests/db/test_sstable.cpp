
#include "test_framework.h"
#include "../../tissdb/storage/sstable.h"
#include "../../tissdb/storage/memtable.h"
#include "../../tissdb/common/document.h"
#include <filesystem>

TEST_CASE(SSTableWriteAndFind) {
    std::string data_dir = "sstable_test_data";
    std::filesystem::create_directories(data_dir);

    TissDB::Storage::Memtable memtable;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    TissDB::Element elem2; elem2.key = "name"; elem2.value = std::string("Bob");
    doc2.elements.push_back(elem2);

    memtable.put("doc1", doc1);
    memtable.put("doc2", doc2);

    std::string sstable_path = TissDB::Storage::SSTable::write_from_memtable(data_dir, memtable);

    TissDB::Storage::SSTable sstable(sstable_path);

    auto retrieved_doc_opt = sstable.find("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_EQ("doc1", TissDB::deserialize(*retrieved_doc_opt).id);

    retrieved_doc_opt = sstable.find("doc2");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_EQ("doc2", TissDB::deserialize(*retrieved_doc_opt).id);

    ASSERT_FALSE(sstable.find("non_existent").has_value());

    std::filesystem::remove_all(data_dir);
}

TEST_CASE(SSTableFindWithIndex) {
    std::string data_dir = "sstable_index_test_data";
    std::filesystem::create_directories(data_dir);

    TissDB::Storage::Memtable memtable;
    const int num_docs = 50;

    for (int i = 0; i < num_docs; ++i) {
        TissDB::Document doc;
        // Pad with zeros to ensure lexicographical order matches numerical order
        doc.id = "doc" + std::string(2 - std::to_string(i).length(), '0') + std::to_string(i);
        TissDB::Element elem;
        elem.key = "value";
        elem.value = std::string("data" + std::to_string(i));
        doc.elements.push_back(elem);
        memtable.put(doc.id, doc);
    }

    std::string sstable_path = TissDB::Storage::SSTable::write_from_memtable(data_dir, memtable);
    TissDB::Storage::SSTable sstable(sstable_path);

    // Test finding all existing keys
    for (int i = 0; i < num_docs; ++i) {
        std::string key = "doc" + std::string(2 - std::to_string(i).length(), '0') + std::to_string(i);
        auto result = sstable.find(key);
        ASSERT_TRUE(result.has_value());
        TissDB::Document retrieved_doc = TissDB::deserialize(*result);
        ASSERT_EQ(key, retrieved_doc.id);
        ASSERT_EQ("data" + std::to_string(i), std::get<std::string>(retrieved_doc.elements[0].value));
    }

    // Test finding non-existent keys
    ASSERT_FALSE(sstable.find("doc_non_existent").has_value());
    ASSERT_FALSE(sstable.find("a_before_all").has_value()); // Before first key
    ASSERT_FALSE(sstable.find("doc25_between").has_value()); // Between keys
    ASSERT_FALSE(sstable.find("z_after_all").has_value()); // After last key

    std::filesystem::remove_all(data_dir);
}

TEST_CASE(SSTableTombstone) {
    std::string data_dir = "sstable_tombstone_test_data";
    std::filesystem::create_directories(data_dir);

    TissDB::Storage::Memtable memtable;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    memtable.put("doc1", doc1);
    memtable.del("doc1"); // Add tombstone

    std::string sstable_path = TissDB::Storage::SSTable::write_from_memtable(data_dir, memtable);

    TissDB::Storage::SSTable sstable(sstable_path);

    auto retrieved_doc_opt = sstable.find("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt->empty()); // Should be an empty vector for tombstone

    std::filesystem::remove_all(data_dir);
}

TEST_CASE(SSTableScan) {
    std::string data_dir = "sstable_scan_test_data";
    std::filesystem::create_directories(data_dir);

    TissDB::Storage::Memtable memtable;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    TissDB::Element elem2; elem2.key = "name"; elem2.value = std::string("Bob");
    doc2.elements.push_back(elem2);

    memtable.put("doc1", doc1);
    memtable.put("doc2", doc2);

    std::string sstable_path = TissDB::Storage::SSTable::write_from_memtable(data_dir, memtable);

    TissDB::Storage::SSTable sstable(sstable_path);
    std::vector<TissDB::Document> docs = sstable.scan();

    ASSERT_EQ(2, docs.size());
    bool found_doc1 = false;
    bool found_doc2 = false;
    for (const auto& doc : docs) {
        if (doc.id == "doc1") found_doc1 = true;
        if (doc.id == "doc2") found_doc2 = true;
    }
    ASSERT_TRUE(found_doc1);
    ASSERT_TRUE(found_doc2);

    std::filesystem::remove_all(data_dir);
}
