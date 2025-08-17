#include "test_framework.h"
#include "../../tissdb/storage/memtable.h"
#include "../../tissdb/common/document.h"
#include "../../tissdb/common/serialization.h"

TEST_CASE(MemtableSizeAndIsFull) {
    TissDB::Storage::Memtable memtable(1024); // Set a small max size for testing

    ASSERT_EQ(0, memtable.approximate_size());
    ASSERT_FALSE(memtable.is_full());

    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "key1"; elem1.value = std::string("value1");
    doc1.elements.push_back(elem1);

    memtable.put("doc1", doc1);
    size_t doc1_size = TissDB::serialize(doc1).size();
    ASSERT_EQ(doc1.id.length() + doc1_size, memtable.approximate_size());
    ASSERT_FALSE(memtable.is_full());

    // Fill up the memtable
    for (int i = 0; i < 100; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string val = "val" + std::to_string(i);
        TissDB::Document doc;
        doc.id = key;
        TissDB::Element elem; elem.key = "k"; elem.value = val;
        doc.elements.push_back(elem);
        if (!memtable.is_full()) {
            memtable.put(key, doc);
        } else {
            break;
        }
    }

    ASSERT_TRUE(memtable.approximate_size() > 0);
    ASSERT_TRUE(memtable.is_full());
}

TEST_CASE(MemtableScanEdgeCases) {
    // Test scanning an empty memtable
    {
        TissDB::Storage::Memtable memtable;
        std::vector<TissDB::Document> docs = memtable.scan();
        ASSERT_EQ(0, docs.size());
    }

    // Test scanning a memtable with only tombstones
    {
        TissDB::Storage::Memtable memtable;
        TissDB::Document doc1;
        doc1.id = "doc1";
        TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
        doc1.elements.push_back(elem1);
        memtable.put("doc1", doc1);
        memtable.del("doc1");

        std::vector<TissDB::Document> docs = memtable.scan();
        ASSERT_EQ(1, docs.size());
        ASSERT_TRUE(docs[0].elements.empty()); // Check for tombstone
    }
}
