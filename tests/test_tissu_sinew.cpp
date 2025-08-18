#include "db/test_framework.h"
#include "quanta_tissu/tissu_sinew.h"
#include <vector>
#include <string>
#include <map>

#include <arpa/inet.h> // For htonl

// Helper functions to build expected binary buffers for tests
static void append_uint32_be(std::vector<char>& buf, uint32_t val) {
    val = htonl(val);
    char* p = (char*)&val;
    buf.insert(buf.end(), p, p + sizeof(uint32_t));
}

static void append_uint64_be(std::vector<char>& buf, uint64_t val) {
    uint32_t high = htonl(val >> 32);
    uint32_t low = htonl(val & 0xFFFFFFFF);
    char* p_high = (char*)&high;
    char* p_low = (char*)&low;
    buf.insert(buf.end(), p_high, p_high + sizeof(uint32_t));
    buf.insert(buf.end(), p_low, p_low + sizeof(uint32_t));
}

class MockSession : public tissudb::TissuSession {
public:
    MockSession() : tissudb::TissuSession(-1, nullptr) {} // -1 sockfd, null impl

    std::vector<char> captured_send_buffer;

protected:
    std::unique_ptr<tissudb::TissuResult> send_and_receive_raw(const std::vector<char>& message_buffer) override {
        captured_send_buffer = message_buffer;
        return std::make_unique<tissudb::TissuResult>("mock_response");
    }
};


// Helper to extract a simple string query from the raw buffer for transaction tests
std::string extract_simple_query(const std::vector<char>& buffer) {
    if (buffer.size() < 4) return "";
    uint32_t len;
    memcpy(&len, buffer.data(), sizeof(uint32_t));
    len = ntohl(len);
    if (buffer.size() != 4 + len) return "[invalid buffer size]";
    return std::string(buffer.begin() + 4, buffer.end());
}

TEST_CASE(Transaction_Commit) {
    MockSession mock_session;
    {
        auto tx = mock_session.beginTransaction();
        // The first call is BEGIN. We don't need to check it here again.
        tx->commit();
    }
    // The mock captures the LAST buffer sent. In this case, "COMMIT".
    ASSERT_EQ("COMMIT", extract_simple_query(mock_session.captured_send_buffer));
}

TEST_CASE(Transaction_Rollback) {
    MockSession mock_session;
    {
        auto tx = mock_session.beginTransaction();
        tx->rollback();
    }
    ASSERT_EQ("ROLLBACK", extract_simple_query(mock_session.captured_send_buffer));
}

TEST_CASE(Transaction_AutoRollbackOnDestruction) {
    MockSession mock_session;
    {
        auto tx = mock_session.beginTransaction();
        // tx goes out of scope without commit or rollback
    }
    ASSERT_EQ("ROLLBACK", extract_simple_query(mock_session.captured_send_buffer));
}

TEST_CASE(SecureParameterizedQuery_Serialization) {
    MockSession mock_session;
    std::string query = "SELECT * FROM users WHERE name = $1 AND age = $2";
    std::vector<tissudb::TissValue> params;
    params.emplace_back("test_user");
    params.emplace_back((int64_t)42);

    mock_session.run(query, params);
    const auto& actual_buffer = mock_session.captured_send_buffer;

    // Manually construct the expected buffer
    std::vector<char> expected_body;
    // 1. Query
    append_uint32_be(expected_body, query.length());
    expected_body.insert(expected_body.end(), query.begin(), query.end());
    // 2. Param count
    expected_body.push_back(2);
    // 3. Param 1 (string)
    expected_body.push_back((uint8_t)tissudb::TissParamType::STRING);
    std::string p1_val = "test_user";
    append_uint32_be(expected_body, p1_val.length());
    expected_body.insert(expected_body.end(), p1_val.begin(), p1_val.end());
    // 4. Param 2 (int64)
    expected_body.push_back((uint8_t)tissudb::TissParamType::INT64);
    append_uint32_be(expected_body, sizeof(int64_t));
    int64_t p2_val = 42;
    uint64_t p2_val_be;
    memcpy(&p2_val_be, &p2_val, sizeof(int64_t));
    append_uint64_be(expected_body, p2_val_be);

    std::vector<char> expected_message;
    append_uint32_be(expected_message, expected_body.size());
    expected_message.insert(expected_message.end(), expected_body.begin(), expected_body.end());

    ASSERT_EQ(expected_message, actual_buffer);
}


TEST_CASE(DeprecatedParameterizedQuery_Substitution) {
    MockSession mock_session;
    std::map<std::string, tissudb::TissValue> params;
    params["name"] = "John \"The Rock\" Doe";
    params["age"] = 42;

    mock_session.run_with_client_side_substitution("VALUES ($name, $age)", params);
    const auto& actual_buffer = mock_session.captured_send_buffer;

    std::string expected_query = "VALUES (\"John \\\"The Rock\\\" Doe\", 42)";

    std::vector<char> expected_message;
    append_uint32_be(expected_message, expected_query.length());
    expected_message.insert(expected_message.end(), expected_query.begin(), expected_query.end());

    ASSERT_EQ(expected_message, actual_buffer);
}
