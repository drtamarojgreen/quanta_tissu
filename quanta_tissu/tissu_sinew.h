#ifndef TISSU_SINEW_H
#define TISSU_SINEW_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <memory> // For std::unique_ptr
#include <mutex>
#include <condition_variable>
#include <queue>
#include <variant>
#include <iostream> // For StdLogger

// Forward declarations
namespace tissudb {
    class TissuSession;
    class TissuConfig;
    class TissuResult;
    class TissuClient;
    class TissuClientImpl; // Opaque pointer type for the implementation
    class TissuTransaction;
}

namespace tissudb {

/**
 * @brief Enum for tagging parameter types in the binary protocol.
 */
enum class TissParamType : uint8_t {
    NULL_TYPE = 0x00,
    STRING    = 0x01,
    INT64     = 0x02,
    FLOAT64   = 0x03, // double
    BOOL      = 0x04
};

/**
 * @brief Base exception for all Tissu Sinew errors.
 */
class TissuException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Exception for errors related to network connections.
 */
class TissuConnectionException : public TissuException {
public:
    using TissuException::TissuException;
};

/**
 * @brief Exception for errors related to query syntax or execution.
 */
class TissuQueryException : public TissuException {
public:
    using TissuException::TissuException;
};


/**
 * @brief A variant type to hold different TissDB-compatible values.
 */
class TissValue {
public:
    using ValueType = std::variant<std::nullptr_t, std::string, int64_t, double, bool>;

    // Constructors
    TissValue() : value_(nullptr) {}
    TissValue(std::nullptr_t) : value_(nullptr) {}
    TissValue(const char* v) {
        if (v) {
            value_ = std::string(v);
        } else {
            value_ = nullptr;
        }
    }
    TissValue(std::string v) : value_(std::move(v)) {}
    TissValue(int64_t v) : value_(v) {}
    TissValue(int v) : value_(static_cast<int64_t>(v)) {}
    TissValue(double v) : value_(v) {}
    TissValue(bool v) : value_(v) {}

    // Formats the value as a string for inclusion in a query.
    std::string toQueryString() const;

    const ValueType& getValue() const { return value_; }

private:
    ValueType value_;
};

/**
 * @brief Represents the result of a query execution.
 */
class TissuResult {
public:
    TissuResult(std::string value) : value_(std::move(value)) {}

    /**
     * @brief Returns the result as a string.
     */
    const std::string& asString() const { return value_; }

private:
    std::string value_;
};

// --- Logging ---

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
};

class NullLogger : public ILogger {
public:
    void info(const std::string& message) override {}
    void error(const std::string& message) override {}
};

class StdLogger : public ILogger {
public:
    void info(const std::string& message) override {
        std::cout << "[INFO] " << message << std::endl;
    }
    void error(const std::string& message) override {
        std::cerr << "[ERROR] " << message << std::endl;
    }
};


/**
 * @brief Configuration for connecting to a TissDB server.
 */
class TissuConfig {
public:
    std::string host = "localhost";
    int port = 9876;
    std::string username;
    std::string password;
    size_t pool_size = 5;
    int connect_timeout_ms = 5000; // milliseconds
    std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>();
};

/*
 * =====================================================================================
 *       Secure Parameterized Query Protocol Definition
 * =====================================================================================
 *
 * The client sends a single binary message to the server for a parameterized query.
 * All multi-byte integers are in network byte order (big-endian).
 *
 * The message has the following structure:
 *
 * [Total Body Length] - 4 bytes, uint32_t. This is the total length of the
 *                       message body that follows this field.
 *     |
 *     +---- [Query String Length] - 4 bytes, uint32_t (N)
 *     |
 *     +---- [Query String] - N bytes, UTF-8 encoded string
 *     |      (e.g., "INSERT INTO users (name, age) VALUES ($1, $2)")
 *     |
 *     +---- [Parameter Count] - 1 byte, uint8_t (P)
 *     |
 *     +---- [Parameter 1]
 *     |      |
 *     |      +--- [Type] - 1 byte, uint8_t (see TissParamType enum)
 *     |      |
 *     |      +--- [Value Length] - 4 bytes, uint32_t (L1)
 *     |      |
 *     |      +--- [Value] - L1 bytes, the parameter's binary data
 *     |
 *     +---- [Parameter 2]
 *     |      |
 *     |      +--- [Type] - 1 byte, uint8_t
 *     |      |
 *     |      +--- [Value Length] - 4 bytes, uint32_t (L2)
 *     |      |
 *     |      +--- [Value] - L2 bytes
 *     |
 *     ... (up to P parameters)
 *
 * Note: The server is expected to handle the mapping of placeholders like $1, $2, etc.
 * to the positional parameters sent in the message. The client is responsible for
 * ensuring the number of placeholders matches the number of parameters.
 *
 */

