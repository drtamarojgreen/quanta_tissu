#include "tissu_sinew.h"
#include <iostream>
#include <stdexcept>

int main() {
    std::cout << "Starting Tissu Sinew C++ client example..." << std::endl;

    try {
        // 1. Create a configuration
        tissudb::TissuConfig config;
        config.host = "tissudb.example.com";
        config.port = 12345;
        config.username = "user";
        config.password = "secret";

        // 2. Create a client
        // The client's lifetime is managed by the unique_ptr
        std::unique_ptr<tissudb::TissuClient> client = tissudb::TissuClient::create(config);

        if (!client) {
            std::cerr << "Failed to create TissuClient." << std::endl;
            return 1;
        }

        // 3. Get a session from the client
        // The session's lifetime is also managed by a unique_ptr
        std::unique_ptr<tissudb::TissuSession> session = client->getSession();

        if (!session) {
            std::cerr << "Failed to get TissuSession." << std::endl;
            return 1;
        }

        // 4. Run a query
        std::cout << "\nRunning a test query..." << std::endl;
        session->run("MATCH (n) RETURN n LIMIT 10");
        std::cout << "Test query executed.\n" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Tissu Sinew C++ client example finished successfully." << std::endl;
    // client and session will be automatically destroyed here thanks to unique_ptr
    return 0;
}
