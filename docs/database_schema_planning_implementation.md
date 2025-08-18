# Roadmap: Implementing Multi-Database Support in TissDB

## 1. Introduction & Goal

This document outlines a strategic roadmap for evolving TissDB from its current single-database architecture to a robust multi-database system. The primary goal is to enable true multi-tenancy, allowing TissDB to manage multiple, isolated databases, each with its own set of collections, schemas, and data.

This enhancement is a critical step towards making TissDB a more scalable and versatile database solution, capable of serving diverse applications and users simultaneously from a single server instance.

## 2. Current Architecture Analysis

A thorough analysis of the current codebase reveals a single-database design centered around the `LSMTree` class:

-   **Single Database Instance:** The `main.cpp` entry point instantiates a single `TissDB::Storage::LSMTree` object. This object represents the entire database.
-   **Database as Collection Manager:** The `LSMTree` class acts as a container for multiple "collections." It manages these collections in an in-memory map (`std::map<std::string, std::unique_ptr<Collection>>`).
-   **Hardcoded Injection:** The single `LSMTree` instance is injected directly into the `TissDB::API::HttpServer`, making the entire API layer operate on the assumption of a single, globally available database.
-   **Database-Agnostic API:** API endpoints do not specify a database. URLs are structured as `/<collection>/<id>`, which implicitly targets the one and only database instance.

This architecture, while effective for a single-tenant application, does not support the isolation or management of multiple databases.

## 3. Proposed Architecture

To achieve multi-database support, we propose introducing a new top-level management layer and updating the API to be database-aware.

### 3.1. The `DatabaseManager`

A new class, `DatabaseManager`, will be introduced as the central component for managing all database instances.

-   **Responsibility:** The `DatabaseManager` will be responsible for the full lifecycle of databases: creation, deletion, and retrieval.
-   **Structure:** It will maintain a map of database names to their corresponding `LSMTree` instances:
    ```cpp
    // In a new file: tissdb/storage/database_manager.h
    class DatabaseManager {
    private:
        std::map<std::string, std::unique_ptr<LSMTree>> databases_;
        std::string base_data_path_; // e.g., "tissdb_data/"
    public:
        // Methods for creating, deleting, and getting databases
        void create_database(const std::string& db_name);
        void delete_database(const std::string& db_name);
        LSMTree& get_database(const std::string& db_name);
    };
    ```
-   **Instantiation:** The `main.cpp` file will be updated to instantiate a single `DatabaseManager` instead of a single `LSMTree`.

### 3.2. API and URL Scheme Changes

The REST API must be modified to direct requests to the correct database.

-   **New URL Structure:** All data-related endpoints will be prefixed with the database name. The new structure will be: `/<database>/<collection>/...`
-   **API Server Logic:** The `HttpServer` will be modified to:
    1.  Parse the `<database>` name from the URL path.
    2.  Use the `DatabaseManager` to retrieve the appropriate `LSMTree` instance.
    3.  Perform the requested operation on the retrieved database instance.
-   **New Management Endpoints:** New top-level endpoints will be added to manage the databases themselves (e.g., `PUT /<database>` to create a new database).

## 4. Phased Implementation Plan

We recommend a three-phase approach to implement this architecture.

### Phase 1: Core Multi-Database Backend

**Objective:** Implement the foundational `DatabaseManager` and modify the storage layer to support multiple, named database instances on disk.

-   **Task 1.1: Create `DatabaseManager` Class:**
    -   Create `tissdb/storage/database_manager.h` and `.cpp`.
    -   Implement the `create_database`, `delete_database`, and `get_database` methods. `create_database` will instantiate a new `LSMTree` object and create a corresponding subdirectory for its data (e.g., `tissdb_data/sales_db/`).

-   **Task 1.2: Update `LSMTree` for Isolation:**
    -   Ensure the `LSMTree` constructor properly initializes its storage within the path provided by the `DatabaseManager`. All its file operations (WAL, SSTables) must be contained within its designated directory.

-   **Task 1.3: Update `main.cpp`:**
    -   Replace `LSMTree storage;` with `DatabaseManager db_manager("tissdb_data");`.
    -   Pass the `DatabaseManager` instance to the `HttpServer`.

### Phase 2: API Integration

**Objective:** Expose the multi-database functionality through the REST API.

-   **Task 2.1: Update `HttpServer` Routing:**
    -   Modify `HttpServer::handle_client` to parse the new `/<database>/...` URL structure.
    -   The first path segment will now be interpreted as the database name.
    -   Use `db_manager.get_database(db_name)` to retrieve the target database for the request.

-   **Task 2.2: Implement Database Management Endpoints:**
    -   `PUT /<database>`: Creates a new database by calling `db_manager.create_database(db_name)`.
    -   `DELETE /<database>`: Deletes a database by calling `db_manager.delete_database(db_name)`.
    -   `GET /_databases`: A new endpoint to list all available databases.

-   **Task 2.3: Update Existing Endpoints:**
    -   Modify all existing collection and document endpoints (e.g., `GET /<collection>/<id>`, `POST /<collection>`) to work with the new URL scheme and routing logic.

### Phase 3: Persistence and Configuration

**Objective:** Make the multi-database system robust and persistent across server restarts.

-   **Task 3.1: Implement `DatabaseManager` Persistence:**
    -   Create a manifest file (e.g., `tissdb_data/manifest.json`) that stores the names of all existing databases.
    -   On startup, the `DatabaseManager` will read this manifest to repopulate its `databases_` map.
    -   The `create_database` and `delete_database` methods must update the manifest atomically.

-   **Task 3.2: Enhance On-Disk Storage:**
    -   Continue the development of the LSM-Tree persistence (as outlined in the original `tissdb_plan.md`), ensuring all file paths are correctly scoped to their database's subdirectory.

## 5. API Endpoint Changes

The following table summarizes the proposed changes to the REST API.

| Operation                 | Old URL Structure               | New URL Structure                     | Notes                                        |
| ------------------------- | ------------------------------- | ------------------------------------- | -------------------------------------------- |
| **Database Management**   |                                 |                                       |                                              |
| Create Database           | N/A                             | `PUT /<database>`                     | Creates a new, empty database.               |
| Delete Database           | N/A                             | `DELETE /<database>`                  | Deletes an entire database and its data.     |
| List Databases            | N/A                             | `GET /_databases`                     | Returns a list of all database names.        |
| **Collection Management** |                                 |                                       |                                              |
| Create Collection         | `PUT /<collection>`             | `PUT /<database>/<collection>`        |                                              |
| Delete Collection         | `DELETE /<collection>`          | `DELETE /<database>/<collection>`     |                                              |
| List Collections          | `GET /_collections`             | `GET /<database>/_collections`        | Lists collections within a specific database. |
| **Document Operations**   |                                 |                                       |                                              |
| Create Document           | `POST /<collection>`            | `POST /<database>/<collection>`       |                                              |
| Get Document              | `GET /<collection>/<id>`        | `GET /<database>/<collection>/<id>`   |                                              |
| Update Document           | `PUT /<collection>/<id>`        | `PUT /<database>/<collection>/<id>`   |                                              |
| Delete Document           | `DELETE /<collection>/<id>`     | `DELETE /<database>/<collection>/<id>` |                                              |
| **Query & Indexing**      |                                 |                                       |                                              |
| Query Collection          | `POST /<collection>/_query`     | `POST /<database>/<collection>/_query`|                                              |
| Create Index              | `POST /<collection>/_index`     | `POST /<database>/<collection>/_index`|                                              |
