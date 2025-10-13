#include "../db/test_framework.h"
#include <iostream>

// --- Unity Build ---
// We include the source and test files directly into this main file
// to create a single compilation unit. This simplifies the build process
// as we don't need a complex build script or makefile.

// Include the implementation of the code we are testing.
// The path is relative to this test_main.cpp file.
#include "../../quanta_tissu/nexus_flow/graph_logic.cpp"

// Include the test cases for the code.
#include "test_graph_logic.cpp"


int main() {
    std::cout << "--- Running Nexus Flow Test Suite ---" << std::endl;
    TestFramework::run_all_tests();
    std::cout << "--- Nexus Flow Test Suite Finished ---" << std::endl;
    return 0;
}
