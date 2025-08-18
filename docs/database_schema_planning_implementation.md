# Architecture: TissDB Multi-Database Support

## 1. Introduction

This document describes the multi-database architecture of TissDB. The system has been designed to support true multi-tenancy, allowing TissDB to manage multiple, isolated databases from a single server instance. Each database has its own set of collections, data, and persistence files.

This enhancement makes TissDB a more scalable and versatile database solution, capable of serving diverse applications and users simultaneously.

## 2. System Architecture

The multi-database architecture is centered around a new top-level `DatabaseManager` class that orchestrates all database instances.

### 2.1. The `DatabaseManager`
The `TissDB::Storage::DatabaseManager` is the central component for managing all database instances.

-   **Responsibility:** It handles the full lifecycle of databases: creation, deletion, loading, and retrieval.
-   **Persistence:** It maintains a `manifest.json` file in the root data directory (e.g., `tissdb_data/`). This file lists all active databases, allowing the manager to restore its state upon server restart.
-   **Instantiation:** The `main.cpp` file instantiates a single `DatabaseManager` and passes it to the `HttpServer`, making it available to the entire API layer.

### 2.2. The `LSMTree` as a Database Instance
Each individual database is represented by an instance of the `TissDB::Storage::LSMTree` class.

-   **Isolation:** Each `LSMTree` instance is initialized with a unique path (e.g., `tissdb_data/my_db/`) and all its files, including its Write-Ahead Log, are stored within this directory.
-   **Data Durability:** Data persistence within each database is ensured by a Write-Ahead Log (`wal.log`). All data-modifying operations (`put`, `del`) and collection management operations (`create_collection`, `delete_collection`) are first recorded in the WAL before being applied to the in-memory data structures.
-   **Recovery:** On startup, the `LSMTree` reads its WAL to recover its state, rebuilding the in-memory collections and documents.

### 2.3. Graceful Shutdown
To prevent data loss, the server implements a graceful shutdown mechanism.

-   **Signal Handling:** The `main` function in `main.cpp` registers handlers for `SIGINT` and `SIGTERM`.
-   **Shutdown Sequence:** Upon receiving a shutdown signal, the application calls `shutdown()` on the `DatabaseManager`, which in turn calls `shutdown()` on each `LSMTree` instance. This ensures that the Write-Ahead Log for every database is properly flushed and closed before the server exits.

## 3. API Endpoints

The REST API is database-aware. All endpoints are prefixed with a database name, allowing clients to target specific database instances.

| Operation                 | URL Structure                         | Notes                                           |
| ------------------------- | ------------------------------------- | ----------------------------------------------- |
| **Database Management**   |                                       |                                                 |
| Create Database           | `PUT /<database>`                     | Creates a new, empty database.                  |
| Delete Database           | `DELETE /<database>`                  | Deletes an entire database and its data.        |
| **Collection Management** |                                       |                                                 |
| Create Collection         | `PUT /<database>/<collection>`        | Creates a new collection within a database.     |
| Delete Collection         | `DELETE /<database>/<collection>`     | Deletes a collection from a database.           |
| List Collections          | `GET /<database>/_collections`        | Lists collections within a specific database.   |
| **Document Operations**   |                                       |                                                 |
| Create Document           | `POST /<database>/<collection>`       | Creates a document in a collection.             |
| Get Document              | `GET /<database>/<collection>/<id>`   | Retrieves a document by its ID.                 |
| Update Document           | `PUT /<database>/<collection>/<id>`   | Updates (replaces) a document.                  |
| Delete Document           | `DELETE /<database>/<collection>/<id>` | Deletes a document.                             |
| **Query & Indexing**      |                                       |                                                 |
| Query Collection          | `POST /<database>/<collection>/_query`| Executes a TissQL query against a collection.   |
| Create Index              | `POST /<database>/<collection>/_index`| (Placeholder) Creates an index on a field.      |
