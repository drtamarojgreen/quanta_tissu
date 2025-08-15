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

-   [x] **Task 1.5: Single-Field B-Tree Indexing**
    -   [x] Implement or integrate a **persistent** B-Tree library.
    -   [x] Create an API endpoint (`POST /<collection>/_index`) to create an index.
    -   [x] Modify the write path to automatically update the index.
    -   [x] Update the query planner to use the index for lookups.

## Phase 2: V1.1

This phase focuses on evolving the MVP into a more robust database with better data management, full query language support, and transactional capabilities.

-   [x] **Task 2.1: Collection Management**
    -   [x] Implement logic to manage multiple collections within the database.
    -   [x] Create API endpoints for collection management (`PUT /<collection>`, `DELETE /<collection>`).

-   [x] **Task 2.2: LSM Tree Compaction**
    -   [x] Design and implement a compaction strategy (e.g., size-tiered or level-tiered).
    -   [x] Implement a background thread pool for compaction.
    -   [x] Implement logic to merge segments, handle overwrites, and purge tombstones.

-   [x] **Task 2.3: TissQL UPDATE and DELETE Support**
    -   [x] Extend the TissQL grammar and parser for `UPDATE` and `DELETE` statements.
    -   [x] Update the query executor to handle `UPDATE` (read-modify-write) and `DELETE` (tombstone) operations.
    -   [x] Ensure `WHERE` clauses in these statements can leverage indexes.

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

## Phase 4: Production Readiness

This phase focuses on tasks required to make TissDB a production-ready and robust system.

-   [ ] **Robust Error Handling and Logging**
    -   [ ] Implement comprehensive error handling across all components.
    -   [ ] Integrate a structured logging framework.
    -   [ ] Define logging levels and ensure appropriate information is captured.

-   [ ] **Configuration Management**
    -   [ ] Externalize all configurable parameters (e.g., port, data directory, compaction settings).
    -   [ ] Implement a robust configuration loading and validation mechanism.
    -   [ ] Support different deployment environments (development, staging, production).

-   [ ] **Monitoring and Alerting**
    -   [ ] Integrate with industry-standard monitoring systems (e.g., Prometheus, Grafana).
    -   [ ] Expose key metrics (e.g., read/write latency, storage usage, compaction progress, error rates).
    -   [ ] Implement alerting for critical system events and performance thresholds.

-   [ ] **Performance Optimization**
    -   [ ] Conduct detailed profiling to identify performance bottlenecks.
    -   [ ] Optimize critical code paths (e.g., data serialization/deserialization, index lookups).
    -   [ ] Implement caching strategies where appropriate.

-   [ ] **Deployment and Operations**
    -   [ ] Develop Docker images for easy deployment.
    -   [ ] Provide Kubernetes manifests for container orchestration.
    -   [ ] Create comprehensive operational guides for deployment, scaling, and troubleshooting.

-   [ ] **Security Hardening**
    -   [ ] Conduct security audits and penetration testing.
    -   [ ] Implement secure communication (TLS/SSL) for all network interfaces.
    -   [ ] Address potential vulnerabilities (e.g., input validation, buffer overflows).

## Testing and Quality Assurance

This section outlines the testing efforts required to ensure the quality and reliability of TissDB.

-   [x] **Comprehensive Unit Tests**
    -   [x] Achieve high unit test coverage for all core components (storage, query, API).
    -   [x] Ensure tests cover edge cases and error conditions.

-   [x] **Integration Tests**
    -   [x] Develop tests for interactions between different modules (e.g., API calls to LSMTree, WAL recovery).
    -   [x] Verify end-to-end data flow and consistency.

-   [ ] **Performance Tests**
    -   [ ] Implement benchmarks to measure read, write, and query throughput.
    -   [ ] Conduct load tests to assess system behavior under high concurrency.
    -   [ ] Analyze latency distributions and identify performance regressions.

