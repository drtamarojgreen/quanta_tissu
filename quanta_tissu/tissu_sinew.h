#ifndef TISSU_SINEW_H
#define TISSU_SINEW_H

#include <string>
#include <vector>
#include <map>
#include <memory> // For std::unique_ptr

// Forward declarations
namespace tissudb {
    class TissuSession;
    class TissuConfig;
}

namespace tissudb {

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
class TissuSession {
public:
    // TissuSession should not be copyable or movable.
    TissuSession(const TissuSession&) = delete;
    TissuSession& operator=(const TissuSession&) = delete;

    /**
     * @brief Runs a query.
     * @param query The TissQL query string to execute.
     */
    void run(const std::string& query);

    ~TissuSession();

    // In a real implementation, this would return a result object.
    // void run(const std::string& query, const std::map<std::string, ValueType>& params);

protected:
    // Only TissuClient should be able to create sessions.
    friend class TissuClient;
    TissuSession(int sockfd); // Constructor now takes a socket descriptor

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
     * @brief Gets a session for interacting with the database.
     * @return A unique pointer to a new TissuSession.
     */
    std::unique_ptr<TissuSession> getSession();

    ~TissuClient();

protected:
    TissuClient();

private:
    // Private implementation details would go here (e.g., connection pool)
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace tissudb

#endif // TISSU_SINEW_H
