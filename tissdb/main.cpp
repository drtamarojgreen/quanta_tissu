#include "storage/database_manager.h"
#include "api/http_server.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <csignal>
#include <atomic>

// --- Configuration ---
const int DEFAULT_PORT = 8080;
const std::string DEFAULT_DATA_PATH = "./tissdb_data";
const std::string DEFAULT_DB_NAME = "tiss_db";

// Global flag for signal handling
std::atomic<bool> shutdown_requested(false);

void signal_handler(int signum) {
    std::cout << "\nCaught signal " << signum << ". Shutting down..." << std::endl;
    shutdown_requested.store(true);
}

int main(int argc, char* argv[]) {
    // --- Basic command-line argument parsing ---
    int port = DEFAULT_PORT;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid port number provided: '" << argv[1] << "'. Using default port " << DEFAULT_PORT << "." << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Port number '" << argv[1] << "' is out of range. Using default port " << DEFAULT_PORT << "." << std::endl;
        }
    }

    // --- Server Initialization ---
    std::cout << "TissDB starting..." << std::endl;
    std::cout << "  - Data path: " << DEFAULT_DATA_PATH << std::endl;
    std::cout << "  - Listening on port: " << port << std::endl;

    try {
        // 1. Initialize the database manager
        TissDB::Storage::DatabaseManager db_manager(DEFAULT_DATA_PATH);

        // 2. Ensure a default database exists
        if (!db_manager.database_exists(DEFAULT_DB_NAME)) {
            std::cout << "Creating default database: " << DEFAULT_DB_NAME << std::endl;
            db_manager.create_database(DEFAULT_DB_NAME);
        }

        // 3. Initialize the API server
        TissDB::API::HttpServer server(db_manager, port);

        // 4. Start the server (this will start a background thread)
        server.start();
        std::cout << "Server has started successfully." << std::endl;

        // 4. Register signal handlers for graceful shutdown
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        std::cout << "Press Ctrl+C to exit." << std::endl;

        // 5. Wait for shutdown signal
        while (!shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 6. Perform graceful shutdown
        std::cout << "Stopping server..." << std::endl;
        server.stop();
        std::cout << "Shutting down database manager..." << std::endl;
        db_manager.shutdown();
        std::cout << "Shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
