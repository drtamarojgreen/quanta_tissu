#include "test_framework.h"
#include "../../tissdb/storage/collection.h"
#include "../../tissdb/common/document.h"

TEST_CASE(CollectionPutGet) {
    TissDB::Storage::Collection collection;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    collection.put("doc1", doc1);

    auto retrieved_doc_opt = collection.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() != nullptr);
    ASSERT_EQ("doc1", retrieved_doc_opt.value()->id);
    ASSERT_EQ("Alice", std::get<std::string>(retrieved_doc_opt.value()->elements[0].value));

    // Update document
    TissDB::Document doc1_updated;
    doc1_updated.id = "doc1";
    TissDB::Element elem1_updated; elem1_updated.key = "name"; elem1_updated.value = std::string("Bob");
    doc1_updated.elements.push_back(elem1_updated);
    collection.put("doc1", doc1_updated);

    retrieved_doc_opt = collection.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() != nullptr);
    ASSERT_EQ("Bob", std::get<std::string>(retrieved_doc_opt.value()->elements[0].value));
}

TEST_CASE(CollectionDelete) {
    TissDB::Storage::Collection collection;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    collection.put("doc1", doc1);
    ASSERT_TRUE(collection.get("doc1").has_value());

    collection.del("doc1");
    auto retrieved_doc_opt = collection.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_TRUE(retrieved_doc_opt.value() == nullptr); // Should be a tombstone

    // Try to get a non-existent doc
    ASSERT_FALSE(collection.get("non_existent").has_value());
}

TEST_CASE(CollectionScan) {
    TissDB::Storage::Collection collection;

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    TissDB::Element elem2; elem2.key = "name"; elem2.value = std::string("Bob");
    doc2.elements.push_back(elem2);

    collection.put("doc1", doc1);
    collection.put("doc2", doc2);

    std::vector<TissDB::Document> docs = collection.scan();
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
    collection.del("doc1");
    docs = collection.scan();
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
