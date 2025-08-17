#include "tissu_sinew.h"
#include <stdexcept> // For std::runtime_error
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <chrono>

// For native socket programming
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> // For htonl, ntohl
#endif
#include <cstring>     // For memset, strerror
#include <cerrno>      // For errno

namespace tissudb {

/**
 * @brief Helper function to reliably receive a specific number of bytes from a socket.
 */
static bool recv_all(int sockfd, void* buffer, size_t len) {
    char* p = static_cast<char*>(buffer);
    size_t bytes_to_read = len;
    while (bytes_to_read > 0) {
        ssize_t bytes_received = recv(sockfd, p, bytes_to_read, 0);
        if (bytes_received <= 0) {
            return false;
        }
        bytes_to_read -= bytes_received;
        p += bytes_received;
    }
    return true;
}


// --- TissuClientImpl (Opaque Implementation for TissuClient) ---

class TissuClientImpl {
public:
    TissuClientImpl(const TissuConfig& config) : config_(config) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw TissuConnectionException("WSAStartup failed.");
        }
#endif
        config_.logger->info("Initializing connection pool for " + config_.host + ":" + std::to_string(config_.port));

        for (size_t i = 0; i < config_.pool_size; ++i) {
            int sockfd = connect_to_server();
            if (sockfd != -1) {
                all_connections_.push_back(sockfd);
                available_connections_.push(sockfd);
            }
        }
        config_.logger->info("Connection pool initialized with " + std::to_string(available_connections_.size()) + " connections.");
    }

    ~TissuClientImpl() {
        config_.logger->info("Closing all connections.");
        for (int sockfd : all_connections_) {
#ifdef _WIN32
            closesocket(sockfd);
#else
            close(sockfd);
#endif
        }
#ifdef _WIN32
        WSACleanup();
#endif
        config_.logger->info("TissuClientImpl destroyed.");
    }

    int getConnection() {
        std::unique_lock<std::mutex> lock(mtx_);
        if (cv_.wait_for(lock, std::chrono::milliseconds(config_.connect_timeout_ms), [this]{ return !available_connections_.empty(); })) {
            int sockfd = available_connections_.front();
            available_connections_.pop();
            config_.logger->info("Connection " + std::to_string(sockfd) + " acquired. Pool size: " + std::to_string(available_connections_.size()));
            return sockfd;
        } else {
            throw TissuConnectionException("Timeout waiting for connection from pool.");
        }
    }

    void releaseConnection(int sockfd) {
        if (sockfd == -1) {
            config_.logger->info("Ignoring release of dead connection marker.");
            return;
        }
        std::unique_lock<std::mutex> lock(mtx_);
        available_connections_.push(sockfd);
        lock.unlock();
        cv_.notify_one();
        config_.logger->info("Connection " + std::to_string(sockfd) + " released. Pool size: " + std::to_string(available_connections_.size()));
    }

    void declareConnectionDead(int sockfd) {
        std::unique_lock<std::mutex> lock(mtx_);
        config_.logger->info("Connection " + std::to_string(sockfd) + " declared dead. Removing from pool.");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        all_connections_.erase(std::remove(all_connections_.begin(), all_connections_.end(), sockfd), all_connections_.end());
    }

    const TissuConfig& getConfig() const {
        return config_;
    }

private:
    int connect_to_server() {
        struct addrinfo hints, *servinfo, *p;
        int rv, sockfd = -1;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        std::string port_str = std::to_string(config_.port);

        if ((rv = getaddrinfo(config_.host.c_str(), port_str.c_str(), &hints, &servinfo)) != 0) {
            throw TissuConnectionException("getaddrinfo failed: " + std::string(gai_strerror(rv)));
        }

        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
#ifdef _WIN32
                closesocket(sockfd);
#else
                close(sockfd);
#endif
                sockfd = -1;
                continue;
            }
            break;
        }

        freeaddrinfo(servinfo);

        if (sockfd == -1) {
            throw TissuConnectionException("TissuClient: Failed to connect to host");
        }

        config_.logger->info("Successfully connected socket: " + std::to_string(sockfd));
        return sockfd;
    }

    TissuConfig config_;
    std::vector<int> all_connections_;
    std::queue<int> available_connections_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

// --- TissValue Method Implementations ---
std::string TissValue::toQueryString() const {
    // ... (same as before)
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) return "null";
        else if constexpr (std::is_same_v<T, std::string>) {
            std::string escaped = arg;
            size_t pos = 0;
            while ((pos = escaped.find('"', pos)) != std::string::npos) {
                escaped.replace(pos, 1, "\\");
                pos += 2;
            }
            return '"' + escaped + '"';
        }
        else if constexpr (std::is_same_v<T, int64_t>) return std::to_string(arg);
        else if constexpr (std::is_same_v<T, double>) return std::to_string(arg);
        else if constexpr (std::is_same_v<T, bool>) return arg ? "true" : "false";
    }, value_);
}


// --- TissuSession::Impl (Private Implementation) ---
class TissuSession::Impl {
public:
    Impl(int sockfd, TissuClientImpl* client_impl)
        : sockfd_(sockfd), client_impl_(client_impl) {}

    ~Impl() {
        if (client_impl_ && sockfd_ != -1) {
            client_impl_->releaseConnection(sockfd_);
        }
    }

