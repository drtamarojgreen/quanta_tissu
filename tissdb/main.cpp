#include "storage/lsm_tree.h"
#include "api/http_server.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <stdexcept>

// --- Configuration ---
const int DEFAULT_PORT = 8080;
const std::string DEFAULT_DATA_DIR = "./tissdb_data";

int main(int argc, char* argv[]) {
    // --- Basic command-line argument parsing ---
    int port = DEFAULT_PORT;
    std::string data_dir = DEFAULT_DATA_DIR;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid port number provided: '" << argv[1] << "'. Using default port " << DEFAULT_PORT << "." << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Port number '" << argv[1] << "' is out of range. Using default port " << DEFAULT_PORT << "." << std::endl;
        }
    }
    if (argc > 2) {
        data_dir = argv[2];
    }

    // --- Server Initialization ---
    std::cout << "TissDB starting..." << std::endl;
    std::cout << "  - Data directory: " << data_dir << std::endl;
    std::cout << "  - Listening on port: " << port << std::endl;

    try {
        // 1. Initialize the storage engine
        TissDB::Storage::LSMTree storage(data_dir);

        // 2. Initialize the API server
        TissDB::API::HttpServer server(storage, port);

        // 3. Start the server (this will start a background thread)
        server.start();

        std::cout << "Server has started successfully. Running in the background." << std::endl;
        std::cout << "Press Ctrl+C to exit." << std::endl;

        // 4. Keep the main thread alive.
        // A more robust server would use proper signal handling (e.g., for SIGINT, SIGTERM)
        // to call server.stop() and allow for a graceful shutdown. For this placeholder,
        // we'll just loop indefinitely. The HttpServer's destructor will handle stopping.
        while (true) {
            std::this_thread::sleep_for(std::chrono::hours(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred during startup: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
