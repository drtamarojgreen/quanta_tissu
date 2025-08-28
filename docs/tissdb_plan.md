# Tissdb: A Comprehensive Development Plan

## 1. Introduction & Vision

Tissdb is a new database designed to serve as a high-performance, flexible data backbone for the Quanta ecosystem. The vision for Tissdb is to provide a powerful data integration layer that supports multiple data formats and enables complex, graph-based analysis.

It utilizes a hybrid storage model supporting both **compressed XML** for complex, hierarchical documents and **CSV-style formats** for efficient, tabular knowledge storage. The query engine is built on algorithms that model the data as a **3D graph network**, allowing for powerful queries that can traverse relationships between disparate data sources.

Development will follow a two-layer security strategy:
1.  **Unsecured Knowledge Layer (Phase 1):** The initial version will be an open, unsecured layer designed for rapid development and seamless knowledge sharing between trusted services.
2.  **Secure Transactional Layer (Phase 3):** A later version will add a robust, secure layer on top, with features like RBAC, encryption, and ACID transactions for sensitive data and critical workloads.

## 2. Data Model

Tissdb's data model is a graph where both documents and records are treated as "nodes".

### 2.1 XML-style Documents
Ideal for complex, hierarchical data with nested structures.

*   **Document**: A single record in a collection, identified by a unique ID. A document is composed of elements.
*   **Element**: A key-value pair. The key is a tag, and the value can be a primitive, a nested element, or a list of primitives/elements.
*   **Primitive Types**: `string`, `number` (integer and float), `boolean`, `datetime` (ISO 8601), and `binary` (Base64 encoded).

**Example XML Document:**
```xml
<document id="product_789">
  <name>Wireless Headphones</name>
  <brand>AudioPhonic</brand>
  <price>199.99</price>
  <is_available>true</is_available>
  <release_date>2023-10-26T10:00:00Z</release_date>
  <specs>
    <weight>250g</weight>
    <bluetooth_version>5.2</bluetooth_version>
    <features>
      <feature>Noise Cancellation</feature>
      <feature>24-hour Battery</feature>
      <feature>Water Resistant</feature>
    </features>
  </specs>
  <reviews>
    <review id="rev_1">
      <rating>5</rating>
      <comment>Amazing sound quality!</comment>
    </review>
    <review id="rev_2">
      <rating>4</rating>
      <comment>Very comfortable.</comment>
    </review>
  </reviews>
</document>
```

## 3. Storage Engine

The storage engine will use a log-structured merge-tree (LSM) approach for high write throughput.

*   **File Structure**: Each collection will be stored as a set of segmented, append-only files. A write-ahead log (WAL) will ensure durability and atomic writes.
*   **Compaction**: A background process will periodically run compaction on the segment files to merge data, remove deleted entries, and improve read performance.
*   **Indexing**: Indexes will be implemented using B-trees. When a document is written, both the data and corresponding index entries are updated atomically.

## 4. API & Query Language (TissQL)

Interaction with Tissdb will be through a RESTful API and the TissQL query language.

*   **Extended REST API**:
    *   `POST /<collection>`: Create a new document.
    *   `GET /<collection>/<id>`: Retrieve a document.
    *   `PUT /<collection>/<id>`: Update a document.
    *   `DELETE /<collection>/<id>`: Delete a document.
    *   `POST /<collection>/_query`: Execute a TissQL query.
    *   `PUT /<collection>`: Create a new collection.
    *   `DELETE /<collection>`: Delete a collection.
    *   `POST /<collection>/_index`: Create an index on a field.

*   **TissQL (Tissdb Query Language) Examples**:
    *   **Select**: `SELECT name, price FROM products WHERE brand = 'AudioPhonic' AND price < 200`
    *   **Projection**: `SELECT name, specs.bluetooth_version FROM products`
    *   **Update**: `UPDATE products SET price = 179.99 WHERE id = 'product_789'`
    *   **Delete**: `DELETE FROM products WHERE is_available = false`

### API Payload Examples

