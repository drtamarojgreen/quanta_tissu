#include "../quanta_tissu/tissu_sinew.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <command>" << std::endl;
        return 1;
    }

    tissudb::TissuConfig config;
    config.host = argv[1];
    try {
        config.port = std::stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid port number: " << argv[2] << std::endl;
        return 1;
    }

    std::string command = argv[3];

    try {
        std::unique_ptr<tissudb::TissuClient> client = tissudb::TissuClient::create(config);
        if (!client) {
            std::cerr << "Failed to create TissuClient." << std::endl;
            return 1;
        }

        std::unique_ptr<tissudb::ISession> session = client->getSession();
        std::unique_ptr<tissudb::TissuResult> result = session->run(command);

        if (result) {
            std::cout << result->asString() << std::endl;
        }
    } catch (const tissudb::TissuException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
