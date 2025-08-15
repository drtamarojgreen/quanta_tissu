#include "test_framework.h"
#include "../../tissdb/storage/collection.h"
#include "../../tissdb/common/document.h"
#include <filesystem>

TEST_CASE(CollectionBasicCRUD) {
    std::string collection_path = "test_collection_crud";
    if (std::filesystem::exists(collection_path)) {
        std::filesystem::remove_all(collection_path);
    }

    TissDB::Storage::Collection collection(collection_path);

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    collection.put("doc1", doc1);

    auto retrieved_doc_opt = collection.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_EQ("doc1", retrieved_doc_opt->id);
    ASSERT_EQ("Alice", std::get<std::string>(retrieved_doc_opt->elements[0].value));

    // Update
    TissDB::Document doc1_updated;
    doc1_updated.id = "doc1";
    TissDB::Element elem1_updated; elem1_updated.key = "name"; elem1_updated.value = std::string("Bob");
    doc1_updated.elements.push_back(elem1_updated);
    collection.put("doc1", doc1_updated);

    retrieved_doc_opt = collection.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_EQ("Bob", std::get<std::string>(retrieved_doc_opt->elements[0].value));

    // Delete
    collection.del("doc1");
    retrieved_doc_opt = collection.get("doc1");
    ASSERT_FALSE(retrieved_doc_opt.has_value());

    collection.shutdown();
    std::filesystem::remove_all(collection_path);
}

TEST_CASE(CollectionScan) {
    std::string collection_path = "test_collection_scan";
    if (std::filesystem::exists(collection_path)) {
        std::filesystem::remove_all(collection_path);
    }

    TissDB::Storage::Collection collection(collection_path);

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

    bool found_doc1 = false;
    bool found_doc2 = false;
    for (const auto& doc : docs) {
        if (doc.id == "doc1") found_doc1 = true;
        if (doc.id == "doc2") found_doc2 = true;
    }
    ASSERT_TRUE(found_doc1);
    ASSERT_TRUE(found_doc2);

    collection.shutdown();
    std::filesystem::remove_all(collection_path);
}

TEST_CASE(CollectionIndex) {
    std::string collection_path = "test_collection_index";
    if (std::filesystem::exists(collection_path)) {
        std::filesystem::remove_all(collection_path);
    }

    TissDB::Storage::Collection collection(collection_path);
    collection.create_index("city");

    TissDB::Document doc1;
    doc1.id = "user1";
    TissDB::Element e1; e1.key = "city"; e1.value = std::string("New York");
    doc1.elements.push_back(e1);
    collection.put("user1", doc1);

    TissDB::Document doc2;
    doc2.id = "user2";
    TissDB::Element e2; e2.key = "city"; e2.value = std::string("Los Angeles");
    doc2.elements.push_back(e2);
    collection.put("user2", doc2);

    std::vector<std::string> results = collection.find_by_index("city", "New York");
    ASSERT_EQ(1, results.size());
    ASSERT_EQ("user1", results[0]);

    collection.shutdown();
    std::filesystem::remove_all(collection_path);
}