-   [ ] **Stress and Reliability Tests**
    -   [ ] Test system stability under sustained high load.
    -   [ ] Simulate network partitions, node failures, and disk errors to verify fault tolerance.
    -   [ ] Conduct long-running tests to detect memory leaks or resource exhaustion.

-   [ ] **Security Testing**
    -   [ ] Perform vulnerability scanning and penetration testing.
    -   [ ] Verify access control mechanisms and data encryption.


## Important Considerations / Disclaimers

It is crucial to understand the current scope and limitations of TissDB, especially when considering its use for relational data:

-   **Schema Enforcement:** TissDB does not enforce a strict schema. Documents can be inserted into a collection with any fields, potentially leading to inconsistencies if relational integrity (e.g., every "user" document must have an "email" field) is expected. TissDB will not prevent violations of such implicit schemas.

-   **Relational Operations (Joins):** While TissQL has been extended, it does not support complex multi-collection joins that are fundamental to relational databases. Attempting to perform such operations would result in query errors or require manual, inefficient application-level logic.

-   **Data Consistency:** Even with the conceptual implementation of multi-document transactions, ensuring full ACID (Atomicity, Consistency, Isolation, Durability) properties across "relational tables" (collections) in a distributed document database is extremely complex and not fully implemented.

-   **Indexing for Relational Queries:** While compound indexing is conceptually complete, it is designed for optimizing queries on document fields within a single collection, not for optimizing complex relational queries across multiple "tables" or collections.

-   **Bugs and Edge Cases:** As a software system, especially one in a simulated development phase, TissDB will inevitably have bugs and unhandled edge cases. Expecting zero errors is unrealistic.

-   **Conceptual vs. Implemented Features:** Many tasks, particularly in Phase 3 and 4, were marked as complete conceptually (e.g., "Implement a leader election mechanism (e.g., using Raft)"). This signifies that the design approach was outlined, not that the complex, production-ready code has been written, rigorously tested, and hardened for real-world deployment.

## Phase 5: Relational Completeness

This phase outlines the significant tasks required to transform TissDB from a document database into a relationally complete system, capable of handling structured relational data with integrity and efficiency.

-   [ ] **Schema Definition and Enforcement**
    -   [ ] Implement DDL (Data Definition Language) for defining table schemas (columns, data types, constraints).
    -   [ ] Enforce schema validation on data insertion and updates.
    -   [ ] Support primary key and unique constraints.
    -   [ ] Implement foreign key constraints and referential integrity checks.

-   [ ] **Relational Query Language (Full SQL Support)**
    -   [ ] Extend TissQL parser to support a significant subset of SQL (e.g., `JOIN`, `GROUP BY` with `HAVING`, subqueries, `ORDER BY`).
    -   [ ] Develop a robust SQL query optimizer to generate efficient execution plans for relational operations.
    -   [ ] Implement relational algebra operators (e.g., join, projection, selection) in the query executor.

-   [ ] **Transaction Management (Full ACID Compliance)**
    -   [ ] Implement true multi-statement, multi-row ACID transactions with strict isolation levels.
    -   [ ] Develop a robust concurrency control mechanism (e.g., strict two-phase locking or optimistic concurrency control with validation) for relational operations.
    -   [ ] Ensure atomicity and durability across all relational operations, including schema changes.

-   [ ] **Relational Indexing**
    -   [ ] Support B-tree indexes on multiple columns (composite indexes) for efficient join and filtering operations.
    -   [ ] Implement specialized indexes for unique constraints and foreign keys.

-   [ ] **Data Storage for Relational Data**
    -   [ ] Adapt the underlying storage engine (LSM-Tree) to efficiently store and retrieve structured relational data, potentially optimizing for column-oriented storage or hybrid approaches.

-   [ ] **Metadata Management for Relational Schema**
    -   [ ] Implement system catalogs to store and manage metadata about tables, columns, indexes, and constraints.



## Phase 6: Addressing Relational Limitations (Implementation Focus)

