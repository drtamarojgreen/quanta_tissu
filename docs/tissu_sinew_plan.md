# Tissu Sinew: A Comprehensive Guide to the JDK Middle Layer for TissDB

## 1. Introduction

Tissu Sinew is a high-performance, modern, and intuitive JDK middle layer for TissDB. It provides a native Java interface for developers to interact with the TissDB data store, abstracting away the complexities of the underlying wire protocol and data formats. This guide provides all the necessary details for a developer to effectively use Tissu Sinew in their applications.

---

## 2. Architecture

Tissu Sinew acts as a bridge between your Java application and the TissDB server. Its architecture is designed for performance, scalability, and ease of use.

### 2.1. Architectural Diagram

```
+-----------------------------+      +-------------------------+      +------------------+
|      Java Application       |      |      Tissu Sinew        |      |   TissDB Server  |
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
|                             |      |  +-------------------+  |      |                  |
|                             |      |  |   Query Engine    |  |      |                  |
|                             |      |  | (TissQL Builder)  |  |      |                  |
|                             |      |  +-------------------+  |      |                  |
|                             |      |            |            |      |                  |
|                             |      |  +-------------------+  |      |      TCP/IP      |
|                             |      |  |   Protocol Layer  |<------>|  (Wire Protocol) |
|                             |      |  | (Serialization)   |  |      |                  |
|                             |      |  +-------------------+  |      |                  |
|                             |      |                         |      |                  |
+-----------------------------+      +-------------------------+      +------------------+
```

### 2.2. Key Components

*   **Public API (`TissuClient`, `TissuSession`):** This is the main entry point for your application. It provides a clean and intuitive interface for all database operations.
*   **Connection Pool:** Manages a pool of persistent connections to the TissDB server to reduce connection overhead and improve performance. It is highly configurable to suit different application needs.
*   **Query Engine:** Includes a powerful TissQL query builder that allows you to construct complex queries programmatically, reducing the risk of syntax errors and injection attacks. It also handles the parsing of results into Java objects.
*   **Protocol Layer:** This is the low-level component that handles the communication with the TissDB server. It manages the TissDB wire protocol, including request serialization and response deserialization.

---

## 3. Getting Started

To start using Tissu Sinew, you need to add it as a dependency to your project.

### 3.1. Maven Dependency

Add the following to your `pom.xml`:

```xml
<dependency>
    <groupId>com.tissudb</groupId>
    <artifactId>tissu-sinew</artifactId>
    <version>1.0.0</version>
</dependency>
```

### 3.2. Gradle Dependency

Add the following to your `build.gradle`:

```groovy
implementation 'com.tissudb:tissu-sinew:1.0.0'
```

### 3.3. First Connection

Here's how to establish your first connection and run a simple command:

```java
import com.tissudb.sinew.TissuClient;
import com.tissudb.sinew.TissuSession;
import com.tissudb.sinew.config.TissuConfig;
import com.tissudb.sinew.results.TissuResult;

public class Main {
    public static void main(String[] args) {
        // 1. Configure the client
        TissuConfig config = TissuConfig.builder()
                .withHost("localhost")
                .withPort(9876)
                .withCredentials("user", "password")
                .build();

        // 2. Create a client (this manages the connection pool)
        try (TissuClient client = TissuClient.create(config)) {
            // 3. Get a session from the client
            try (TissuSession session = client.getSession()) {
                // 4. Run a query
                TissuResult result = session.run("PING");
                System.out.println(result.asString()); // Expected output: PONG
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
```

---

## 4. API Reference

### 4.1. `TissuClient`

The `TissuClient` is a thread-safe object that manages the connection pool to the TissDB server. It should be created once per application and reused.

**`static TissuClient create(TissuConfig config)`**
Creates a new client instance with the given configuration.

**`TissuSession getSession()`**
Acquires a `TissuSession` from the connection pool. This is the primary way to interact with the database.

**`void close()`**
Closes the client and all its connections gracefully.

### 4.2. `TissuSession`

