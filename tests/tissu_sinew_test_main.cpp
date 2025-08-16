#include "db/test_framework.h"

// Include only the test files relevant to the Tissu Sinew C++ connector.
#include "test_tissu_sinew.cpp"
#include "test_tissu_client.cpp"

int main() {
    // We need to provide a logger for the tests that use a real client.
    // The default NullLogger would hide all output.
    auto logger = std::make_shared<tissudb::StdLogger>();

    // The test framework doesn't have a built-in way to pass context to tests,
    // so for now, we'll just run them. The tests that need a logger can create it.
    // Ideally, the framework would be improved to support this.

    TestFramework::run_all_tests();
    return 0;
}