This phase outlines the significant tasks required to transform TissDB from a document database into a relationally complete system, capable of handling structured relational data with integrity and efficiency.

-   [ ] **Implement Schema Enforcement:**
    -   [ ] Develop and integrate DDL parser for `CREATE TABLE`, `ALTER TABLE`, `DROP TABLE`.
    -   [ ] Implement schema storage in system catalogs.
    -   [ ] Enforce data type validation during `INSERT` and `UPDATE`.
    -   [ ] Implement primary key and unique constraint validation.
    -   [ ] Implement foreign key constraint validation and cascade actions.

-   [ ] **Implement Full SQL Query Processing:**
    -   [ ] Implement `JOIN` algorithms (e.g., nested loop, hash join, sort-merge join).
    -   [ ] Implement subquery execution.
    -   [ ] Implement `ORDER BY` and `LIMIT` clauses efficiently.
    -   [ ] Develop a cost-based query optimizer for SQL queries.
    -   [x] Integrate SQL query execution with the LSM-Tree storage engine.

-   [ ] **Implement Robust ACID Transactions:**
    -   [ ] Develop a comprehensive transaction manager with strict two-phase locking or MVCC.
    -   [ ] Ensure atomicity and durability for multi-statement, multi-row operations.
    -   [ ] Implement crash recovery for in-flight transactions.

-   [ ] **Relational Indexing:**
    -   [ ] Implement multi-column (composite) B-tree indexes for efficient join and filtering operations.
    -   [ ] Develop specialized index structures for foreign keys.

-   [ ] **Data Storage for Relational Data:**
    -   [ ] Evaluate and implement optimizations for storing structured relational data within the LSM-Tree (e.g., columnar storage for specific tables).

-   [ ] **Comprehensive Error Handling for Relational Operations:**
    -   [ ] Implement specific error codes and messages for relational constraint violations, query parsing errors, and execution failures.

## Phase 7: Operationalization and Full Feature Integration

This phase encompasses all remaining steps to make TissDB completely operational for queries, relational tables, reporting, analytics, and business intelligence.

-   [ ] **End-to-End System Integration & Testing:**
    -   [ ] Integrate all implemented relational features (DDL, SQL parser/optimizer, ACID transactions, relational indexing) with the core LSM-Tree storage.
    -   [ ] Conduct comprehensive end-to-end integration testing across all phases (MVP, V1.1, V2.0, Relational).
    -   [ ] Verify data consistency and integrity under concurrent relational operations.

-   [ ] **Reporting & Analytics Platform Integration:**
    -   [ ] Fully implement and integrate JDBC/ODBC connectors with the relational query engine.
    -   [ ] Develop and integrate the Query Workbench with the full SQL capabilities.
    -   [ ] Develop and integrate the Dashboard Builder with data sources from TissDB (including relational views).
    -   [ ] Develop and integrate the Graph Explorer (if graph capabilities are fully realized).

-   [ ] **Advanced Query Optimization & Performance Tuning:**
    -   [ ] Implement advanced query optimization techniques for complex relational queries (e.g., join reordering, index selection for multi-table queries).
    -   [ ] Conduct extensive performance benchmarking and tuning for all query types (document, aggregate, relational).
    -   [ ] Optimize data access patterns for reporting and analytical workloads.

-   [ ] **Operational Maturity & Hardening:**
    -   [ ] Finalize and validate all aspects of error handling, logging, monitoring, and alerting for the integrated system.
    -   [ ] Complete and validate deployment automation (Docker, Kubernetes) for all components.
    -   [ ] Conduct final security audits and penetration testing on the fully integrated system.
    -   [ ] Develop comprehensive documentation for users, administrators, and developers covering all features.

-   [ ] **User Experience & Tooling Refinement:**
    -   [ ] Refine client libraries (Python, JS/TS) to fully support all new relational and BI features.
    -   [ ] Enhance user-facing tools (Query Workbench, Dashboard Builder) for usability and feature completeness.
