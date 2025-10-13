#include "test_framework.h"
#include "../../tissdb/common/document.h"
#include "../../tissdb/common/serialization.h"
#include "../../tissdb/query/parser.h"

#include <optional>

// Forward declaration of the internal helper function for testing
namespace TissDB {
namespace Query {
    std::optional<Timestamp> try_parse_timestamp(const std::string& s);
}
}

TEST_CASE(TimestampComparison) {
    TissDB::Timestamp t1{1000};
    TissDB::Timestamp t2{2000};
    TissDB::Timestamp t3{1000};

    ASSERT_TRUE(t1 == t3);
    ASSERT_TRUE(t1 != t2);
    ASSERT_TRUE(t1 < t2);
    ASSERT_TRUE(t2 > t1);
    ASSERT_TRUE(t1 <= t3);
    ASSERT_TRUE(t1 <= t2);
    ASSERT_TRUE(t2 >= t1);
    ASSERT_TRUE(t2 >= t3);
}

TEST_CASE(TimestampParser) {
    // Valid ISO 8601 strings
    auto ts1 = TissDB::Query::try_parse_timestamp("2025-01-01T12:00:00Z");
    ASSERT_TRUE(ts1.has_value());

    auto ts2 = TissDB::Query::try_parse_timestamp("2024-12-31T23:59:59.999Z");
    ASSERT_TRUE(ts2.has_value());

    // Check a known value
    auto ts3 = TissDB::Query::try_parse_timestamp("1970-01-01T00:00:01Z");
    ASSERT_TRUE(ts3.has_value());
    ASSERT_EQ(ts3->microseconds_since_epoch_utc, 1000000);

    auto ts4 = TissDB::Query::try_parse_timestamp("1970-01-01T00:00:01.5Z");
    ASSERT_TRUE(ts4.has_value());
    ASSERT_EQ(ts4->microseconds_since_epoch_utc, 1500000);

    // Invalid strings
    auto ts_invalid1 = TissDB::Query::try_parse_timestamp("not-a-timestamp");
    ASSERT_FALSE(ts_invalid1.has_value());

    auto ts_invalid2 = TissDB::Query::try_parse_timestamp("2025-01-01 12:00:00"); // Missing T and Z
    ASSERT_FALSE(ts_invalid2.has_value());
}

TEST_CASE(TimestampSerialization) {
    TissDB::Document doc_in;
    doc_in.id = "ts_doc";

    TissDB::Element elem;
    elem.key = "event_time";
    TissDB::Timestamp ts_val{1234567890123456LL};
    elem.value = ts_val;
    doc_in.elements.push_back(elem);

    // Serialize
    auto serialized_data = TissDB::serialize(doc_in);

    // Deserialize
    TissDB::Document doc_out = TissDB::deserialize(serialized_data);

    // Verify
    ASSERT_EQ(doc_in.id, doc_out.id);
    ASSERT_EQ(doc_in.elements.size(), doc_out.elements.size());

    auto& elem_out = doc_out.elements[0];
    ASSERT_EQ(elem.key, elem_out.key);
    ASSERT_TRUE(std::holds_alternative<TissDB::Timestamp>(elem_out.value));

    auto& ts_out = std::get<TissDB::Timestamp>(elem_out.value);
    ASSERT_EQ(ts_val.microseconds_since_epoch_utc, ts_out.microseconds_since_epoch_utc);
}
