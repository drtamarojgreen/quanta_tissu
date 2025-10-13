# Development Plan for tisslm_cpp (Tissu Sinew)

## 1. Project Overview

The `tisslm_cpp` project is a C++ client library, named "Tissu Sinew," designed to interact with a TissDB database. It provides a high-level API for managing connections, sessions, and transactions. The library also includes components for parsing Data Definition Language (DDL) statements, suggesting an embedded or tightly-coupled database system.

The current codebase is a well-structured skeleton with a clear separation of concerns:
- **`tissu_sinew.h/.cpp`**: Defines the public API for the client, including connection configuration, sessions, and transaction management. It uses the Pimpl idiom to hide implementation details.
- **`ddl_parser.h/.cpp`**: Defines the Abstract Syntax Tree (AST) for DDL statements (CREATE, DROP, ALTER) and a placeholder parser class.
- **`schema_manager.h/.cpp`**: (Assumed from filenames) Likely responsible for managing the database schema using the DDL parser.
- **`main.cpp`**: Provides a simple example demonstrating how to use the Tissu Sinew client.

## 2. Architecture and Design

The current architecture is sound and should be maintained. Key design patterns in use are:
- **Factory Method (`TissuClient::create`)**: Simplifies object creation for the client, providing a single point of entry.
- **Pimpl (Pointer to Implementation)**: Used in `TissuClient` and `TissuSession` to decouple the interface from the implementation. This is crucial for hiding complex third-party dependencies (e.g., networking libraries) and can help reduce compilation times for client code.
- **RAII (Resource Acquisition Is Initialization)**: `std::unique_ptr` is used effectively to manage the lifecycle of `TissuClient` and `TissuSession` objects, preventing resource leaks. The `TissuTransaction` class also follows this pattern to ensure transactions are automatically rolled back if not explicitly committed.

## 3. Build System

A robust build system is critical for managing the project. **CMake** is the recommended choice due to its cross-platform nature, widespread adoption in the C++ community, and excellent support for managing dependencies and build configurations.

A `CMakeLists.txt` file should be created in `quanta_tissu/tisslm/program/` with the following structure:
- Define the project name, version, and required C++ standard (C++17 is a good baseline given the use of `std::variant` and `std::unique_ptr`).
- Create a `tissu_sinew` library target, compiling `tissu_sinew.cpp`, `ddl_parser.cpp`, and `schema_manager.cpp`.
- Create an executable target for the example in `main.cpp` and link it against the `tissu_sinew` library.
- Add an option to build with tests enabled (e.g., a CMake `BUILD_TESTING` option).
- Use `find_package` to locate and link external dependencies like networking libraries.

## 4. Development Roadmap

Development should proceed in the following phases:

### Phase 1: Core Implementation & Testing Framework
- **Implement the DDL Parser**: Flesh out the `DDLParser::parse` method. A hand-written recursive descent parser is likely sufficient for the defined DDL scope.
- **Implement the Schema Manager**: Implement the `SchemaManager` to use the `DDLParser` and manage an in-memory or on-disk representation of the database schema.
- **Set up Testing Framework**: Integrate a testing framework like GoogleTest or Catch2.
- **Write Unit Tests**:
  - Write extensive tests for the `DDLParser` covering valid syntax, edge cases, and invalid syntax.
  - Write tests for the `SchemaManager` to ensure correct schema manipulation (creating, dropping, altering tables).

### Phase 2: Networking and Protocol Implementation
- **Choose and Integrate a Networking Library**: Select a suitable networking library. Boost.Asio or the standalone Asio library are excellent choices. The implementation details should be entirely contained within the `TissuClientImpl` and `TissuSession::Impl` classes.
- **Implement Connection Logic**: Implement the client connection and reconnection logic within `TissuClientImpl`.
- **Implement Session Protocol**: Implement the request/response protocol for sending queries and receiving results within `TissuSession::Impl`. This will involve defining a serialization format for queries/commands and a deserialization format for results.
- **Implement Connection Pooling**: Implement the connection pool within `TissuClientImpl` to manage a set of active connections to the database, which can be handed out as sessions.

### Phase 3: Feature Enhancement and Refinement
- **Full Query Result Handling**: Extend `TissuResult` to handle complex, structured data returned from the database, not just a simple string. This could involve representing rows, columns, and different data types.
- **Asynchronous API**: Introduce an asynchronous API (e.g., using `std::future` or callbacks/coroutines) for non-blocking database operations, which is critical for high-performance applications.
- **Logging Enhancement**: Improve the `ILogger` interface and provide more concrete logging implementations (e.g., a file-based logger).
- **Error Handling**: Refine the exception hierarchy (`TissuException`) to provide more specific and detailed error information for connection, query, and transaction failures.

### Phase 4: Documentation and Packaging
- **API Documentation**: Use Doxygen to generate comprehensive API documentation from the source code comments.
- **User Guide**: Write a markdown-based user guide with detailed examples and best practices for using the library.
- **Packaging**: Use CPack (the CMake packaging tool) to create distributable packages for different platforms (e.g., `.deb`, `.rpm`, `.zip`).

## 5. Dependencies

- **Build**: CMake (version 3.10+)
- **Testing**: GoogleTest (recommended)
- **Networking**: Asio (standalone or via Boost)
- **(Optional) Parsing**: ANTLR, if a more complex grammar is required in the future.