#include "tissu_sinew.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void run_test_session(tissudb::TissuClient& client, int thread_id) {
    try {
        std::cout << "[Thread " << thread_id << "] Acquiring session..." << std::endl;
        // Session is acquired here. When 'session' goes out of scope, the connection is returned to the pool.
        auto session = client.getSession();
        std::cout << "[Thread " << thread_id << "] Session acquired. Running PING." << std::endl;

        auto result = session->run("PING");

        if (result) {
            std::cout << "[Thread " << thread_id << "] Response: " << result->asString() << std::endl;
        } else {
            std::cerr << "[Thread " << thread_id << "] Received null result." << std::endl;
        }

        // Simulate some work
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "[Thread " << thread_id << "] Work done. Releasing session." << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "[Thread " << thread_id << "] ERROR: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "--- Tissu Sinew Test Application ---" << std::endl;

    // 1. Configure the client
    tissudb::TissuConfig config;
    config.host = "127.0.0.1";
    config.port = 8080; // Changed to match the actual server port

    // 2. Create a client (this manages the connection pool)
    auto client = tissudb::TissuClient::create(config);

    if (!client) {
        std::cerr << "Failed to create TissuClient. Exiting." << std::endl;
        return 1;
    }

    // 3. Run multiple sessions concurrently to test the pool
    std::cout << "\n--- Testing Concurrent Sessions (should use the pool) ---" << std::endl;
    std::vector<std::thread> threads;
    int num_threads = 7; // More than the pool size (5) to test waiting

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(run_test_session, std::ref(*client), i + 1);
        // Stagger the thread creation slightly
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\n--- Test Application Finished ---" << std::endl;

    return 0;
}