    int sockfd_;
    TissuClientImpl* client_impl_;
};

// --- TissuSession Method Implementations ---
TissuSession::TissuSession(int sockfd, TissuClientImpl* client_impl)
    : pimpl(std::make_unique<Impl>(sockfd, client_impl)) {
    pimpl->client_impl_->getConfig().logger->info("TissuSession created with connection " + std::to_string(sockfd));
}

TissuSession::~TissuSession() {
    pimpl->client_impl_->getConfig().logger->info("TissuSession for connection " + std::to_string(pimpl->sockfd_) + " destroyed.");
}

std::unique_ptr<TissuResult> TissuSession::run(const std::string& query) {
    if (pimpl->sockfd_ == -1) throw TissuException("Session has no valid connection.");

    uint32_t len = query.length();
    uint32_t net_len = htonl(len);

    if (send(pimpl->sockfd_, (const char*)&net_len, sizeof(net_len), 0) == -1 || send(pimpl->sockfd_, query.c_str(), len, 0) == -1) {
        pimpl->client_impl_->getConfig().logger->error("send failed: " + std::string(strerror(errno)));
        pimpl->client_impl_->declareConnectionDead(pimpl->sockfd_);
        pimpl->sockfd_ = -1;
        throw TissuQueryException("Failed to send query.");
    }

    uint32_t response_net_len;
    if (!recv_all(pimpl->sockfd_, (char*)&response_net_len, sizeof(response_net_len))) {
        pimpl->client_impl_->getConfig().logger->error("recv header failed: " + std::string(strerror(errno)));
        pimpl->client_impl_->declareConnectionDead(pimpl->sockfd_);
        pimpl->sockfd_ = -1;
        throw TissuQueryException("Failed to receive response header.");
    }

    uint32_t response_len = ntohl(response_net_len);
    const uint32_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024;
    if (response_len > MAX_RESPONSE_SIZE) {
        pimpl->client_impl_->declareConnectionDead(pimpl->sockfd_);
        pimpl->sockfd_ = -1;
        throw TissuQueryException("Response size limit exceeded.");
    }

    std::vector<char> buffer(response_len);
    if (response_len > 0 && !recv_all(pimpl->sockfd_, buffer.data(), response_len)) {
        pimpl->client_impl_->getConfig().logger->error("recv payload failed: " + std::string(strerror(errno)));
        pimpl->client_impl_->declareConnectionDead(pimpl->sockfd_);
        pimpl->sockfd_ = -1;
        throw TissuQueryException("Failed to receive response payload.");
    }

    return std::make_unique<TissuResult>(std::string(buffer.begin(), buffer.end()));
}

std::unique_ptr<TissuResult> TissuSession::run(const std::string& query, const std::map<std::string, TissValue>& params) {
    // ... (same as before)
    std::string final_query = query;
    std::vector<std::pair<std::string, TissValue>> sorted_params(params.begin(), params.end());
    std::sort(sorted_params.begin(), sorted_params.end(),
              [](const auto& a, const auto& b) { return a.first.length() > b.first.length(); });
    for (const auto& [key, value] : sorted_params) {
        std::string placeholder = "$" + key;
        std::string value_str = value.toQueryString();
        size_t pos = 0;
        while ((pos = final_query.find(placeholder, pos)) != std::string::npos) {
            final_query.replace(pos, placeholder.length(), value_str);
            pos += value_str.length();
        }
    }
    return run(final_query);
}

std::unique_ptr<TissuTransaction> TissuSession::beginTransaction() {
    run("BEGIN");
    return std::unique_ptr<TissuTransaction>(new TissuTransaction(this));
}


// --- TissuTransaction Method Implementations ---
TissuTransaction::TissuTransaction(ISession* session)
    : session_(session), is_active_(true) {
}

TissuTransaction::~TissuTransaction() {
    if (is_active_) {
        try {
            rollback();
        } catch (const std::exception&) {
            // Cannot log here easily and should not throw from a destructor.
        }
    }
}

void TissuTransaction::commit() {
    if (!is_active_) throw TissuException("Transaction is not active.");
    session_->run("COMMIT");
    is_active_ = false;
}

void TissuTransaction::rollback() {
    if (!is_active_) throw TissuException("Transaction is not active.");
    session_->run("ROLLBACK");
    is_active_ = false;
}


// --- TissuClient Method Implementations ---
TissuClient::TissuClient(std::unique_ptr<TissuClientImpl> impl) : pimpl(std::move(impl)) {}

TissuClient::~TissuClient() {
    pimpl->getConfig().logger->info("TissuClient destroyed.");
}

std::unique_ptr<TissuClient> TissuClient::create(const TissuConfig& config) {
    try {
        auto pimpl = std::make_unique<TissuClientImpl>(config);
        return std::unique_ptr<TissuClient>(new TissuClient(std::move(pimpl)));
    } catch (const TissuException& e) {
        config.logger->error("Failed to initialize TissuClient: " + std::string(e.what()));
        return nullptr;
    }
}

std::unique_ptr<ISession> TissuClient::getSession() {
    if (!pimpl) throw TissuException("Client is not initialized.");
    int sockfd = pimpl->getConnection();
    return std::unique_ptr<TissuSession>(new TissuSession(sockfd, pimpl.get()));
}

} // namespace tissudb