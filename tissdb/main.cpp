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
const int DEFAULT_PORT = 9876;
const std::string DEFAULT_DATA_DIR = "tissdb_data";

// Global flag for signal handling
std::atomic<bool> shutdown_requested(false);

void signal_handler(int signum) {
    std::cout << "\nCaught signal " << signum << ". Shutting down..." << std::endl;
    shutdown_requested.store(true);
}

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options]\n\n"
              << "Options:\n"
              << "  -h, --help           Show this help message and exit\n"
              << "  --port <port>        Specify the port to listen on (default: " << DEFAULT_PORT << ")\n"
              << "  --data-dir <path>    Specify the data directory (default: " << DEFAULT_DATA_DIR << ")\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    // --- Argument Parsing ---
    int port = DEFAULT_PORT;
    std::string data_dir = DEFAULT_DATA_DIR;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--port") {
            if (i + 1 < argc) {
                try {
                    port = std::stoi(argv[++i]);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error: Invalid port number '" << argv[i] << "'." << std::endl;
                    return 1;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Error: Port number '" << argv[i] << "' is out of range." << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: --port option requires an argument." << std::endl;
                return 1;
            }
        } else if (arg == "--data-dir") {
            if (i + 1 < argc) {
                data_dir = argv[++i];
            } else {
                std::cerr << "Error: --data-dir option requires an argument." << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'." << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    // --- Server Initialization ---
    try {
        std::cout << "TissDB starting..." << std::endl;

        // 1. Initialize the database manager
        TissDB::Storage::DatabaseManager db_manager(data_dir);
        std::cout << "  - Data directory: " << data_dir << std::endl;

        // 2. Initialize the API server
        TissDB::API::HttpServer server(db_manager, port);
        std::cout << "  - Listening on port: " << port << std::endl;

        // 3. Start the server (this will start a background thread)
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
