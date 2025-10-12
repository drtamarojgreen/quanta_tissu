#include "db/test_framework.h"
#include "../quanta_tissu/tisslm/program/tissu_sinew.h"
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iostream>

// For mock server
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define SHUT_RDWR SD_BOTH
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

// =================================================================================================
// Mock TissDB Server
// =================================================================================================
// A simple, single-threaded TCP server to test the TissuClient.
// It understands the length-prefix protocol.
class MockServer {
public:
    MockServer(int port) : port_(port), server_fd_(-1), stop_(false) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            perror("WSAStartup failed");
        }
#endif
    }

    ~MockServer() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void start() {
        server_thread_ = std::thread([this]() { this->run(); });
        // Wait for the server to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void stop() {
        stop_ = true;
        if (server_fd_ != -1) {
            shutdown(server_fd_, SHUT_RDWR);
#ifdef _WIN32
            closesocket(server_fd_);
#else
            close(server_fd_);
#endif
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
        socklen_t addrlen = sizeof(address);

        if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket failed");
            return;
        }

        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))) {
            perror("setsockopt");
#ifdef _WIN32
            closesocket(server_fd_);
#else
            close(server_fd_);
#endif
            return;
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
#ifdef _WIN32
            closesocket(server_fd_);
#else
            close(server_fd_);
#endif
            return;
        }
        if (listen(server_fd_, 5) < 0) {
            perror("listen");
#ifdef _WIN32
            closesocket(server_fd_);
#else
            close(server_fd_);
#endif
            return;
        }

        while (!stop_) {
            int client_fd = accept(server_fd_, (struct sockaddr *)&address, &addrlen);
            if (client_fd < 0) {
                if (stop_) break;
                perror("accept");
                continue;
            }

            // Handle the client connection using the length-prefix protocol
            uint32_t net_len;
            ssize_t read_bytes = recv(client_fd, (char*)&net_len, sizeof(net_len), 0);
            if (read_bytes != sizeof(net_len)) {
#ifdef _WIN32
                closesocket(client_fd);
#else
                close(client_fd);
#endif
                continue;
            }
            uint32_t msg_len = ntohl(net_len);
            std::vector<char> buffer(msg_len);
            read_bytes = recv(client_fd, buffer.data(), msg_len, 0);
            if (read_bytes != msg_len) {
#ifdef _WIN32
                closesocket(client_fd);
#else
                close(client_fd);
#endif
                continue;
            }

            // --- Apply server behavior ---
            Behavior current_behavior = behavior_.load();

            if (current_behavior == Behavior::CloseImmediately) {
#ifdef _WIN32
                closesocket(client_fd);
#else
                close(client_fd);
#endif
                continue; // Done with this client, wait for next.
            }

            if (current_behavior == Behavior::SlowResponse) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_.load()));
            }

            // Default behavior is Echo
            uint32_t resp_len_net = htonl(msg_len);
            send(client_fd, (const char*)&resp_len_net, sizeof(resp_len_net), 0);
            send(client_fd, buffer.data(), msg_len, 0);

#ifdef _WIN32
            closesocket(client_fd);
#else
            close(client_fd);
#endif
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