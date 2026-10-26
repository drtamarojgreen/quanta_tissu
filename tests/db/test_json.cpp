#include "test_framework.h"
#include "../../tissdb/json/json.h"
#include <string>

TEST_CASE(JsonParsingAndAccessors) {
    std::string json_str = R"({
        "name": "TissDB",
        "version": 1.0,
        "is_beta": true,
        "features": null
    })";

    TissDB::Json::JsonValue parsed_json = TissDB::Json::JsonValue::parse(json_str);

    ASSERT_TRUE(parsed_json.is_object());

    const auto& obj = parsed_json.as_object();

    ASSERT_TRUE(obj.at("name").is_string());
    ASSERT_EQ(obj.at("name").as_string(), "TissDB");

    ASSERT_TRUE(obj.at("version").is_number());
    ASSERT_EQ(obj.at("version").as_number(), 1.0);

    ASSERT_TRUE(obj.at("is_beta").is_bool());
    ASSERT_EQ(obj.at("is_beta").as_bool(), true);

    ASSERT_TRUE(obj.at("features").is_null());
}

TEST_CASE(JsonNestedObjectAndArray) {
    std::string json_str = R"({
        "user": {
            "name": "Jules",
            "roles": ["admin", "developer"]
        }
    })";

    TissDB::Json::JsonValue parsed_json = TissDB::Json::JsonValue::parse(json_str);

    ASSERT_TRUE(parsed_json.is_object());
    const auto& root = parsed_json.as_object();

    ASSERT_TRUE(root.at("user").is_object());
    const auto& user = root.at("user").as_object();
    ASSERT_EQ(user.at("name").as_string(), "Jules");

    ASSERT_TRUE(user.at("roles").is_array());
    const auto& roles = user.at("roles").as_array();
    ASSERT_EQ(roles.size(), 2);
    ASSERT_EQ(roles[0].as_string(), "admin");
    ASSERT_EQ(roles[1].as_string(), "developer");
}

TEST_CASE(JsonSerialization) {
    TissDB::Json::JsonObject obj;
    obj["key"] = TissDB::Json::JsonValue("value");
    obj["number"] = TissDB::Json::JsonValue(42.0);

    TissDB::Json::JsonValue json_val(obj);
    std::string serialized_str = json_val.serialize();

    // Note: JSON object key order is not guaranteed.
    // A robust test would parse the string back and check for equivalence.
    TissDB::Json::JsonValue reparsed_json = TissDB::Json::JsonValue::parse(serialized_str);
    ASSERT_TRUE(reparsed_json.is_object());
    const auto& reparsed_obj = reparsed_json.as_object();
    ASSERT_EQ(reparsed_obj.at("key").as_string(), "value");
    ASSERT_EQ(reparsed_obj.at("number").as_number(), 42.0);
}

TEST_CASE(JsonSerializationEscapesStrings) {
    // Values such as SPARQL results in N-Triples form contain quotes and must stay valid JSON.
    const std::string literal = "\"34\"^^<http://www.w3.org/2001/XMLSchema#integer>";
    const std::string tricky = "back\\slash \"quoted\"\nnew line\ttab\x01";
    TissDB::Json::JsonObject obj;
    obj["age"] = TissDB::Json::JsonValue(literal);
    obj["text"] = TissDB::Json::JsonValue(tricky);
    obj["key \"with\" quotes"] = TissDB::Json::JsonValue(true);

    std::string serialized = TissDB::Json::JsonValue(obj).serialize();
    ASSERT_TRUE(serialized.find("\"age\":\"\\\"34\\\"^^<http://www.w3.org/2001/XMLSchema#integer>\"") != std::string::npos);
    ASSERT_TRUE(serialized.find("\\u0001") != std::string::npos);

    TissDB::Json::JsonValue reparsed = TissDB::Json::JsonValue::parse(serialized);
    const auto& reparsed_obj = reparsed.as_object();
    ASSERT_EQ(literal, reparsed_obj.at("age").as_string());
    ASSERT_EQ(tricky, reparsed_obj.at("text").as_string());
    ASSERT_TRUE(reparsed_obj.at("key \"with\" quotes").as_bool());
}

TEST_CASE(JsonParsesUnicodeEscapes) {
    TissDB::Json::JsonValue parsed = TissDB::Json::JsonValue::parse("{\"s\": \"caf\\u00e9 \\ud83d\\ude00 \\u0041\"}");
    ASSERT_EQ(std::string("caf\xC3\xA9 \xF0\x9F\x98\x80 A"), parsed.as_object().at("s").as_string());
    bool rejected = false;
    try {
        TissDB::Json::JsonValue::parse("\"\\u12\"");
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    ASSERT_TRUE(rejected);
}
