#include "test_framework.h"

// Include only the sstable test file
#include "test_sstable.cpp"

int main() {
    TestFramework::run_all_tests();
    return 0;
}
