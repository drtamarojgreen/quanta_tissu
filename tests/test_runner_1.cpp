#include "db/test_framework.h"
#include <iostream>

// Include only one test file
#include "test_tissu_sinew.cpp"

int main() {
    std::cout << "--- Runner 1 started ---" << std::endl;
    TestFramework::run_all_tests();
    std::cout << "--- Runner 1 finished ---" << std::endl;
    return 0;
}
