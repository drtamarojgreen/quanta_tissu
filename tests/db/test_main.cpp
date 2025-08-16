#include "test_framework.h"

// Include individual test files here
#include "test_document.cpp"
#include "test_wal.cpp"
#include "test_memtable.cpp"
#include "test_sstable.cpp"
#include "test_bpp_tree.cpp"
#include "test_indexer.cpp"
#include "test_collection.cpp"
#include "test_constraints.cpp"
#include "test_lsm_tree.cpp"
#include "test_parser.cpp"
#include "test_executor.cpp"
#include "test_serialization.cpp"
#include "test_json.cpp"
#include "test_transactions.cpp"
#include "test_stddev.cpp"

// Tissu Sinew Client Library Tests
#include "../test_tissu_sinew.cpp"
#include "../test_tissu_client.cpp"

int main() {
    TestFramework::run_all_tests();
    return 0;
}
