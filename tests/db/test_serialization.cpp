#include "test_framework.h"
#include "../../tissdb/common/document.h"
#include "../../tissdb/common/serialization.h"
#include <vector>
#include <string>

// Helper function to compare documents
bool are_documents_equal(const TissDB::Document& doc1, const TissDB::Document& doc2) {
    if (doc1.id != doc2.id) {
        return false;
    }
    if (doc1.elements.size() != doc2.elements.size()) {
        return false;
    }
    // Note: This comparison is simple and assumes element order is the same.
    // For a more robust comparison, we might need to ignore order or sort elements first.
    for (size_t i = 0; i < doc1.elements.size(); ++i) {
        if (doc1.elements[i].key != doc2.elements[i].key) {
            return false;
        }
        if (doc1.elements[i].value != doc2.elements[i].value) {
            return false;
        }
    }
    return true;
}

TEST_CASE(SerializationRoundtrip) {
    // 1. Create a complex document
    TissDB::Document original_doc;
    original_doc.id = "doc-123";

    TissDB::Element e1;
    e1.key = "author";
    e1.value = std::string("Jules");
    original_doc.elements.push_back(e1);

    TissDB::Element e2;
    e2.key = "version";
    e2.value = TissDB::Number(1.23);
    original_doc.elements.push_back(e2);

    TissDB::Element e3;
    e3.key = "published";
    e3.value = TissDB::Boolean(true);
    original_doc.elements.push_back(e3);

    TissDB::Element e4;
    e4.key = "data";
    e4.value = TissDB::BinaryData{0xDE, 0xAD, 0xBE, 0xEF};
    original_doc.elements.push_back(e4);

    // 2. Serialize the document
    std::vector<uint8_t> serialized_data = TissDB::serialize(original_doc);

    // 3. Deserialize the data
    TissDB::Document deserialized_doc = TissDB::deserialize(serialized_data);

    // 4. Verify the documents are identical
    ASSERT_EQ(original_doc.id, deserialized_doc.id);
    ASSERT_EQ(original_doc.elements.size(), deserialized_doc.elements.size());
    ASSERT_TRUE(are_documents_equal(original_doc, deserialized_doc));

    // Deeper checks for individual elements
    ASSERT_EQ(std::get<std::string>(deserialized_doc.elements[0].value), "Jules");
    ASSERT_EQ(std::get<TissDB::Number>(deserialized_doc.elements[1].value), 1.23);
    ASSERT_EQ(std::get<TissDB::Boolean>(deserialized_doc.elements[2].value), true);
    TissDB::BinaryData expected_data = {0xDE, 0xAD, 0xBE, 0xEF};
    ASSERT_EQ(std::get<TissDB::BinaryData>(deserialized_doc.elements[3].value), expected_data);
}
