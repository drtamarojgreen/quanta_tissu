# TissDB Development Task List

This document outlines the development tasks for TissDB, derived from the official development plan. The tasks are organized into phases, from Minimum Viable Product (MVP) to a full-featured, distributed database.

## Phase 1: MVP (V1.0)

The goal of this phase is to deliver a functional, single-node TissDB instance capable of basic document CRUD operations, simple TissQL queries, and single-field indexing.

-   [x] **Task 1.2: Core Append-Only Storage Layer**
    -   **Sub-tasks**:
        -   [x] Design the on-disk format for segment files, including document entries and a file header.
        -   [x] Implement a Write-Ahead Log (WAL) for durability. Writes will first go to the WAL, then to an in-memory table (`memtable`).
        -   [x] Implement the `memtable` flush mechanism, which writes the sorted contents of the `memtable` to a new, immutable segment file on disk.
    -   **Acceptance Criteria**:
        -   [x] The server can be shut down unexpectedly during a write operation and recover its state from the WAL upon restart.
        -   [x] Data written to the database is correctly persisted in segment files.

-   [x] **Task 1.3: Basic REST API for Document CRUD**
    -   **Sub-tasks**:
        -   [x] Implement a lightweight C++ HTTP server.
        -   [x] Implement the `POST /<collection>` endpoint to create new documents.
        -   [x] Implement the `GET /<collection>/<id>` endpoint to retrieve a document by its ID (requiring a full scan initially).
        -   [x] Implement `PUT /<collection>/<id>` (full update) and `DELETE /<collection>/<id>` (tombstone write).
    -   **Acceptance Criteria**:
        -   [x] All CRUD endpoints can be successfully tested using a tool like `curl` or Postman.
        -   [x] Appropriate HTTP status codes (200, 201, 404, etc.) are returned.

-   [x] **Task 1.4: TissQL Parser for Basic SELECT**
    -   **Sub-tasks**:
        -   [x] Define a formal grammar (e.g., in EBNF) for a subset of TissQL: `SELECT <fields> FROM <collection> WHERE <field> = <value>`.
        -   [x] Implement a parser from the grammar.
        -   [x] The parser must produce a simple Abstract Syntax Tree (AST) representing the query.
        -   [x] Implement a basic query executor that walks the AST and performs a full collection scan, filtering documents based on the `WHERE` clause.
    -   **Acceptance Criteria**:
        -   [x] The `POST /<collection>/_query` endpoint correctly parses and executes valid `SELECT` queries.
        -   [x] Malformed queries result in a `400 Bad Request` error with a descriptive message.

-   [x] **Task 1.5: Single-Field B-Tree Indexing**
    -   **Sub-tasks**:
        -   [x] Implement or integrate a persistent B-Tree library.
        -   [x] Create an API endpoint `POST /<collection>/_index` to trigger the creation of an index on a specific field.
        -   [x] Modify the write path to automatically update the relevant index whenever a document is created, updated, or deleted.
        -   [x] Update the query planner to detect when a `WHERE` clause can use an index, and direct the executor to use the index instead of a full scan.
    -   **Acceptance Criteria**:
        -   [x] Creating an index on a field is successful.
        -   [x] A `SELECT` query using an indexed field in the `WHERE` clause shows a significant performance improvement over a query on a non-indexed field.

## Phase 2: V1.1

This phase focuses on evolving the MVP into a more robust database with better data management, full query language support, and transactional capabilities.

-   [x] **Task 2.1: Collection Management**
    -   **Sub-tasks**:
        -   [x] Implement logic to manage multiple collections within the database.
        -   [x] Create API endpoints for collection management (`PUT /<collection>`, `DELETE /<collection>`).
    -   **Acceptance Criteria**:
        -   [x] Multiple collections can be created and deleted.
        -   [x] CRUD operations on one collection do not affect others.

