# Tissu Sinew: A Comprehensive Guide to the TissDB C++ Connector

Tissu Sinew is a high-performance, lightweight C++ connector for TissDB. It provides a native C++ interface for developers to interact with the TissDB data store, abstracting away the complexities of the underlying socket communication and protocol. It is designed with a modern C++ approach, using smart pointers for resource management and providing a thread-safe client for connection pooling.

This guide provides everything you need to know to use Tissu Sinew in your C++ applications.

## 1. Architecture

Tissu Sinew acts as a bridge between your C++ application and the TissDB server.

```
+-----------------------------+      +-------------------------+      +------------------+
|      C++ Application        |      |      Tissu Sinew        |      |   TissDB Server  |
|-----------------------------|      |-------------------------|      |------------------|
|                             |      |                         |      |                  |
|  +-----------------------+  |      |  +-------------------+  |      |                  |
|  |   Your Business Logic |<------>|  |    Public API     |  |      |                  |
|  +-----------------------+  |      |  | (TissuClient,    |  |      |                  |
|                             |      |  |  TissuSession)    |  |      |                  |
|                             |      |  +-------------------+  |      |                  |
|                             |      |            |            |      |                  |
|                             |      |  +-------------------+  |      |                  |
|                             |      |  | Connection Pool   |  |      |                  |
|                             |      |  +-------------------+  |      |                  |
|                             |      |            |            |      |                  |
|                             |      |  +-------------------+  |      |      TCP/IP      |
|                             |      |  |   Socket Layer    |<------>|  (Raw Sockets)   |
|                             |      |  +-------------------+  |      |                  |
|                             |      |                         |      |                  |
+-----------------------------+      +-------------------------+      +------------------+
```

### Key Components

*   **Public API (`TissuClient`, `TissuSession`):** The main entry point for your application. It provides a clean, RAII-compliant interface for all database operations.
*   **Connection Pool:** The `TissuClient` manages a pool of persistent socket connections to the TissDB server to reduce connection overhead and improve performance in multi-threaded environments.
*   **Socket Layer:** The low-level component that handles direct TCP socket communication with the TissDB server.

---

## 2. Building and Linking

To use Tissu Sinew, you need to compile `tissu_sinew.cpp` and link it with your application.

### 2.1. Dependencies
- A C++17 compatible compiler (e.g., g++, clang)
- The pthreads library for multi-threading support.

### 2.2. Compilation Example

First, compile the Tissu Sinew source file into an object file:
```bash
g++ -std=c++17 -c quanta_tissu/tissu_sinew.cpp -o tissu_sinew.o -I.
```
*Note: The `-I.` flag tells the compiler to look for headers in the current directory, which is necessary to find `quanta_tissu/tissu_sinew.h`.*

Next, compile your application and link it with the `tissu_sinew.o` object file:
```bash
g++ -std=c++17 my_app.cpp tissu_sinew.o -o my_app -lpthread
```
You can now run your application: `./my_app`.

---

## 3. Getting Started

Here is a simple example of how to connect to TissDB, run a command, and handle potential errors.

