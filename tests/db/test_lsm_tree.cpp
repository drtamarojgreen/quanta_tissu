
#include "test_framework.h"
#include "../../tissdb/storage/lsm_tree.h"
#include "../../tissdb/common/document.h"
#include <filesystem>

TEST_CASE(LSMTreeCollectionManagement) {
    std::string data_dir = "lsm_tree_test_data";
    if (std::filesystem::exists(data_dir)) {
        std::filesystem::remove_all(data_dir);
    }

    TissDB::Storage::LSMTree lsm_tree(data_dir);

    lsm_tree.create_collection("users");
    lsm_tree.create_collection("products");

    std::vector<std::string> collections = lsm_tree.list_collections();
    ASSERT_EQ(2, collections.size());

    lsm_tree.delete_collection("users");
    collections = lsm_tree.list_collections();
    ASSERT_EQ(1, collections.size());
    ASSERT_EQ("products", collections[0]);

    lsm_tree.shutdown();
    std::filesystem::remove_all(data_dir);
}

TEST_CASE(LSMTreeDocumentOperations) {
    std::string data_dir = "lsm_tree_doc_ops_data";
    if (std::filesystem::exists(data_dir)) {
        std::filesystem::remove_all(data_dir);
    }

    TissDB::Storage::LSMTree lsm_tree(data_dir);
    lsm_tree.create_collection("test_col");

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    lsm_tree.put("test_col", "doc1", doc1);

    auto retrieved_doc_opt = lsm_tree.get("test_col", "doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_EQ("doc1", retrieved_doc_opt->id);

    lsm_tree.del("test_col", "doc1");
    retrieved_doc_opt = lsm_tree.get("test_col", "doc1");
    ASSERT_FALSE(retrieved_doc_opt.has_value());

    lsm_tree.shutdown();
    std::filesystem::remove_all(data_dir);
}

TEST_CASE(LSMTreeScan) {
    std::string data_dir = "lsm_tree_scan_data";
    if (std::filesystem::exists(data_dir)) {
        std::filesystem::remove_all(data_dir);
    }

    TissDB::Storage::LSMTree lsm_tree(data_dir);
    lsm_tree.create_collection("scan_col");

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    TissDB::Element elem2; elem2.key = "name"; elem2.value = std::string("Bob");
    doc2.elements.push_back(elem2);

    lsm_tree.put("scan_col", "doc1", doc1);
    lsm_tree.put("scan_col", "doc2", doc2);

    std::vector<TissDB::Document> docs = lsm_tree.scan("scan_col");
    ASSERT_EQ(2, docs.size());

    lsm_tree.shutdown();
    std::filesystem::remove_all(data_dir);
}

TEST_CASE(LSMTreeIndex) {
    std::string data_dir = "lsm_tree_index_data";
    if (std::filesystem::exists(data_dir)) {
        std::filesystem::remove_all(data_dir);
    }

    TissDB::Storage::LSMTree lsm_tree(data_dir);
    lsm_tree.create_collection("index_col");
    lsm_tree.create_index("index_col", "city");

    TissDB::Document doc1;
    doc1.id = "user1";
    TissDB::Element e1; e1.key = "city"; e1.value = std::string("New York");
    doc1.elements.push_back(e1);
    lsm_tree.put("index_col", "user1", doc1);

    TissDB::Document doc2;
    doc2.id = "user2";
    TissDB::Element e2; e2.key = "city"; e2.value = std::string("Los Angeles");
    doc2.elements.push_back(e2);
    lsm_tree.put("index_col", "user2", doc2);

    std::vector<std::string> results = lsm_tree.find_by_index("index_col", "city", "New York");
    ASSERT_EQ(1, results.size());
    ASSERT_EQ("user1", results[0]);

    lsm_tree.shutdown();
    std::filesystem::remove_all(data_dir);
}