**Create a new document (`POST /products`)**
```json
// Request Body
{
  "name": "Smart Watch",
  "brand": "TechGear",
  "price": 299.99,
  "is_available": true
}

// Response Body (201 Created)
{
  "id": "a1b2c3d4-e5f6-4a3b-8c2d-1e9f8a7b6c5d",
  "name": "Smart Watch",
  "brand": "TechGear",
  "price": 299.99,
  "is_available": true
}
```

**Execute a query (`POST /products/_query`)**
```json
// Request Body
{
  "query": "SELECT name, price FROM products WHERE brand = 'TechGear'"
}

// Response Body (200 OK)
[
  {
    "name": "Smart Watch",
    "price": 299.99
  }
]
```

## 5. High-Level Architecture

Tissdb's layered architecture is designed for modularity and separation of concerns.

1.  **API Layer**: Built on a high-performance HTTP framework. Handles request/response cycles, authentication, and rate limiting.
2.  **Query Engine**:
    *   **Parser**: Converts TissQL strings into an Abstract Syntax Tree (AST).
    *   **Planner**: Creates an optimal execution plan from the AST.
    *   **Executor**: Executes the plan against the storage engine.
3.  **Transaction Manager**: Ensures ACID properties at the document level. Manages read/write locks.
4.  **Storage Engine**:
    *   **LSM Tree Manager**: Handles writing to memory tables and flushing to disk segments.
    *   **Compactor**: Runs in the background to merge segments.
    *   **Index Manager**: Manages B-tree index files.
5.  **Cache Manager**: Implements an LRU (Least Recently Used) cache for hot documents and index blocks.

## 6. Security Considerations

TissDB will be developed with a two-layer security model to balance the need for rapid, frictionless development with the requirement for robust security for sensitive workloads.

### Layer 1: Unsecured Knowledge Layer (Phases 1 & 2)
The initial focus is on creating an open, unsecured data layer. This is designed for trusted internal environments where services and agents in the Quanta ecosystem need to share knowledge and data with minimal friction.
*   **No Authentication/Authorization**: The API will not enforce any authentication or authorization. All clients will have full access.
*   **No Encryption**: Data will not be encrypted at rest or in transit by default. This simplifies development, debugging, and performance for the knowledge-sharing use case.
*   **Focus**: The priority for this layer is functionality, performance, and ease of integration.

### Layer 2: Secure Transactional Layer (Phase 3)
This layer will be built on top of the knowledge layer and will introduce robust, configurable security features for sensitive data and critical applications.
*   **Encryption at Rest**: All data files on disk (segments, indexes, WAL) will be optionally encrypted using AES-256. Master keys will be managed by an external system (e.g., HashiCorp Vault).
*   **Encryption in Transit**: The REST API will support TLS 1.2 or higher to ensure all communication can be encrypted.
*   **Access Control**: Role-Based Access Control (RBAC) will be implemented. Administrators will be able to define roles with granular permissions (e.g., `read`, `write`, `delete`) on a per-collection basis.
*   **Injection Prevention**: The TissQL query engine will exclusively use parameterized queries internally. All user input will be treated as data, not executable code, to eliminate the risk of injection attacks.
This is a core design principle and applies to both layers.

## 7. Detailed Development Roadmap

*   **Phase 1 (MVP) - 3 Months**:
    *   Task 1.2: Build the core append-only storage layer for a single collection.
    *   Task 1.3: Implement the basic REST API for document CRUD.
    *   Task 1.4: Develop the TissQL parser for basic `SELECT` queries.
    *   Task 1.5: Implement single-field B-tree indexing.

*   **Phase 2 (V1.1) - 3 Months**:
    *   Task 2.1: Implement collection management.
    *   Task 2.2: Implement the LSM tree with background compaction.
    *   Task 2.2: Add support for `UPDATE` and `DELETE` in TissQL.
    *   Task 2.3: Introduce compound indexes.
    *   Task 2.4: Enhance TissQL with aggregate functions (`COUNT`, `AVG`).
    *   Task 2.5: Implement multi-document transactions.