```cpp
#include "quanta_tissu/tissu_sinew.h"
#include <iostream>
#include <memory>

int main() {
    // 1. Configure the client
    tissudb::TissuConfig config;
    config.host = "127.0.0.1";
    config.port = 8080; // Ensure this matches your TissDB server port
    config.pool_size = 5; // Max number of connections in the pool

    // 2. Create a client, which manages the connection pool.
    std::unique_ptr<tissudb::TissuClient> client = tissudb::TissuClient::create(config);

    if (!client) {
        std::cerr << "Failed to create TissuClient. Is the server running?" << std::endl;
        return 1;
    }

    try {
        // 3. Get a session from the client.
        // The session is a unique_ptr, ensuring the connection is returned to the pool
        // when it goes out of scope (RAII).
        std::unique_ptr<tissudb::ISession> session = client->getSession();

        // 4. Run a query
        std::unique_ptr<tissudb::TissuResult> result = session->run("PING");

        if (result) {
            std::cout << "Response from server: " << result->asString() << std::endl;
        }

    } catch (const tissudb::TissuException& e) {
        std::cerr << "A database error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

---

## 4. Core Concepts

### TissuClient
The `TissuClient` is the main entry point for the library. It is a thread-safe object that manages the connection pool. **You should create only one `TissuClient` instance per application and share it across all threads.**

### TissuSession
A `TissuSession` represents a single logical connection to the database. It is acquired from the `TissuClient`'s connection pool. **`TissuSession` is not thread-safe.** Each thread that needs to interact with the database must acquire its own session.

### Connection Pooling
Tissu Sinew uses a connection pool to manage connections to TissDB. When you call `client->getSession()`, you are given an available connection from the pool. If no connections are available, the call will block until one is freed. When the `TissuSession` object goes out of scope, its destructor automatically returns the connection to the pool. This makes resource management safe and efficient.

---

## 5. API Reference

### `TissuConfig`
A struct used to configure the `TissuClient`.
```cpp
struct TissuConfig {
    std::string host = "localhost";
    int port = 9876;
    size_t pool_size = 5;
    int connect_timeout_ms = 5000;
    std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>();
};
```

### `TissuClient`
The thread-safe client manager.
- `static std::unique_ptr<TissuClient> create(const TissuConfig& config)`: Factory method to create a client.
- `std::unique_ptr<ISession> getSession()`: Acquires a session from the pool.

### `ISession`
The interface for a database session.
- `virtual std::unique_ptr<TissuResult> run(const std::string& query) = 0`: Executes a raw TissQL query.
- `virtual std::unique_ptr<TissuResult> run(const std::string& query, const std::map<std::string, TissValue>& params) = 0`: Executes a parameterized query.
- `virtual std::unique_ptr<TissuTransaction> beginTransaction() = 0`: Starts a new transaction.

### `TissuResult`
Represents the result of a query.
- `const std::string& asString() const`: Returns the raw string result from the server.

### `TissValue`
A variant-like class to hold values for parameterized queries. It can hold `std::string`, `int64_t`, `double`, `bool`, and `nullptr_t`.

---

## 6. Usage Examples

### Parameterized Queries
Using parameterized queries is the recommended way to pass data to the database, as it prevents SQL injection vulnerabilities. Parameters are specified in the query string with a `$` prefix (e.g., `$name`).

```cpp
#include "quanta_tissu/tissu_sinew.h"
#include <iostream>
#include <map>

// ... (client setup code) ...

try {
    auto session = client->getSession();

    std::map<std::string, tissudb::TissValue> params;
    params["name"] = "John Doe";
    params["age"] = 30;

    session->run("INSERT INTO users (name, age) VALUES ($name, $age)", params);

    std::cout << "User inserted successfully." << std::endl;

} catch (const tissudb::TissuException& e) {
    std::cerr << "Error inserting user: " << e.what() << std::endl;
}
```

### Transactions
Transactions allow you to execute a group of commands as a single atomic unit. The `TissuTransaction` object uses RAII: if it goes out of scope without `commit()` being called, it will automatically roll back.

```cpp
#include "quanta_tissu/tissu_sinew.h"
#include <iostream>

// ... (client setup code) ...

try {
    auto session = client->getSession();

    // beginTransaction() returns a unique_ptr, ensuring RAII.
    auto tx = session->beginTransaction(); // Sends "BEGIN" to the server

    session->run("UPDATE accounts SET balance = balance - 100 WHERE id = 1");
    session->run("UPDATE accounts SET balance = balance + 100 WHERE id = 2");

    tx->commit(); // Sends "COMMIT"

    std::cout << "Transaction committed successfully." << std::endl;

} catch (const tissudb::TissuException& e) {
    std::cerr << "Transaction failed: " << e.what() << std::endl;
    // The transaction will be automatically rolled back here as the 'tx' object
    // is destroyed during stack unwinding.
}
```

---

## 7. Concurrency
The `TissuClient` is designed to be shared across multiple threads. Each thread must acquire its own `TissuSession` to ensure thread safety.

```cpp
#include "quanta_tissu/tissu_sinew.h"
#include <iostream>
#include <vector>
#include <thread>

void worker(tissudb::TissuClient& client, int thread_id) {
    try {
        // Each thread gets its own session
        auto session = client.getSession();
        auto result = session->run("PING");
        std::cout << "[Thread " << thread_id << "] Response: " << result->asString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Thread " << thread_id << "] Error: " << e.what() << std::endl;
    }
}

int main() {
    tissudb::TissuConfig config;
    auto client = tissudb::TissuClient::create(config);
    if (!client) return 1;

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(worker, std::ref(*client), i + 1);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
```

---

## 8. Error Handling

Tissu Sinew reports errors by throwing exceptions. All library-specific exceptions inherit from `tissudb::TissuException`.

- **`TissuException`**: The base class for all Tissu Sinew exceptions.
- **`TissuConnectionException`**: Thrown for network-related issues (e.g., cannot connect to the server, connection lost).
- **`TissuQueryException`**: Thrown for errors that occur during query execution after a connection has been established.

It is crucial to wrap all database interactions in `try...catch` blocks to handle these potential errors gracefully.