/**
 * @brief Represents a session with the TissDB server, used to run queries.
 * This class is not thread-safe.
 */
class ISession {
public:
    virtual ~ISession() = default;
    virtual std::unique_ptr<TissuResult> run(const std::string& query) = 0;
    virtual std::unique_ptr<TissuResult> run(const std::string& query, const std::vector<TissValue>& params) = 0;

    // DEPRECATED: This method uses client-side string substitution and is vulnerable to SQL injection.
    virtual std::unique_ptr<TissuResult> run_with_client_side_substitution(const std::string& query, const std::map<std::string, TissValue>& params) = 0;
    virtual std::unique_ptr<TissuTransaction> beginTransaction() = 0;
};

class TissuSession : public ISession {
public:
    // TissuSession should not be copyable or movable.
    TissuSession(const TissuSession&) = delete;
    TissuSession& operator=(const TissuSession&) = delete;

    /**
     * @brief Runs a query and returns the result.
     * @param query The TissQL query string to execute.
     * @return A unique pointer to a TissuResult object.
     */
    std::unique_ptr<TissuResult> run(const std::string& query) override;

    /**
     * @brief Runs a secure parameterized query using the binary protocol.
     * @param query The TissQL query string with positional placeholders (e.g., $1, $2).
     * @param params A vector of parameter values.
     * @return A unique pointer to a TissuResult object.
     */
    std::unique_ptr<TissuResult> run(const std::string& query, const std::vector<TissValue>& params) override;

    /**
     * @brief Runs a parameterized query.
     * @param query The TissQL query string with placeholders (e.g., $name).
     * @param params A map of parameter names to their values.
     * @return A unique pointer to a TissuResult object.
     */
    std::unique_ptr<TissuResult> run_with_client_side_substitution(const std::string& query, const std::map<std::string, TissValue>& params) override;

    /**
     * @brief Begins a new transaction.
     * @return A unique pointer to a new TissuTransaction object.
     */
    std::unique_ptr<TissuTransaction> beginTransaction() override;

    ~TissuSession() override;

    // In a real implementation, this would return a result object.
    // void run(const std::string& query, const std::map<std::string, ValueType>& params);

protected:
    // Only TissuClient should be able to create sessions.
    friend class TissuClient;
    // Constructor now takes a socket descriptor and a pointer to the client's implementation
    TissuSession(int sockfd, TissuClientImpl* client_impl);

    // Protected virtual method for testability. Allows mocking of the raw send/receive.
    virtual std::unique_ptr<TissuResult> send_and_receive_raw(const std::vector<char>& message_buffer);

private:
    class Impl; // Private implementation
    std::unique_ptr<Impl> pimpl;
};

/**
 * @brief The main client for interacting with TissDB.
 * Manages connections and provides sessions. This class is thread-safe.
 */
class TissuClient {
public:
    /**
     * @brief Creates a new TissuClient with the given configuration.
     * @param config The configuration settings.
     * @return A unique pointer to the new TissuClient instance.
     */
    static std::unique_ptr<TissuClient> create(const TissuConfig& config);

    // TissuClient should not be copyable or movable.
    TissuClient(const TissuClient&) = delete;
    TissuClient& operator=(const TissuClient&) = delete;

    /**
     * @brief Gets a session for interacting with the database from the pool.
     * @return A unique pointer to a new ISession.
     */
    std::unique_ptr<ISession> getSession();

    ~TissuClient();

private:
    // Private constructor for use by the `create` factory method.
    explicit TissuClient(std::unique_ptr<TissuClientImpl> pimpl);

    friend class TissuSession; // TissuSession needs to access TissuClientImpl
    std::unique_ptr<TissuClientImpl> pimpl;
};


/**
 * @brief Represents a transaction, ensuring it is either committed or rolled back.
 */
class TissuTransaction {
public:
    // Non-copyable and non-movable
    TissuTransaction(const TissuTransaction&) = delete;
    TissuTransaction& operator=(const TissuTransaction&) = delete;
    TissuTransaction(TissuTransaction&&) = delete;
    TissuTransaction& operator=(TissuTransaction&&) = delete;

    /**
     * @brief Commits the transaction.
     */
    void commit();

    /**
     * @brief Rolls back the transaction.
     */
    void rollback();

    /**
     * @brief Destructor that ensures rollback if the transaction is still active.
     */
    ~TissuTransaction();

protected:
    friend class TissuSession;
    explicit TissuTransaction(ISession* session);

private:
    ISession* session_;
    bool is_active_;
};

} // namespace tissudb

#endif // TISSU_SINEW_H