*   **Phase 3 (V2.0) - 6 Months**:
    *   Task 3.1: Design and implement a leader-follower replication strategy.
    *   Task 3.2: Implement range-based sharding for horizontal scaling.
    *   Task 3.3: Implement RBAC as defined in the Security section.
    *   Task 3.4: Develop official client libraries for Python and JavaScript.
    *   Task 3.5: Implement encryption at rest.

## 9. Phase 2: V1.1 - Detailed Breakdown

This section provides a more granular breakdown of the tasks required to deliver Version 1.1.

### Objective
Evolve the MVP into a more robust database by implementing background data management (compaction), full query language support for writes, and foundational transactional and analytical capabilities.

---

### Task 2.1: LSM Tree Compaction
*   **Sub-tasks**:
    1.  Design and implement a compaction strategy (e.g., size-tiered or level-tiered).
    2.  Implement a dedicated background thread pool to manage compaction tasks without blocking foreground operations.
    3.  Develop the logic to merge multiple sorted segment files into a new segment, correctly handling overwritten values and removing entries marked with a tombstone.
    4.  Update the database manifest atomically upon successful completion of a compaction to reflect the new segment state.
*   **Acceptance Criteria**:
    *   Deleted documents are physically purged from disk after a compaction cycle completes.
    *   The total number of segment files is managed effectively, preventing performance degradation on reads.
    *   The database remains fully available for reads and writes during the compaction process.

### Task 2.2: TissQL UPDATE and DELETE Support
*   **Sub-tasks**:
    1.  Extend the TissQL grammar and parser to support `UPDATE ... SET ... WHERE ...` and `DELETE FROM ... WHERE ...` statements.
    2.  Update the query planner and executor to handle these new statement types.
    3.  The `DELETE` executor will find matching documents and write new entries with a "tombstone" marker.
    4.  The `UPDATE` executor will effectively perform a read-modify-write operation, creating a new version of the document and writing it to the current memtable.
*   **Acceptance Criteria**:
    *   `UPDATE` queries correctly modify the specified fields in all matching documents.
    *   `DELETE` queries make matching documents inaccessible for all subsequent reads.
    *   The `WHERE` clause in both `UPDATE` and `DELETE` statements correctly utilizes existing indexes to find target documents.

### Task 2.3: Compound Indexing
*   **Sub-tasks**:
    1.  Update the `POST /<collection>/_index` endpoint to accept an array of field names.
    2.  Modify the B-Tree implementation to handle composite keys, where key values are concatenated from multiple document fields.
    3.  Enhance the query planner to detect when a query's `WHERE` clause can be partially or fully satisfied by a compound index.
*   **Acceptance Criteria**:
    *   An index can be successfully created on `(brand, price)`.
    *   A query like `SELECT * FROM products WHERE brand = 'AudioPhonic' AND price < 200` uses the compound index to narrow the search space efficiently.

### Task 2.4: TissQL Aggregate Functions
*   **Sub-tasks**:
    1.  Extend the TissQL grammar to support aggregate functions (`COUNT`, `AVG`, `SUM`, `MIN`, `MAX`) and the `GROUP BY` clause.
    2.  Implement an aggregation operator in the query executor that can process rows and compute aggregate values.
    3.  Implement the `GROUP BY` logic, likely using a hash table to group results before aggregation.
*   **Acceptance Criteria**:
    *   `SELECT COUNT(*) FROM products WHERE is_available = true` returns the correct count.
    *   `SELECT brand, AVG(price) FROM products GROUP BY brand` returns the correct average price for each brand.

### Task 2.5: Multi-Document Transactions
*   **Sub-tasks**:
    1.  Implement API endpoints for transaction control: `POST /_transaction/begin`, `POST /_transaction/commit`, `POST /_transaction/rollback`.
    2.  Implement a transaction manager that assigns a transaction ID and tracks the read and write sets for each active transaction.
    3.  Implement a concurrency control mechanism (e.g., two-phase locking) to ensure serializable isolation.
    4.  Modify the WAL to log transaction boundaries (`BEGIN`, `COMMIT`) to enable atomic commits and recovery.
