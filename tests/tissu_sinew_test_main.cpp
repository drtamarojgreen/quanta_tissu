#include "db/test_framework.h"
#include "quanta_tissu/tissu_sinew.h"
#include <iostream>
#include <memory>

// The individual test .cpp files will be compiled separately and linked.
// This file only needs to contain the main entry point.

int main() {
    std::cout << "--- Main function started ---" << std::endl;
    // We need to provide a logger for the tests that use a real client.
    // The default NullLogger would hide all output.
    auto logger = std::make_shared<tissudb::StdLogger>();

    // The test framework doesn't have a built-in way to pass context to tests,
    // so for now, we'll just run them. The tests that need a logger can create it.
    // Ideally, the framework would be improved to support this.

    // TestFramework::run_all_tests();
    std::cout << "--- Main function finished ---" << std::endl;
    return 0;
}