-   [x] **Task 2.2: LSM Tree Compaction**
    -   **Sub-tasks**:
        -   [x] Design and implement a compaction strategy (e.g., size-tiered or level-tiered).
        -   [x] Implement a dedicated background thread pool to manage compaction tasks without blocking foreground operations.
        -   [x] Develop the logic to merge multiple sorted segment files into a new segment, correctly handling overwritten values and removing entries marked with a tombstone.
        -   [x] Update the database manifest atomically upon successful completion of a compaction to reflect the new segment state.
    -   **Acceptance Criteria**:
        -   [x] Deleted documents are physically purged from disk after a compaction cycle completes.
        -   [x] The total number of segment files is managed effectively, preventing performance degradation on reads.
        -   [x] The database remains fully available for reads and writes during the compaction process.

-   [x] **Task 2.3: TissQL UPDATE and DELETE Support**
    -   **Sub-tasks**:
        -   [x] Extend the TissQL grammar and parser to support `UPDATE ... SET ... WHERE ...` and `DELETE FROM ... WHERE ...` statements.
        -   [x] Update the query planner and executor to handle these new statement types.
        -   [x] The `DELETE` executor will find matching documents and write new entries with a "tombstone" marker.
        -   [x] The `UPDATE` executor will effectively perform a read-modify-write operation, creating a new version of the document and writing it to the current memtable.
    -   **Acceptance Criteria**:
        -   [x] `UPDATE` queries correctly modify the specified fields in all matching documents.
        -   [x] `DELETE` queries make matching documents inaccessible for all subsequent reads.
        -   [x] The `WHERE` clause in both `UPDATE` and `DELETE` statements correctly utilizes existing indexes to find target documents.

-   [ ] **Task 2.4: Compound Indexing**
    -   **Sub-tasks**:
        -   [ ] Update the `POST /<collection>/_index` endpoint to accept an array of field names.
        -   [ ] Modify the B-Tree implementation to handle composite keys, where key values are concatenated from multiple document fields.
        -   [ ] Enhance the query planner to detect when a query's `WHERE` clause can be partially or fully satisfied by a compound index.
    -   **Acceptance Criteria**:
        -   [ ] An index can be successfully created on `(brand, price)`.
        -   [ ] A query like `SELECT * FROM products WHERE brand = 'AudioPhonic' AND price < 200` uses the compound index to narrow the search space efficiently.

-   [ ] **Task 2.5: TissQL Aggregate Functions**
    -   **Sub-tasks**:
        -   [ ] Extend the TissQL grammar to support aggregate functions (`COUNT`, `AVG`, `SUM`, `MIN`, `MAX`) and the `GROUP BY` clause.
        -   [ ] Implement an aggregation operator in the query executor that can process rows and compute aggregate values.
        -   [ ] Implement the `GROUP BY` logic, likely using a hash table to group results before aggregation.
    -   **Acceptance Criteria**:
        -   [ ] `SELECT COUNT(*) FROM products WHERE is_available = true` returns the correct count.
        -   [ ] `SELECT brand, AVG(price) FROM products GROUP BY brand` returns the correct average price for each brand.

-   [ ] **Task 2.6: Multi-Document Transactions**
    -   **Sub-tasks**:
        -   [ ] Implement API endpoints for transaction control: `POST /_transaction/begin`, `POST /_transaction/commit`, `POST /_transaction/rollback`.
        -   [ ] Implement a transaction manager that assigns a transaction ID and tracks the read and write sets for each active transaction.
        -   [ ] Implement a concurrency control mechanism (e.g., two-phase locking) to ensure serializable isolation.
        -   [ ] Modify the WAL to log transaction boundaries (`BEGIN`, `COMMIT`) to enable atomic commits and recovery.
    -   **Acceptance Criteria**:
        -   [ ] A transaction that updates two separate documents can be committed atomically; either both updates are visible, or neither is.
        -   [ ] If a transaction is rolled back, none of its changes are visible.
        -   [ ] The server can recover from a crash mid-transaction, correctly rolling back any uncommitted changes upon restart.

## Phase 3: V2.0

This phase transforms TissDB from a single-node database into a distributed, scalable, and secure system.

