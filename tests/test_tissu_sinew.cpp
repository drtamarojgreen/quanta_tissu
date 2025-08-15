#include "db/test_framework.h"
#include "quanta_tissu/tissu_sinew.h"
#include <vector>
#include <string>
#include <map>

// A mock session for testing purposes
class MockSession : public tissudb::ISession {
public:
    std::vector<std::string> received_queries;

    std::unique_ptr<tissudb::TissuResult> run(const std::string& query) override {
        received_queries.push_back(query);
        return std::make_unique<tissudb::TissuResult>("mock_response");
    }

    std::unique_ptr<tissudb::TissuResult> run(const std::string& query, const std::map<std::string, tissudb::TissValue>& params) override {
        // For testing transactions, we just delegate to the simple run method.
        // The parameterized run method is tested separately.
        return run(query);
    }

    std::unique_ptr<tissudb::TissuTransaction> beginTransaction() override {
        run("BEGIN");
        // This is slightly different from the real implementation, but allows us to construct TissuTransaction for testing.
        return std::unique_ptr<tissudb::TissuTransaction>(new tissudb::TissuTransaction(this));
    }
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
    tissudb::TissuSession session(0, nullptr); // We are not using the connection, just the substitution logic.
    std::map<std::string, tissudb::TissValue> params;
    params["name"] = "John Doe";
    params["age"] = 42;
    params["cash"] = 123.45;
    params["is_active"] = true;

    // We can't test the private substitution method directly, so we test the public run method.
    // Since we can't mock the non-virtual run(string) method it calls, this test is more of a compile-check
    // and a placeholder for when we can test against a server or with more advanced mocking.
    // For now, we will test the TissValue formatting directly.
    ASSERT_EQ("\"John Doe\"", tissudb::TissValue("John Doe").toQueryString());
    ASSERT_EQ("42", tissudb::TissValue(42).toQueryString());
    ASSERT_EQ("123.450000", tissudb::TissValue(123.45).toQueryString());
    ASSERT_EQ("true", tissudb::TissValue(true).toQueryString());
    ASSERT_EQ("null", tissudb::TissValue().toQueryString());
}
