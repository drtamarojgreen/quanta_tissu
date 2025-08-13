
#include "test_main.cpp"
#include "../../../tissdb/common/document.h"
#include "../../../tissdb/common/serialization.h"
#include <chrono>

TEST_CASE(DocumentSerialization) {
    TissDB::Document doc;
    doc.id = "test_doc_1";

    TissDB::Element elem1;
    elem1.key = "name";
    elem1.value = std::string("Test Document");
    doc.elements.push_back(elem1);

    TissDB::Element elem2;
    elem2.key = "version";
    elem2.value = 1.0;
    doc.elements.push_back(elem2);

    TissDB::Element elem3;
    elem3.key = "is_active";
    elem3.value = true;
    doc.elements.push_back(elem3);

    TissDB::Element elem4;
    elem4.key = "created_at";
    elem4.value = std::chrono::system_clock::now();
    doc.elements.push_back(elem4);

    TissDB::Element elem5;
    elem5.key = "binary_data";
    TissDB::BinaryData bin_data = {0x01, 0x02, 0x03, 0x04};
    elem5.value = bin_data;
    doc.elements.push_back(elem5);

    TissDB::Element nested_elem;
    nested_elem.key = "specs";
    TissDB::Element nested_child1;
    nested_child1.key = "weight";
    nested_child1.value = std::string("100g");
    TissDB::Element nested_child2;
    nested_child2.key = "color";
    nested_child2.value = std::string("red");
    nested_elem.value = std::vector<TissDB::Element>{nested_child1, nested_child2};
    doc.elements.push_back(nested_elem);

    std::vector<uint8_t> serialized_doc = TissDB::serialize(doc);
    TissDB::Document deserialized_doc = TissDB::deserialize(serialized_doc);

    ASSERT_EQ(doc.id, deserialized_doc.id);
    ASSERT_EQ(doc.elements.size(), deserialized_doc.elements.size());

    // Verify elements
    ASSERT_EQ(std::get<std::string>(doc.elements[0].value), std::get<std::string>(deserialized_doc.elements[0].value));
    ASSERT_EQ(std::get<double>(doc.elements[1].value), std::get<double>(deserialized_doc.elements[1].value));
    ASSERT_EQ(std::get<bool>(doc.elements[2].value), std::get<bool>(deserialized_doc.elements[2].value));
    ASSERT_EQ(std::get<std::chrono::system_clock::time_point>(doc.elements[3].value), std::get<std::chrono::system_clock::time_point>(deserialized_doc.elements[3].value));
    ASSERT_EQ(std::get<TissDB::BinaryData>(doc.elements[4].value), std::get<TissDB::BinaryData>(deserialized_doc.elements[4].value));

    // Verify nested elements
    auto original_nested_elements = std::get<std::vector<TissDB::Element>>(doc.elements[5].value);
    auto deserialized_nested_elements = std::get<std::vector<TissDB::Element>>(deserialized_doc.elements[5].value);
    ASSERT_EQ(original_nested_elements.size(), deserialized_nested_elements.size());
    ASSERT_EQ(std::get<std::string>(original_nested_elements[0].value), std::get<std::string>(deserialized_nested_elements[0].value));
    ASSERT_EQ(std::get<std::string>(original_nested_elements[1].value), std::get<std::string>(deserialized_nested_elements[1].value));
}
