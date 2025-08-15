#include "tissu_sinew.h"
#include <iostream>
#include <stdexcept> // For std::runtime_error
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

// For native socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring> // For memset

namespace tissudb {

// --- TissuClientImpl (Opaque Implementation for TissuClient) ---

class TissuClientImpl {
public:
    TissuClientImpl(const TissuConfig& config) : config_(config), pool_size_(5) {
        std::cout << "TissuClientImpl: Initializing connection pool for " << config_.host << ":" << config_.port << std::endl;

        for (size_t i = 0; i < pool_size_; ++i) {
            int sockfd = connect_to_server();
            all_connections_.push_back(sockfd);
            available_connections_.push(sockfd);
        }
        std::cout << "Connection pool initialized with " << all_connections_.size() << " connections." << std::endl;
    }

    ~TissuClientImpl() {
        std::cout << "TissuClientImpl: Closing all connections." << std::endl;
        for (int sockfd : all_connections_) {
            close(sockfd);
        }
        std::cout << "TissuClientImpl destroyed." << std::endl;
    }

    int getConnection() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return !available_connections_.empty(); });

        int sockfd = available_connections_.front();
        available_connections_.pop();
        std::cout << "Connection " << sockfd << " acquired from pool. Pool size: " << available_connections_.size() << std::endl;
        return sockfd;
    }

    void releaseConnection(int sockfd) {
        std::unique_lock<std::mutex> lock(mtx_);
        available_connections_.push(sockfd);
        lock.unlock();
        cv_.notify_one();
        std::cout << "Connection " << sockfd << " released to pool. Pool size: " << available_connections_.size() << std::endl;
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
                close(sockfd);
                sockfd = -1;
                continue;
            }
            break;
        }

        freeaddrinfo(servinfo);

        if (sockfd == -1) {
            throw TissuConnectionException("TissuClient: Failed to connect to host");
        }

        std::cout << "Successfully connected socket: " << sockfd << std::endl;
        return sockfd;
    }

    TissuConfig config_;
    size_t pool_size_;
    std::vector<int> all_connections_;
    std::queue<int> available_connections_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

// --- TissValue Method Implementations ---

std::string TissValue::toQueryString() const {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return "null";
        } else if constexpr (std::is_same_v<T, std::string>) {
            // Basic escaping for quotes
            std::string escaped = arg;
            size_t pos = 0;
            while ((pos = escaped.find('"', pos)) != std::string::npos) {
                escaped.replace(pos, 1, "\\\"");
                pos += 2;
            }
            return "\"" + escaped + "\"";
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        }
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
    TissuClientImpl* client_impl_; // Pointer back to the client's implementation
};

// --- TissuSession Method Implementations ---

TissuSession::TissuSession(int sockfd, TissuClientImpl* client_impl)
    : pimpl(std::make_unique<Impl>(sockfd, client_impl)) {
    std::cout << "TissuSession created with connection " << sockfd << "." << std::endl;
}

TissuSession::~TissuSession() {
    // The pimpl destructor will handle releasing the connection
    std::cout << "TissuSession for connection " << pimpl->sockfd_ << " destroyed." << std::endl;
}

std::unique_ptr<TissuResult> TissuSession::run(const std::string& query) {
    if (pimpl->sockfd_ == -1) {
        throw TissuException("Session has no valid connection.");
    }

    std::string message = query + "\n";
    if (send(pimpl->sockfd_, message.c_str(), message.length(), 0) == -1) {
        perror("send");
        pimpl->sockfd_ = -1;
        throw TissuQueryException("Failed to send query.");
    }

    char buffer[4096];
    std::string response_str;
    ssize_t bytes_received = recv(pimpl->sockfd_, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        response_str.assign(buffer, bytes_received);
    } else if (bytes_received == 0) {
        pimpl->sockfd_ = -1; // Mark connection as dead
        throw TissuQueryException("Server closed the connection during read.");
    } else {
        perror("recv");
        pimpl->sockfd_ = -1; // Mark connection as dead
        throw TissuQueryException("Failed to receive response.");
    }

    return std::make_unique<TissuResult>(response_str);
}

std::unique_ptr<TissuResult> TissuSession::run(const std::string& query, const std::map<std::string, TissValue>& params) {
    std::string final_query = query;
    for (const auto& [key, value] : params) {
        std::string placeholder = "$" + key;
        size_t pos = final_query.find(placeholder);
        if (pos != std::string::npos) {
            final_query.replace(pos, placeholder.length(), value.toQueryString());
        }
    }
    // This is a simple implementation. A robust version would handle multiple occurrences
    // and avoid replacing substrings (e.g., if a key is "name" and another is "lastname").

    return run(final_query);
}

std::unique_ptr<TissuTransaction> TissuSession::beginTransaction() {
    run("BEGIN");
    // Use `new` because the constructor is protected. std::make_unique cannot access it.
    return std::unique_ptr<TissuTransaction>(new TissuTransaction(this));
}


// --- TissuTransaction Method Implementations ---

TissuTransaction::TissuTransaction(ISession* session)
    : session_(session), is_active_(true) {
    std::cout << "Transaction started." << std::endl;
}

TissuTransaction::~TissuTransaction() {
    if (is_active_) {
        try {
            std::cerr << "Transaction was not explicitly committed or rolled back. Rolling back automatically." << std::endl;
            rollback();
        } catch (const std::exception& e) {
            std::cerr << "Error during automatic rollback in destructor: " << e.what() << std::endl;
        }
    }
    std::cout << "Transaction destroyed." << std::endl;
}

void TissuTransaction::commit() {
    if (!is_active_) {
        throw TissuException("Transaction is not active.");
    }
    session_->run("COMMIT");
    is_active_ = false;
    std::cout << "Transaction committed." << std::endl;
}

void TissuTransaction::rollback() {
    if (!is_active_) {
        throw TissuException("Transaction is not active.");
    }
    session_->run("ROLLBACK");
    is_active_ = false;
    std::cout << "Transaction rolled back." << std::endl;
}


// --- TissuClient Method Implementations ---

TissuClient::TissuClient() : pimpl(nullptr) {}

TissuClient::~TissuClient() {
    std::cout << "TissuClient destroyed." << std::endl;
}

std::unique_ptr<TissuClient> TissuClient::create(const TissuConfig& config) {
    auto client = std::unique_ptr<TissuClient>(new TissuClient());
    try {
        client->pimpl = std::make_unique<TissuClientImpl>(config);
    } catch (const TissuException& e) { // Catch our specific exception type
        std::cerr << "Failed to initialize TissuClient: " << e.what() << std::endl;
        return nullptr;
    }
    return client;
}

std::unique_ptr<ISession> TissuClient::getSession() {
    if (!pimpl) {
        throw TissuException("Client is not initialized.");
    }
    int sockfd = pimpl->getConnection();
    return std::unique_ptr<TissuSession>(new TissuSession(sockfd, pimpl.get()));
}

} // namespace tissudb
