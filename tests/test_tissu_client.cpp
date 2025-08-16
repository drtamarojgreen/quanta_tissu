#include "db/test_framework.h"
#include "quanta_tissu/tissu_sinew.h"
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iostream>

// For mock server
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// =================================================================================================
// Mock TissDB Server
// =================================================================================================
// A simple, single-threaded TCP server to test the TissuClient.
// It understands the length-prefix protocol.
class MockServer {
public:
    MockServer(int port) : port_(port), server_fd_(-1), stop_(false) {}

    void start() {
        server_thread_ = std::thread([this]() { this->run(); });
        // Wait for the server to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void stop() {
        stop_ = true;
        if (server_fd_ != -1) {
            shutdown(server_fd_, SHUT_RDWR);
            close(server_fd_);
        }
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    // --- Server Behaviors ---
    enum class Behavior {
        Echo,             // Echo the request back
        CloseImmediately, // Close the connection after reading the request
        SlowResponse      // Wait before sending a response
    };
    std::atomic<Behavior> behavior_{Behavior::Echo};
    std::atomic<int> delay_ms_{0};

private:
    void run() {
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);

        if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            return;
        }

        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt");
            close(server_fd_);
            return;
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            close(server_fd_);
            return;
        }
        if (listen(server_fd_, 5) < 0) {
            perror("listen");
            close(server_fd_);
            return;
        }

        while (!stop_) {
            int client_fd = accept(server_fd_, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client_fd < 0) {
                if (stop_) break;
                perror("accept");
                continue;
            }

            // Handle the client connection using the length-prefix protocol
            uint32_t net_len;
            ssize_t read_bytes = read(client_fd, &net_len, sizeof(net_len));
            if (read_bytes != sizeof(net_len)) {
                close(client_fd);
                continue;
            }
            uint32_t msg_len = ntohl(net_len);
            std::vector<char> buffer(msg_len);
            read_bytes = read(client_fd, buffer.data(), msg_len);
            if (read_bytes != msg_len) {
                close(client_fd);
                continue;
            }

            // --- Apply server behavior ---
            Behavior current_behavior = behavior_.load();

            if (current_behavior == Behavior::CloseImmediately) {
                close(client_fd);
                continue; // Done with this client, wait for next.
            }

            if (current_behavior == Behavior::SlowResponse) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_.load()));
            }

            // Default behavior is Echo
            uint32_t resp_len_net = htonl(msg_len);
            send(client_fd, &resp_len_net, sizeof(resp_len_net), 0);
            send(client_fd, buffer.data(), msg_len, 0);

            close(client_fd);
        }
    }

    int port_;
    int server_fd_;
    std::thread server_thread_;
    std::atomic<bool> stop_;
};

// =================================================================================================
// Test Cases
// =================================================================================================

TEST_CASE(Client_BasicConnectionAndQuery) {
    const int port = 9999;
    MockServer server(port);
    server.start();

    tissudb::TissuConfig config;
    config.host = "127.0.0.1";
    config.port = port;
    config.pool_size = 1;

    auto client = tissudb::TissuClient::create(config);
    ASSERT_NE(nullptr, client.get());

    try {
        auto session = client->getSession();
        ASSERT_NE(nullptr, session.get());
        auto result = session->run("PING");
        ASSERT_EQ("PING", result->asString());
    } catch (const tissudb::TissuException& e) {
        FAIL("Threw unexpected exception: " << e.what());
    }

    server.stop();
}

TEST_CASE(Client_ConnectionPoolTimeout) {
    const int port = 9998;
    MockServer server(port);
    server.start();

    tissudb::TissuConfig config;
    config.host = "127.0.0.1";
    config.port = port;
    config.pool_size = 1;
    config.connect_timeout_ms = 100; // Short timeout

    auto client = tissudb::TissuClient::create(config);
    ASSERT_NE(nullptr, client.get());

    // 1. Acquire the only connection in the pool
    auto session1 = client->getSession();
    ASSERT_NE(nullptr, session1.get());

    // 2. In a new thread, try to get another connection. It should time out.
    std::atomic<bool> correct_exception_thrown = false;
    std::thread t([&]() {
        try {
            auto session2 = client->getSession();
            // If we get here, the test fails because we got a session when we shouldn't have.
        } catch (const tissudb::TissuConnectionException& e) {
            std::string msg = e.what();
            if (msg.find("Timeout") != std::string::npos) {
                correct_exception_thrown = true;
            }
        } catch (...) {
            // Any other exception is a failure.
        }
    });

    t.join();

    ASSERT_TRUE(correct_exception_thrown);

    server.stop();
}

TEST_CASE(Client_DeadConnectionHandling) {
    const int port = 9997;
    MockServer server(port);
    // The server will read our query and immediately close the connection.
    server.behavior_.store(MockServer::Behavior::CloseImmediately);
    server.start();

    tissudb::TissuConfig config;
    config.host = "127.0.0.1";
    config.port = port;
    config.pool_size = 1;
    config.connect_timeout_ms = 100; // Short timeout

    auto client = tissudb::TissuClient::create(config);
    ASSERT_NE(nullptr, client.get());

    // 1. The first query should fail because the server closes the connection.
    // Our dead connection handling logic should catch this.
    bool query_exception_thrown = false;
    try {
        auto session = client->getSession();
        ASSERT_NE(nullptr, session.get());
        session->run("This will fail");
        FAIL("The query should have thrown an exception.");
    } catch (const tissudb::TissuQueryException& e) {
        // This is the expected outcome for the first query.
        query_exception_thrown = true;
    }
    ASSERT_TRUE(query_exception_thrown);

    // 2. The connection pool should now be empty because the dead connection was
    // purged instead of being returned. Trying to get another session should time out.
    bool timeout_exception_thrown = false;
    try {
        auto session = client->getSession();
        FAIL("Should not be able to get a session from an empty pool.");
    } catch (const tissudb::TissuConnectionException& e) {
        std::string msg = e.what();
        if (msg.find("Timeout") != std::string::npos) {
            timeout_exception_thrown = true;
        }
    }
    ASSERT_TRUE(timeout_exception_thrown);

    server.stop();
}
