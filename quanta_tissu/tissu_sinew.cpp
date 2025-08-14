#include "tissu_sinew.h"
#include <iostream>
#include <stdexcept> // For std::runtime_error

// For native socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring> // For memset

namespace tissudb {

// --- TissuClient::Impl (Private Implementation) ---

class TissuClient::Impl {
public:
    // Constructor now establishes a connection
    Impl(const TissuConfig& config) : config_(config), sockfd_(-1) {
        std::cout << "TissuClient::Impl: Initializing for " << config_.host << ":" << config_.port << std::endl;
        connect_to_server();
    }

    // Destructor closes the connection
    ~Impl() {
        if (sockfd_ != -1) {
            std::cout << "TissuClient::Impl: Closing socket." << std::endl;
            close(sockfd_);
        }
        std::cout << "TissuClient::Impl destroyed." << std::endl;
    }

    void connect_to_server() {
        struct addrinfo hints, *servinfo, *p;
        int rv;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;

        std::string port_str = std::to_string(config_.port);

        if ((rv = getaddrinfo(config_.host.c_str(), port_str.c_str(), &hints, &servinfo)) != 0) {
            throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(rv)));
        }

        // Loop through all the results and connect to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                // perror("client: socket"); // This is noisy, disable for now
                continue;
            }

            if (connect(sockfd_, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd_);
                // perror("client: connect"); // This is noisy, disable for now
                continue;
            }

            break; // If we get here, we must have connected successfully
        }

        if (p == NULL) {
            freeaddrinfo(servinfo);
            throw std::runtime_error("TissuClient: Failed to connect to host");
        }

        std::cout << "TissuClient: Successfully connected." << std::endl;

        freeaddrinfo(servinfo); // All done with this structure
    }

    int get_socket_fd() const {
        return sockfd_;
    }

private:
    TissuConfig config_;
    int sockfd_; // The single socket connection
};


// --- TissuSession::Impl (Private Implementation) ---

class TissuSession::Impl {
public:
    Impl(int sockfd) : sockfd_(sockfd) {}
    int sockfd_;
};


// --- TissuSession Method Implementations ---

TissuSession::TissuSession(int sockfd) : pimpl(std::make_unique<Impl>(sockfd)) {
    std::cout << "TissuSession created." << std::endl;
}

TissuSession::~TissuSession() {
    std::cout << "TissuSession destroyed." << std::endl;
}

void TissuSession::run(const std::string& query) {
    if (pimpl->sockfd_ == -1) {
        throw std::runtime_error("Session has no valid connection.");
    }
    std::cout << "Executing query over socket: " << query << std::endl;

    std::string message = query + "\n";
    if (send(pimpl->sockfd_, message.c_str(), message.length(), 0) == -1) {
        perror("send");
        throw std::runtime_error("Failed to send query.");
    }

    std::cout << "Query sent. (Response handling not yet implemented)" << std::endl;
}


// --- TissuClient Method Implementations ---

TissuClient::TissuClient() : pimpl(nullptr) {}

TissuClient::~TissuClient() {
    std::cout << "TissuClient destroyed." << std::endl;
}

std::unique_ptr<TissuClient> TissuClient::create(const TissuConfig& config) {
    auto client = std::unique_ptr<TissuClient>(new TissuClient());
    try {
        client->pimpl = std::make_unique<TissuClient::Impl>(config);
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to initialize TissuClient: " << e.what() << std::endl;
        return nullptr;
    }
    return client;
}

std::unique_ptr<TissuSession> TissuClient::getSession() {
    std::cout << "TissuClient creating a new session." << std::endl;
    if (!pimpl) {
        throw std::runtime_error("Client is not initialized.");
    }
    return std::unique_ptr<TissuSession>(new TissuSession(pimpl->get_socket_fd()));
}

} // namespace tissudb
