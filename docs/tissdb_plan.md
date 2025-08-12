# Tissdb: A Development Plan

## 1. Introduction & Vision

Tissdb is a new NoSQL database designed for storing and querying data in a flexible, XML-like format. The vision for Tissdb is to provide a highly intuitive and human-readable data storage solution, ideal for applications where data structure is fluid and evolves over time. It will prioritize ease of use, developer experience, and a simple, powerful query language.

## 2. Data Model

Tissdb's data model is based on an XML-like structure, but with a simpler, more modern syntax.

*   **Database**: The top-level container, analogous to a relational database.
*   **Collection**: A group of related documents. Collections do not enforce a schema.
*   **Document**: A single record in a collection, identified by a unique ID. A document is the basic unit of storage and is composed of elements.
*   **Element**: A key-value pair, where the key is a tag and the value can be a primitive (string, number, boolean) or another element (for nesting).

Example Document:
```xml
<document id="user_123">
  <name>John Doe</name>
  <email>john.doe@example.com</email>
  <attributes>
    <age>30</age>
    <is_active>true</is_active>
  </attributes>
</document>
```

## 3. Storage Engine

The storage engine will be designed for efficiency and simplicity.

*   **File Structure**: Each collection will be stored in its own file. This isolates data and simplifies file management.
*   **Format**: Documents will be appended to the collection file in a serialized binary format to reduce parsing overhead.
*   **Indexing**: Tissdb will support indexing on element values. Indexes will be stored in separate B-tree structured files for fast lookups. Initially, we will support indexing on primitive types.

## 4. API & Query Language (TissQL)

Interaction with Tissdb will be primarily through a RESTful API. A custom query language, TissQL, will be used for data manipulation.

*   **REST API**:
    *   `POST /<collection>`: Create a new document.
    *   `GET /<collection>/<id>`: Retrieve a document.
    *   `PUT /<collection>/<id>`: Update a document.
    *   `DELETE /<collection>/<id>`: Delete a document.
    *   `POST /<collection>/_query`: Execute a TissQL query.

*   **TissQL (Tissdb Query Language)**:
    *   A simple, declarative language for querying documents.
    *   Example: `SELECT * FROM users WHERE attributes.age > 25`
    *   It will support basic filtering, projections, and sorting.

## 5. MVP Features

The Minimum Viable Product will focus on core database functionality.

*   Full CRUD (Create, Read, Update, Delete) operations.
*   Basic indexing on single fields.
*   TissQL for querying.
*   In-memory caching for frequently accessed documents.
*   Basic error handling and logging.

## 6. High-Level Architecture

Tissdb will have a layered architecture:

1.  **API Layer**: Exposes the RESTful endpoints. Handles request validation and parsing.
2.  **Query Engine**: Parses and executes TissQL queries. Optimizes query plans.
3.  **Transaction Manager**: Manages atomicity of operations (initially at the document level).
4.  **Storage Engine**: Responsible for reading from and writing to disk. Manages storage files and indexes.
5.  **Cache Manager**: Implements the in-memory caching layer.

## 7. Development Roadmap

*   **Phase 1 (MVP)**:
    *   Implement the core data model and storage engine.
    *   Develop the REST API and basic TissQL.
    *   Implement single-field indexing.
    *   Timeline: 3 months

*   **Phase 2 (V1.1)**:
    *   Introduce compound indexes.
    *   Enhance TissQL with aggregate functions.
    *   Add support for atomic multi-document transactions.
    *   Improve performance and benchmarking.
    *   Timeline: +3 months

*   **Phase 3 (V2.0)**:
    *   Implement replication and sharding for scalability.
    *   Add user authentication and access control.
    *   Develop client libraries for popular languages (Python, JavaScript).
    *   Timeline: +6 months
