
#include "test_framework.h"
#include "../../tissdb/storage/memtable.h"
#include "../../tissdb/common/document.h"

TEST_CASE(MemtablePutGet) {
    TissDB::Storage::Memtable memtable;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    memtable.put("doc1", doc1);

    auto retrieved_doc_opt = memtable.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() != nullptr);
    ASSERT_EQ("doc1", retrieved_doc_opt.value()->id);
    ASSERT_EQ("Alice", std::get<std::string>(retrieved_doc_opt.value()->elements[0].value));

    // Update document
    TissDB::Document doc1_updated;
    doc1_updated.id = "doc1";
    TissDB::Element elem1_updated; elem1_updated.key = "name"; elem1_updated.value = std::string("Bob");
    doc1_updated.elements.push_back(elem1_updated);
    memtable.put("doc1", doc1_updated);

    retrieved_doc_opt = memtable.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() != nullptr);
    ASSERT_EQ("Bob", std::get<std::string>(retrieved_doc_opt.value()->elements[0].value));
}

TEST_CASE(MemtableDelete) {
    TissDB::Storage::Memtable memtable;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    memtable.put("doc1", doc1);
    ASSERT_TRUE(memtable.get("doc1").has_value());

    memtable.del("doc1");
    auto retrieved_doc_opt = memtable.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() == nullptr); // Should be a tombstone

    // Try to get a non-existent doc
    ASSERT_FALSE(memtable.get("non_existent").has_value());
}

TEST_CASE(MemtableScan) {
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

    std::vector<TissDB::Document> docs = memtable.scan();
    ASSERT_EQ(2, docs.size());

    // Check for presence of docs (order might not be guaranteed by scan, but map is sorted)
    bool found_doc1 = false;
    bool found_doc2 = false;
    for (const auto& doc : docs) {
        if (doc.id == "doc1") found_doc1 = true;
        if (doc.id == "doc2") found_doc2 = true;
    }
    ASSERT_TRUE(found_doc1);
    ASSERT_TRUE(found_doc2);

    // Test with a tombstone
    memtable.del("doc1");
    docs = memtable.scan();
    ASSERT_EQ(2, docs.size()); // Tombstone is still returned by scan

    found_doc1 = false;
    found_doc2 = false;
    for (const auto& doc : docs) {
        if (doc.id == "doc1" && doc.elements.empty()) found_doc1 = true; // Check for tombstone
        if (doc.id == "doc2") found_doc2 = true;
    }
    ASSERT_TRUE(found_doc1);
    ASSERT_TRUE(found_doc2);
}
