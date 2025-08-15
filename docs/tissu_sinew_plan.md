# Tissu Sinew: A C++ Connector for TissDB

## 1. Introduction

Tissu Sinew is a high-performance, lightweight C++ connector for TissDB. It provides a native C++ interface for developers to interact with the TissDB data store, abstracting away the complexities of the underlying socket communication and protocol. It is designed with a modern C++ approach, using smart pointers for resource management and providing a thread-safe client for connection pooling.

This guide provides the necessary details for a developer to use Tissu Sinew in their C++ applications.

---

## 2. Architecture

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

## 3. Getting Started

To use Tissu Sinew, you need to include its header and link against the compiled library or object files.

### 3.1. Compilation

You need to compile `tissu_sinew.cpp` and your application code, and then link them together.

**Example (using g++):**

First, compile the library object file:
```bash
g++ -std=c++17 -c quanta_tissu/tissu_sinew.cpp -o tissu_sinew.o
```

Then, compile your application and link it with the library:
```bash
g++ -std=c++17 -I. my_app.cpp tissu_sinew.o -o my_app -lpthread
```
*(Note: `-I.` assumes `my_app.cpp` is in the root directory and can find `quanta_tissu/tissu_sinew.h`. You may need to adjust include paths. `-lpthread` is required for multi-threading support.)*

### 3.2. First Connection

Here's how to establish your first connection and run a simple command. This example is based on `quanta_tissu/scripts/test_sinew_app.cpp`.

```cpp
#include "quanta_tissu/tissu_sinew.h"
#include <iostream>
#include <memory>

int main() {
    // 1. Configure the client
    tissudb::TissuConfig config;
    config.host = "127.0.0.1";
    config.port = 8080; // Ensure this matches your TissDB server port

    // 2. Create a client, which manages the connection pool.
    // The client is a unique_ptr, ensuring cleanup.
    std::unique_ptr<tissudb::TissuClient> client = tissudb::TissuClient::create(config);

    if (!client) {
        std::cerr << "Failed to create TissuClient. Exiting." << std::endl;
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

## 4. API Reference

### 4.1. `TissuClient`

The `TissuClient` is a thread-safe object that manages the connection pool. It should be created once per application and shared.

**`static std::unique_ptr<TissuClient> create(const TissuConfig& config)`**
Creates a new client instance. Returns `nullptr` on failure.

**`std::unique_ptr<ISession> getSession()`**
Acquires a session from the connection pool. This is the primary way to interact with the database.

### 4.2. `ISession` / `TissuSession`

A `TissuSession` represents a single logical thread of execution. It is **not thread-safe**. Use a separate session per thread.

**`std::unique_ptr<TissuResult> run(const std::string& query)`**
Runs a raw TissQL query.

**`std::unique_ptr<TissuResult> run(const std::string& query, const std::map<std::string, TissValue>& params)`**
Runs a parameterized TissQL query.

**`std::unique_ptr<TissuTransaction> beginTransaction()`**
Starts a new transaction.

### 4.3. `TissuTransaction`

Provides RAII-based transaction management.

**`void commit()`**
Commits the transaction.

**`void rollback()`**
Rolls back the transaction. If neither `commit` nor `rollback` is called, the destructor will automatically roll back the transaction.

---

## 5. Concurrency and Threading

-   **`TissuClient` is thread-safe.** Create one instance and share it across your application threads.
-   **`TissuSession` and `TissuTransaction` are NOT thread-safe.** Do not share them between threads. Each thread should acquire its own session from the client.
-   The connection pool handles the underlying socket connections, which are managed by the `TissuClient`.

---

## 6. Error Handling

Tissu Sinew uses a hierarchy of custom exceptions, all inheriting from `std::runtime_error`.

-   **`TissuException`**: Base class for all library errors.
-   **`TissuConnectionException`**: Thrown for issues connecting to the TissDB server.
-   **`TissuQueryException`**: Thrown for errors during query execution (e.g., send/receive failed).

Always wrap your database calls in a `try-catch` block to handle these exceptions.
