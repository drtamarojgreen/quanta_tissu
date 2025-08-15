#include "db/test_framework.h"
#include "quanta_tissu/tissu_sinew.h"
#include <vector>
#include <string>
#include <map>

// A mock session for testing purposes
class MockSession : public tissudb::TissuSession {
public:
    // We need a constructor that calls the base class constructor.
    // The arguments don't matter for the test since we won't use the connection.
    MockSession() : tissudb::TissuSession(0, nullptr) {}

    std::vector<std::string> received_queries;

    std::unique_ptr<tissudb::TissuResult> run(const std::string& query) override {
        received_queries.push_back(query);
        return std::make_unique<tissudb::TissuResult>("mock_response");
    }

    // We override the parameterized run so that the test code can call it.
    // The TissuSession implementation will do the parameter substitution and then
    // call the simple run(string) method, which we have overridden to capture the query.
    std::unique_ptr<tissudb::TissuResult> run(const std::string& query, const std::map<std::string, tissudb::TissValue>& params) override {
        return tissudb::TissuSession::run(query, params);
    }

    // NOTE: We do not override `beginTransaction` here.
    // We let the base `TissuSession::beginTransaction` be called. That implementation
    // is a `friend` of `TissuTransaction` and can legally construct it. It will
    // call our mocked `run("BEGIN")` method, so the tests will still work.
};

TEST_CASE(Transaction_Commit) {
    MockSession mock_session;
    {
        auto tx = mock_session.beginTransaction();
        tx->commit();
    }
    ASSERT_EQ(2, mock_session.received_queries.size());
    ASSERT_EQ("BEGIN", mock_session.received_queries[0]);
    ASSERT_EQ("COMMIT", mock_session.received_queries[1]);
}

TEST_CASE(Transaction_Rollback) {
    MockSession mock_session;
    {
        auto tx = mock_session.beginTransaction();
        tx->rollback();
    }
    ASSERT_EQ(2, mock_session.received_queries.size());
    ASSERT_EQ("BEGIN", mock_session.received_queries[0]);
    ASSERT_EQ("ROLLBACK", mock_session.received_queries[1]);
}

TEST_CASE(Transaction_AutoRollbackOnDestruction) {
    MockSession mock_session;
    {
        auto tx = mock_session.beginTransaction();
        // tx goes out of scope without commit or rollback
    }
    ASSERT_EQ(2, mock_session.received_queries.size());
    ASSERT_EQ("BEGIN", mock_session.received_queries[0]);
    ASSERT_EQ("ROLLBACK", mock_session.received_queries[1]);
}

TEST_CASE(ParameterizedQuery_Substitution) {
    MockSession mock_session;
    std::map<std::string, tissudb::TissValue> params;
    params["name"] = "John \"The Rock\" Doe";
    params["age"] = 42;
    params["cash"] = 123.45;
    params["is_active"] = true;
    params["data"] = nullptr;

    // Call the parameterized run method. TissuSession's implementation will be used.
    mock_session.run("INSERT INTO users (name, age, cash, is_active, data) VALUES ($name, $age, $cash, $is_active, $data)", params);

    // Assert that the simple run method was called with the correct substituted query.
    ASSERT_EQ(1, mock_session.received_queries.size());
    std::string expected_query = "INSERT INTO users (name, age, cash, is_active, data) VALUES (\"John \\\"The Rock\\\" Doe\", 42, 123.450000, true, null)";
    ASSERT_EQ(expected_query, mock_session.received_queries[0]);
}

TEST_CASE(ParameterizedQuery_Substitution_MultipleOccurrences) {
    MockSession mock_session;
    std::map<std::string, tissudb::TissValue> params;
    params["id"] = 123;

    mock_session.run("SELECT * FROM data WHERE id = $id OR user_id = $id", params);

    ASSERT_EQ(1, mock_session.received_queries.size());
    std::string expected_query = "SELECT * FROM data WHERE id = 123 OR user_id = 123";
    ASSERT_EQ(expected_query, mock_session.received_queries[0]);
}
