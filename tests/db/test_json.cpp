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
