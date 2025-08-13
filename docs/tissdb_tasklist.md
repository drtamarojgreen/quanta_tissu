# TissDB Development Task List

This document outlines the development tasks for TissDB, derived from the official development plan. The tasks are organized into phases, from Minimum Viable Product (MVP) to a full-featured, distributed database.

## Phase 1: MVP (V1.0)

The goal of this phase is to deliver a functional, single-node TissDB instance capable of basic document CRUD operations, simple TissQL queries, and single-field indexing.

-   [x] **Task 1.2: Core Append-Only Storage Layer**
    -   [x] Design the on-disk format for segment files.
    -   [x] Implement a Write-Ahead Log (WAL) for durability.
    -   [x] Implement the `memtable` and its flush-to-disk mechanism.
    -   [x] Ensure the server can recover from the WAL after a crash.

-   [x] **Task 1.3: Basic REST API for Document CRUD**
    -   [x] Implement a lightweight C++ HTTP server.
    -   [x] Implement `POST /<collection>` to create documents.
    -   [x] Implement `GET /<collection>/<id>` to retrieve documents.
    -   [x] Implement `PUT /<collection>/<id>` for full document updates.
    -   [x] Implement `DELETE /<collection>/<id>` for document deletion (tombstones).

-   [x] **Task 1.4: TissQL Parser for Basic SELECT**
    -   [x] Define a formal grammar (EBNF) for basic `SELECT` queries.
    -   [x] Implement a parser to generate an Abstract Syntax Tree (AST).
    -   [x] Implement a basic query executor that can perform full collection scans based on the AST.
    -   [x] Implement the `POST /<collection>/_query` endpoint.

-   [~] **Task 1.5: Single-Field B-Tree Indexing**
    -   [~] Implement or integrate a **persistent** B-Tree library.
    -   [x] Create an API endpoint (`POST /<collection>/_index`) to create an index.
    -   [x] Modify the write path to automatically update the index.
    -   [x] Update the query planner to use the index for lookups.

## Phase 2: V1.1

This phase focuses on evolving the MVP into a more robust database with better data management, full query language support, and transactional capabilities.

-   [x] **Task 2.1: Collection Management**
    -   [x] Implement logic to manage multiple collections within the database.
    -   [x] Create API endpoints for collection management (`PUT /<collection>`, `DELETE /<collection>`).

-   [ ] **Task 2.2: LSM Tree Compaction**
    -   [ ] Design and implement a compaction strategy (e.g., size-tiered or level-tiered).
    -   [ ] Implement a background thread pool for compaction.
    -   [ ] Implement logic to merge segments, handle overwrites, and purge tombstones.

-   [ ] **Task 2.3: TissQL UPDATE and DELETE Support**
    -   [ ] Extend the TissQL grammar and parser for `UPDATE` and `DELETE` statements.
    -   [ ] Update the query executor to handle `UPDATE` (read-modify-write) and `DELETE` (tombstone) operations.
    -   [ ] Ensure `WHERE` clauses in these statements can leverage indexes.

-   [ ] **Task 2.4: Compound Indexing**
    -   [ ] Update the index creation endpoint to accept multiple field names.
    -   [ ] Modify the B-Tree implementation to handle composite keys.
    -   [ ] Enhance the query planner to use compound indexes.

-   [ ] **Task 2.5: TissQL Aggregate Functions**
    -   [ ] Extend the TissQL grammar for `COUNT`, `AVG`, `SUM`, `MIN`, `MAX`, and `GROUP BY`.
    -   [ ] Implement aggregation and grouping operators in the query executor.

-   [ ] **Task 2.6: Multi-Document Transactions**
    -   [ ] Implement transaction control endpoints (`/begin`, `/commit`, `/rollback`).
    -   [ ] Implement a transaction manager to track read/write sets.
    -   [ ] Implement a concurrency control mechanism (e.g., two-phase locking).
    -   [ ] Make the WAL transaction-aware.

## Phase 3: V2.0

This phase transforms TissDB from a single-node database into a distributed, scalable, and secure system.

-   [ ] **Task 3.1: Leader-Follower Replication**
    -   [ ] Implement a leader election mechanism (e.g., using Raft).
    -   [ ] Implement a replicated WAL that the leader streams to followers.
    -   [ ] Implement a robust failover mechanism.

-   [ ] **Task 3.2: Range-Based Sharding**
    -   [ ] Design a sharding strategy based on key ranges.
    -   [ ] Implement a cluster metadata service to manage shard locations.
    -   [ ] Update the query router to proxy requests to the correct nodes.
    -   [ ] Implement an API for shard splitting and rebalancing.

-   [ ] **Task 3.3: Role-Based Access Control (RBAC)**
    -   [ ] Design and implement system collections for users, roles, and permissions.
    -   [ ] Create administrative APIs for managing security.
    -   [ ] Integrate authentication and authorization middleware into the API layer.

-   [ ] **Task 3.4: Official Client Libraries**
    -   [ ] Develop and publish an idiomatic Python client library to PyPI.
    -   [ ] Develop and publish an idiomatic JavaScript/TypeScript client library to npm.

-   [ ] **Task 3.5: Encryption at Rest**
    -   [ ] Integrate a cryptographic library.
    -   [ ] Encrypt all data files on disk (segments, indexes, WAL).
    -   [ ] Implement a secure key management architecture.

## Future Features (Beyond V2.0)

This section lists features mentioned in vision and integration documents that are not yet part of the formal development plan.

-   [ ] **Change Data Capture (CDC) Streams**
-   [ ] **Webhooks**
-   [ ] **Materialized Views**
-   [ ] **JDBC/ODBC Connector for BI Tool Integration**
-   [ ] **TissDB Business Intelligence Suite**
    -   [ ] Query Workbench
    -   [ ] Dashboard Builder
    -   [ ] Graph Explorer