-   [ ] **Task 3.1: Leader-Follower Replication**
    -   **Sub-tasks**:
        -   [ ] Implement a leader election mechanism using a proven consensus algorithm like Raft.
        -   [ ] Extend the Write-Ahead Log (WAL) to become a replicated log that the leader streams to followers.
        -   [ ] Implement the follower logic to receive, acknowledge, and apply log entries to their local storage engine.
        -   [ ] Develop a robust failover mechanism. If a leader becomes unresponsive, the remaining nodes must elect a new leader with a fully up-to-date log.
    -   **Acceptance Criteria**:
        -   [ ] Writes to the leader are successfully replicated to a configurable quorum of followers before the write is acknowledged to the client.
        -   [ ] In the event of a leader failure, a new leader is elected within seconds and the database remains available for writes.
        -   [ ] Follower nodes can be taken offline and brought back online, automatically catching up with the leader's state.

-   [ ] **Task 3.2: Range-Based Sharding**
    -   **Sub-tasks**:
        -   [ ] Design a sharding strategy based on ranges of a designated shard key within each collection.
        -   [ ] Implement a cluster metadata service (or use the Raft cluster itself) to manage the mapping of key ranges to shard-hosting nodes.
        -   [ ] Update the query router in the API layer to inspect incoming requests, determine the target shard(s), and proxy the request to the correct node(s).
        -   [ ] Implement an administrative API for shard splitting and rebalancing to allow for cluster expansion.
    -   **Acceptance Criteria**:
        -   [ ] A collection's data is successfully distributed across multiple nodes.
        -   [ ] Queries are correctly routed to the appropriate nodes, and cross-shard queries are aggregated correctly.
        -   [ ] The cluster can be expanded with new nodes, and existing shards can be rebalanced onto them without downtime.

-   [ ] **Task 3.3: Role-Based Access Control (RBAC)**
    -   **Sub-tasks**:
        -   [ ] Design and implement internal system collections to store user, role, and permission data.
        -   [ ] Create secure administrative API endpoints for managing users and roles (e.g., `POST /_admin/users`, `POST /_admin/roles`).
        -   [ ] Integrate an authentication middleware into the API layer that validates bearer tokens (API Keys).
        -   [ ] Implement an authorization middleware that checks the authenticated user's permissions for the target collection and operation before passing the request to the query engine.
    -   **Acceptance Criteria**:
        -   [ ] An administrator can create a role with specific permissions (e.g., read-only on `collection_a`, write on `collection_b`).
        -   [ ] A user with an API key linked to that role is denied access when attempting an unauthorized operation.

-   [ ] **Task 3.4: Official Client Libraries**
    -   **Sub-tasks**:
        -   [ ] Develop an idiomatic Python client library that provides a clean, high-level interface over the REST API.
        -   [ ] The library must handle connection pooling, request serialization, and response deserialization.
        -   [ ] Package the library and publish it to the Python Package Index (PyPI).
        -   [ ] Repeat the process for a JavaScript/TypeScript client library, publishing it to the npm registry.
    -   **Acceptance Criteria**:
        -   [ ] A developer can `pip install tissdb` or `npm install tissdb` and interact with the database without writing raw HTTP requests.
        -   [ ] Both libraries are well-documented and include comprehensive examples.

-   [ ] **Task 3.5: Encryption at Rest**
    -   **Sub-tasks**:
        -   [ ] Implement a robust cryptographic library.
        -   [ ] Modify the storage engine's file manager to encrypt segment files, index files, and WAL files before writing them to disk.
        -   [ ] Implement a secure key management architecture for managing the master encryption key.
        -   [ ] Ensure the encryption/decryption process has minimal performance overhead on database operations.
    -   **Acceptance Criteria**:
        -   [ ] All data files on disk are fully encrypted.
        -   [ ] Manual inspection of disk files (e.g., with a hex editor) reveals no plaintext data.
        -   [ ] The database cannot be started without access to the master key from the configured KMS.

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
    -   [ ] Adapt the underlying storage engine (LSM-Tree) to efficiently store and retrieve structured relational data, a aynchronously optimizing for column-oriented storage or hybrid approaches.

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
