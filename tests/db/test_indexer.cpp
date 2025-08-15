
#include "test_framework.h"
#include "../../tissdb/storage/indexer.h"
#include "../../tissdb/common/document.h"
#include <filesystem>

TEST_CASE(IndexerCreateIndex) {
    TissDB::Storage::Indexer indexer;
    indexer.create_index("name");
    ASSERT_TRUE(indexer.has_index("name"));
    ASSERT_FALSE(indexer.has_index("age"));
}

TEST_CASE(IndexerUpdateAndFind) {
    TissDB::Storage::Indexer indexer;
    indexer.create_index("name");

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);
    indexer.update_indexes("doc1", doc1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    TissDB::Element elem2; elem2.key = "name"; elem2.value = std::string("Bob");
    doc2.elements.push_back(elem2);
    indexer.update_indexes("doc2", doc2);

    TissDB::Document doc3;
    doc3.id = "doc3";
    TissDB::Element elem3; elem3.key = "name"; elem3.value = std::string("Alice");
    doc3.elements.push_back(elem3);
    indexer.update_indexes("doc3", doc3);

    std::vector<std::string> results = indexer.find_by_index("name", "Alice");
    ASSERT_EQ(2, results.size());
    // Order might not be guaranteed, so check for presence
    bool found_doc1 = false;
    bool found_doc3 = false;
    for (const auto& id : results) {
        if (id == "doc1") found_doc1 = true;
        if (id == "doc3") found_doc3 = true;
    }
    ASSERT_TRUE(found_doc1);
    ASSERT_TRUE(found_doc3);

    results = indexer.find_by_index("name", "Bob");
    ASSERT_EQ(1, results.size());
    ASSERT_EQ("doc2", results[0]);

    results = indexer.find_by_index("name", "Charlie");
    ASSERT_EQ(0, results.size());
}

TEST_CASE(IndexerRemoveFromIndexes) {
    TissDB::Storage::Indexer indexer;
    indexer.create_index("name");

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);
    indexer.update_indexes("doc1", doc1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    TissDB::Element elem2; elem2.key = "name"; elem2.value = std::string("Bob");
    doc2.elements.push_back(elem2);
    indexer.update_indexes("doc2", doc2);

    ASSERT_EQ(1, indexer.find_by_index("name", "Bob").size());

    indexer.remove_from_indexes("doc2", doc2);
    ASSERT_EQ(0, indexer.find_by_index("name", "Bob").size());
    ASSERT_EQ(1, indexer.find_by_index("name", "Alice").size());
}

TEST_CASE(IndexerSaveLoad) {
    std::string data_dir = "indexer_test_data";
    std::filesystem::create_directories(data_dir);

    // Save indexes
    {
        TissDB::Storage::Indexer indexer;
        indexer.create_index("city");
        indexer.create_index("zip");

        TissDB::Document doc1;
        doc1.id = "user1";
        TissDB::Element e1; e1.key = "city"; e1.value = std::string("New York");
        TissDB::Element e2; e2.key = "zip"; e2.value = std::string("10001");
        doc1.elements.push_back(e1); doc1.elements.push_back(e2);
        indexer.update_indexes("user1", doc1);

        TissDB::Document doc2;
        doc2.id = "user2";
        TissDB::Element e3; e3.key = "city"; e3.value = std::string("Los Angeles");
        TissDB::Element e4; e4.key = "zip"; e4.value = std::string("90001");
        doc2.elements.push_back(e3); doc2.elements.push_back(e4);
        indexer.update_indexes("user2", doc2);

        indexer.save_indexes(data_dir);
    }

    // Load indexes and verify
    {
        TissDB::Storage::Indexer indexer;
        indexer.load_indexes(data_dir);

        ASSERT_TRUE(indexer.has_index("city"));
        ASSERT_TRUE(indexer.has_index("zip"));
        ASSERT_FALSE(indexer.has_index("country"));

        std::vector<std::string> results = indexer.find_by_index("city", "New York");
        ASSERT_EQ(1, results.size());
        ASSERT_EQ("user1", results[0]);

        results = indexer.find_by_index("zip", "90001");
        ASSERT_EQ(1, results.size());
        ASSERT_EQ("user2", results[0]);
    }

    std::filesystem::remove_all(data_dir);
}

TEST_CASE(IndexerCompoundIndex) {
    TissDB::Storage::Indexer indexer;
    indexer.create_index({"city", "state"});

    TissDB::Document doc1;
    doc1.id = "doc1";
    doc1.elements.push_back({"city", "New York"});
    doc1.elements.push_back({"state", "NY"});
    indexer.update_indexes("doc1", doc1);

    TissDB::Document doc2;
    doc2.id = "doc2";
    doc2.elements.push_back({"city", "New York"});
    doc2.elements.push_back({"state", "CA"}); // Should not be found
    indexer.update_indexes("doc2", doc2);

    TissDB::Document doc3;
    doc3.id = "doc3";
    doc3.elements.push_back({"city", "Los Angeles"});
    doc3.elements.push_back({"state", "CA"});
    indexer.update_indexes("doc3", doc3);

    // Find with correct compound key
    std::vector<std::string> results = indexer.find_by_index({"city", "state"}, {"New York", "NY"});
    ASSERT_EQ(1, results.size());
    ASSERT_EQ("doc1", results[0]);

    // Find with different compound key
    results = indexer.find_by_index({"city", "state"}, {"Los Angeles", "CA"});
    ASSERT_EQ(1, results.size());
    ASSERT_EQ("doc3", results[0]);

    // Attempt to find with non-existent key
    results = indexer.find_by_index({"city", "state"}, {"New York", "FL"});
    ASSERT_EQ(0, results.size());

    // Attempt to find with a non-existent index (wrong order)
    results = indexer.find_by_index({"state", "city"}, {"NY", "New York"});
    ASSERT_EQ(0, results.size());
}
