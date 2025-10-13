#include "test_framework.h"
#include "http_client.h"
#include <iostream>
#include <stdexcept>

// For now, we assume the server is running on localhost:8080
// A proper test setup would involve starting/stopping the server.
const int TEST_PORT = 8080;

void test_auth_no_token() {
    TEST_CASE("Auth: No Token");
    TissDB::Test::HttpClient client("127.0.0.1", TEST_PORT);
    auto response = client.get("/_databases"); // A protected endpoint
    ASSERT(response.status_code == 401);
    ASSERT(response.body.find("Authorization header missing") != std::string::npos);
    std::cout << "PASS: " << "Auth: No Token" << std::endl;
}

void test_auth_invalid_token() {
    TEST_CASE("Auth: Invalid Token");
    TissDB::Test::HttpClient client("127.0.0.1", TEST_PORT);
    client.set_header("Authorization", "Bearer invalid_token");
    auto response = client.get("/_databases");
    ASSERT(response.status_code == 401);
    ASSERT(response.body.find("Invalid or missing bearer token") != std::string::npos);
    std::cout << "PASS: " << "Auth: Invalid Token" << std::endl;
}

void test_auth_valid_token() {
    TEST_CASE("Auth: Valid Token");
    TissDB::Test::HttpClient client("127.0.0.1", TEST_PORT);
    client.set_header("Authorization", "Bearer static_test_token");
    // This request should pass the auth check.
    // The endpoint is valid, so we expect a 200.
    auto response = client.get("/_databases");
    ASSERT(response.status_code == 200);
    std::cout << "PASS: " << "Auth: Valid Token" << std::endl;
}

void test_health_endpoint_no_token() {
    TEST_CASE("Auth: Health Endpoint No Token");
    TissDB::Test::HttpClient client("127.0.0.1", TEST_PORT);
    auto response = client.get("/_health");
    ASSERT(response.status_code == 200);
    std::cout << "PASS: " << "Auth: Health Endpoint No Token" << std::endl;
}

void test_rbac_permission_denied() {
    TEST_CASE("RBAC: Permission Denied");
    TissDB::Test::HttpClient client("127.0.0.1", TEST_PORT);
    client.set_header("Authorization", "Bearer read_only_token");
    auto response = client.del("/any_database_name");
    ASSERT(response.status_code == 403);
    ASSERT(response.body.find("You do not have permission to delete a database.") != std::string::npos);
    std::cout << "PASS: " << "RBAC: Permission Denied" << std::endl;
}

int main() {
    try {
        test_auth_no_token();
        test_auth_invalid_token();
        test_auth_valid_token();
        test_health_endpoint_no_token();
        test_rbac_permission_denied();
    } catch (const std::exception& e) {
        std::cerr << "A test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
