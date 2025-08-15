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
    TissValue(const char* v) : value_(std::string(v)) {}
    TissValue(std::string v) : value_(std::move(v)) {}
    TissValue(int64_t v) : value_(v) {}
    TissValue(int v) : value_(static_cast<int64_t>(v)) {}
    TissValue(double v) : value_(v) {}
    TissValue(bool v) : value_(v) {}

    // Formats the value as a string for inclusion in a query.
    std::string toQueryString() const;

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

/**
 * @brief Configuration for connecting to a TissDB server.
 */
class TissuConfig {
public:
    std::string host = "localhost";
    int port = 9876;
    std::string username;
    std::string password;
};

/**
 * @brief Represents a session with the TissDB server, used to run queries.
 * This class is not thread-safe.
 */
class ISession {
public:
    virtual ~ISession() = default;
    virtual std::unique_ptr<TissuResult> run(const std::string& query) = 0;
    virtual std::unique_ptr<TissuResult> run(const std::string& query, const std::map<std::string, TissValue>& params) = 0;
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
     * @brief Runs a parameterized query.
     * @param query The TissQL query string with placeholders (e.g., $name).
     * @param params A map of parameter names to their values.
     * @return A unique pointer to a TissuResult object.
     */
    std::unique_ptr<TissuResult> run(const std::string& query, const std::map<std::string, TissValue>& params) override;

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

protected:
    TissuClient();

private:
    friend class TissuSession;
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
