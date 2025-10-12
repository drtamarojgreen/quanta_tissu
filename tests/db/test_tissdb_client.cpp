#include "../../quanta_tissu/tisslm/program/db/tissdb_client.h"
#include <iostream>
#include <cassert>

void test_connection() {
    TissDB::TissDBClient client;
    bool connected = client.test_connection();
    assert(connected && "Failed to connect to TissDB");
    std::cout << "Connection test passed!" << std::endl;
}

