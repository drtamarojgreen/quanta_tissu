# TissDB API Guide

This document provides a comprehensive guide to the TissDB APIs. For a formal definition of all endpoints and schemas, please refer to the **[OpenAPI Specification](api_spec.yml)**.

TissDB exposes two primary sets of HTTP RESTful APIs:

1.  **Core Database API**: This is a low-level API for direct interaction with the database engine. It provides endpoints for managing collections and documents, creating indexes, running queries, and handling transactions. This API is ideal for applications that need to embed TissDB as their primary data store.

2.  **Business Intelligence (BI) Service API**: This is a higher-level, more specialized API designed for the TissDB BI Suite. It allows you to manage BI-specific resources, such as scheduled reports and automated alerts, which are themselves powered by TissQL queries.

---

## Core Database API

The Core API provides direct, low-level access to the TissDB engine.

**Base URL**: `http://<host>:<port>/` (Default port: 8080)

### Collections

A collection is a container for documents, similar to a table in a SQL database.

*   **Create Collection**
    *   `PUT /<collection_name>`
    *   Creates a new, empty collection with the given name.
    *   **Response**: `201 Created`

*   **Delete Collection**
    *   `DELETE /<collection_name>`
    *   Permanently deletes a collection and all documents within it.
    *   **Response**: `204 No Content`

*   **Set Collection Schema**
    *   `PUT /<collection_name>/_schema`
    *   Defines or updates the schema for the collection.
    *   **Request Body**: A JSON object describing the schema fields.
        ```json
        {
          "fields": [
            { "name": "user_id", "type": "String", "required": true },
            { "name": "post_text", "type": "String" },
            { "name": "view_count", "type": "Number", "required": false }
          ]
        }
        ```
    *   **Response**: `200 OK`

### Documents

Documents are flexible, JSON-like records stored within a collection.

*   **Create Document**
    *   `POST /<collection_name>`
    *   Creates a new document in the specified collection. The database will automatically generate a unique ID for the document.
    *   **Request Body**: The JSON document to create.
    *   **Response**: `201 Created` with the new document's ID.

*   **Retrieve Document**
    *   `GET /<collection_name>/<document_id>`
    *   Fetches a single document by its ID.
    *   **Response**: `200 OK` with the JSON document in the body.

*   **Create or Update Document (Upsert)**
    *   `PUT /<collection_name>/<document_id>`
    *   Creates a new document with the specified ID or updates an existing one.
    *   **Request Body**: The full JSON document.
    *   **Response**: `200 OK`

*   **Delete Document**
    *   `DELETE /<collection_name>/<document_id>`
    *   Deletes a single document by its ID.
    *   **Response**: `204 No Content`

### Indexing

Indexes improve query performance on specific fields.

*   **Create Index**
    *   `POST /<collection_name>/_index`
    *   Creates an index on one or more fields.
    *   **Request Body**: A JSON object specifying the field(s) to index.
        *For a single field:*
        ```json
        { "field": "user_id" }
        ```
        *For a composite index:*
        ```json
        { "fields": ["last_name", "first_name"] }
        ```
    *   **Response**: `200 OK`

### Querying

TissDB uses TissQL, a SQL-like language, for querying. Queries are executed via a dedicated endpoint.

*   **Execute TissQL Query**
    *   `POST /<collection_name>/_query`
    *   Runs a TissQL query against the collection. For security, it is highly recommended to use parameterized queries to prevent SQL injection attacks.
    *   **Request Body**: A JSON object containing the query string and an optional array of parameters.
        *To use parameters, insert a `?` placeholder in the query string for each value. The `params` array must then contain the values in the same order.*
        ```json
        {
          "query": "SELECT id, post_text FROM posts WHERE view_count > ? AND author_id = ?",
          "params": [1000, "user123"]
        }
        ```
    *   **Response**: `200 OK` with a JSON array of matching documents.

### Transactions

TissDB supports basic ACID transactions.

*   **Begin Transaction**
    *   `POST /_begin`
    *   Starts a new transaction. The server manages the transaction context for the client connection.
    *   **Response**: `200 OK`

*   **Commit Transaction**
    *   `POST /_commit`
    *   Makes all changes performed within the current transaction permanent.
    *   **Response**: `200 OK`

*   **Rollback Transaction**
    *   `POST /_rollback`
    *   Discards all changes made within the current transaction.
    *   **Response**: `200 OK`

---

## Business Intelligence (BI) Service API

The BI Service API is a high-level API for managing reports and alerts. For detailed information on the request and response schemas, please refer to the **[OpenAPI Specification](api_spec.yml)**.

**Base URL**: `http://tissdb.internal:8080/`

### Reports

Reports are configurations for TissQL queries that run on a schedule and deliver results through various channels.

*   **List all Reports**: `GET /_bi/reports`
*   **Create a new Report**: `POST /_bi/reports`
*   **Get a specific Report**: `GET /_bi/reports/{reportId}`
*   **Update a Report**: `PUT /_bi/reports/{reportId}`
*   **Delete a Report**: `DELETE /_bi/reports/{reportId}`

### Alerts

Alerts are configurations that run a TissQL query periodically, check for a condition, and trigger a notification if the condition is met.

*   **List all Alerts**: `GET /_bi/alerts`
*   **Create a new Alert**: `POST /_bi/alerts`
*   **Get a specific Alert**: `GET /_bi/alerts/{alertId}`
*   **Update an Alert**: `PUT /_bi/alerts/{alertId}`
*   **Delete an Alert**: `DELETE /_bi/alerts/{alertId}`