A `TissuSession` represents a single logical thread of execution. It is **not thread-safe** and should be used within a single thread. Use `try-with-resources` to ensure it is always closed correctly.

**`TissuResult run(String query)`**
Runs a raw TissQL query.

**`TissuResult run(String query, Map<String, Object> parameters)`**
Runs a parameterized TissQL query. This is the recommended way to run queries to prevent injection attacks.

**`TissuTransaction beginTransaction()`**
Starts a new transaction.

**`void close()`**
Releases the session back to the connection pool.

### 4.3. `TissuTransaction`

Provides methods to manage a transaction. Also not thread-safe.

**`void commit()`**
Commits the transaction.

**`void rollback()`**
Rolls back the transaction.

**`TissuResult run(String query, Map<String, Object> parameters)`**
Runs a query within the context of the transaction.

---

## 5. Usage Examples

### 5.1. Inserting Data

```java
try (TissuSession session = client.getSession()) {
    Map<String, Object> params = new HashMap<>();
    params.put("id", 123);
    params.put("name", "John Doe");
    params.put("email", "john.doe@example.com");

    String query = "CREATE (p:Person {id: $id, name: $name, email: $email})";
    session.run(query, params);
    System.out.println("Person created.");
}
```

### 5.2. Querying Data

```java
try (TissuSession session = client.getSession()) {
    String query = "MATCH (p:Person) WHERE p.name = $name RETURN p";
    Map<String, Object> params = Collections.singletonMap("name", "John Doe");

    TissuResult result = session.run(query, params);

    // Iterate over records
    for (TissuRecord record : result.asList()) {
        // Access fields by name
        String name = record.get("p.name").asString();
        String email = record.get("p.email").asString();
        System.out.println("Found person: " + name + " (" + email + ")");
    }
}
```

### 5.3. Using Transactions

```java
try (TissuSession session = client.getSession();
     TissuTransaction tx = session.beginTransaction()) {

    // First operation
    tx.run("CREATE (a:Account {id: 'A', balance: 1000})");

    // Second operation
    tx.run("CREATE (b:Account {id: 'B', balance: 500})");

    // Transfer funds
    tx.run("MATCH (a:Account {id: 'A'}), (b:Account {id: 'B'}) SET a.balance = a.balance - 100, b.balance = b.balance + 100");

    tx.commit();
    System.out.println("Transaction committed successfully.");
} catch (TissuException e) {
    // Transaction will be rolled back automatically on session close if not committed
    System.err.println("Transaction failed: " + e.getMessage());
}
```

---

## 6. Data Type Mapping

Tissu Sinew automatically handles the mapping between Java data types and TissDB's native types.

| Java Type           | TissDB Type |
|---------------------|-------------|
| `String`            | `STRING`    |
| `long`, `Long`      | `INTEGER`   |
| `int`, `Integer`    | `INTEGER`   |
| `double`, `Double`  | `FLOAT`     |
| `boolean`, `Boolean`| `BOOLEAN`   |
| `List<T>`           | `LIST`      |
| `Map<String, T>`    | `MAP`       |
| `LocalDate`         | `DATE`      |
| `ZonedDateTime`     | `DATETIME`  |
| `byte[]`            | `BYTES`     |

---

## 7. Concurrency and Threading

-   **`TissuClient` is thread-safe.** Create one instance and share it across your application.
-   **`TissuSession` and `TissuTransaction` are NOT thread-safe.** Do not share them between threads. Each thread should acquire its own session from the client.
-   The connection pool handles the underlying physical connections, which are thread-safe and managed by the library. You do not need to worry about managing them directly.

---

## 8. Error Handling

Tissu Sinew uses a hierarchy of custom exceptions, all inheriting from `TissuException`.

-   **`TissuConnectionException`:** Thrown when there is an issue connecting to the TissDB server (e.g., network error, invalid credentials).
-   **`TissuQueryException`:** Thrown when a query is syntactically incorrect or semantically invalid.
-   **`TissuTransactionException`:** Thrown for transaction-related errors, such as trying to commit a rolled-back transaction.

Always wrap your database calls in a `try-catch` block to handle these exceptions gracefully.