*   **Acceptance Criteria**:
    *   A transaction that updates two separate documents can be committed atomically; either both updates are visible, or neither is.
    *   If a transaction is rolled back, none of its changes are visible.
    *   The server can recover from a crash mid-transaction, correctly rolling back any uncommitted changes upon restart.

## 10. Phase 3: V2.0 - Detailed Breakdown

This section provides a more granular breakdown of the tasks required to deliver Version 2.0.

### Objective
Transform TissDB from a single-node database into a distributed, scalable, and secure system. This phase also includes developing client libraries to improve developer experience.

---

### Task 3.1: Leader-Follower Replication
*   **Sub-tasks**:
    1.  Implement a leader election mechanism using a proven consensus algorithm like Raft.
    2.  Extend the Write-Ahead Log (WAL) to become a replicated log that the leader streams to followers.
    3.  Implement the follower logic to receive, acknowledge, and apply log entries to their local storage engine.
    4.  Develop a robust failover mechanism. If a leader becomes unresponsive, the remaining nodes must elect a new leader with a fully up-to-date log.
*   **Acceptance Criteria**:
    *   Writes to the leader are successfully replicated to a configurable quorum of followers before the write is acknowledged to the client.
    *   In the event of a leader failure, a new leader is elected within seconds and the database remains available for writes.
    *   Follower nodes can be taken offline and brought back online, automatically catching up with the leader's state.

### Task 3.2: Range-Based Sharding
*   **Sub-tasks**:
    1.  Design a sharding strategy based on ranges of a designated shard key within each collection.
    2.  Implement a cluster metadata service (or use the Raft cluster itself) to manage the mapping of key ranges to shard-hosting nodes.
    3.  Update the query router in the API layer to inspect incoming requests, determine the target shard(s), and proxy the request to the correct node(s).
    4.  Implement an administrative API for shard splitting and rebalancing to allow for cluster expansion.
*   **Acceptance Criteria**:
    *   A collection's data is successfully distributed across multiple nodes.
    *   Queries are correctly routed to the appropriate nodes, and cross-shard queries are aggregated correctly.
    *   The cluster can be expanded with new nodes, and existing shards can be rebalanced onto them without downtime.

### Task 3.3: Role-Based Access Control (RBAC)
*   **Sub-tasks**:
    1.  Design and implement internal system collections to store user, role, and permission data.
    2.  Create secure administrative API endpoints for managing users and roles (e.g., `POST /_admin/users`, `POST /_admin/roles`).
    3.  Integrate an authentication middleware into the API layer that validates bearer tokens (API Keys).
    4.  Implement an authorization middleware that checks the authenticated user's permissions for the target collection and operation before passing the request to the query engine.
*   **Acceptance Criteria**:
    *   An administrator can create a role with specific permissions (e.g., read-only on `collection_a`, write on `collection_b`).
    *   A user with an API key linked to that role is denied access when attempting an unauthorized operation.

### Task 3.4: Official Client Libraries
*   **Sub-tasks**:
    1.  Develop an idiomatic Python client library that provides a clean, high-level interface over the REST API.
    2.  The library must handle connection pooling, request serialization, and response deserialization.
    3.  Package the library and publish it to the Python Package Index (PyPI).
    4.  Repeat the process for a JavaScript/TypeScript client library, publishing it to the npm registry.
*   **Acceptance Criteria**:
    *   A developer can `pip install tissdb` or `npm install tissdb` and interact with the database without writing raw HTTP requests.
    *   Both libraries are well-documented and include comprehensive examples.

### Task 3.5: Encryption at Rest
*   **Sub-tasks**:
    1.  Implement a robust cryptographic library.
    2.  Modify the storage engine's file manager to encrypt segment files, index files, and WAL files before writing them to disk.
    3.  Implement a secure key management architecture for managing the master encryption key.
    4.  Ensure the encryption/decryption process has minimal performance overhead on database operations.
*   **Acceptance Criteria**:
    *   All data files on disk are fully encrypted.
    *   Manual inspection of disk files (e.g., with a hex editor) reveals no plaintext data.
    *   The database cannot be started without access to the master key from the configured KMS.
