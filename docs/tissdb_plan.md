# Tissdb: A Comprehensive Development Plan

## 1. Introduction & Vision

Tissdb is a new NoSQL database designed for storing and querying data in a flexible, XML-like format. The vision for Tissdb is to provide a highly intuitive and human-readable data storage solution, ideal for applications where data structure is fluid and evolves over time. It prioritizes developer experience with a simple, powerful query language and a straightforward API. The core design philosophy is to keep the database simple, maintainable, and easy to reason about.

## 2. Data Model

Tissdb's data model is based on an XML-like structure, but with a simpler, more modern syntax and a richer set of data types.

*   **Database**: The top-level container for collections.
*   **Collection**: A group of related documents. Collections do not enforce a schema by default, but optional schema validation can be enabled on a per-collection basis.
*   **Document**: A single record in a collection, identified by a unique ID (UUID by default). A document is composed of elements.
*   **Element**: A key-value pair. The key is a tag, and the value can be a primitive, a nested element, or a list of primitives/elements.
*   **Primitive Types**: `string`, `number` (integer and float), `boolean`, `datetime` (ISO 8601), and `binary` (Base64 encoded).

**More Complex Example Document:**
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

*   **Serialization**: Documents will be serialized using **MessagePack** for a compact and fast binary representation.
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

Security is a first-class citizen in Tissdb's design.

*   **Encryption at Rest**: All data files on disk (segments, indexes, WAL) will be encrypted using AES-256. Master keys will be managed by an external system (e.g., HashiCorp Vault) to avoid storing keys with the data.
*   **Encryption in Transit**: The REST API will require TLS 1.2 or higher to ensure all communication is encrypted. Communication between nodes in a future clustered environment will also be secured with TLS.
*   **Access Control**: Role-Based Access Control (RBAC) will be implemented as a core feature. Administrators will be able to define roles with granular permissions (e.g., `read`, `write`, `delete`) on a per-collection basis.
*   **Injection Prevention**: The TissQL query engine will exclusively use parameterized queries internally. All user input will be treated as data, not executable code, to eliminate the risk of injection attacks.

## 7. Detailed Development Roadmap

*   **Phase 1 (MVP) - 3 Months**:
    *   Task 1.1: Implement document serialization/deserialization with MessagePack.
    *   Task 1.2: Build the core append-only storage layer for a single collection.
    *   Task 1.3: Implement the basic REST API for document CRUD.
    *   Task 1.4: Develop the TissQL parser for basic `SELECT` queries.
    *   Task 1.5: Implement single-field B-tree indexing.

*   **Phase 2 (V1.1) - 3 Months**:
    *   Task 2.1: Implement the LSM tree with background compaction.
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
