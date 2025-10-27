# TissDB Query Implementation Details

This document provides an overview of the internal implementation of the TissDB query engine, including key data structures, assumptions, and execution strategies.

## Core Design Principles

-   **Document-Oriented:** TissDB is a document-oriented database. Data is stored in flexible, JSON-like documents.
-   **`_id` as Primary Key:** Every document in a collection is required to have a unique `_id` field, which serves as its primary key. This is fundamental to how the database identifies and manages documents.

## Query Execution Flow

The execution of a TissQL query follows these steps:

1.  **Parsing:** The raw SQL query string is parsed into an Abstract Syntax Tree (AST) by the `Parser` (`query/parser.cpp`). The AST represents the query in a structured way.
2.  **Execution:** The `execute_select_statement` (or `update`, `delete`, `insert`) function in the corresponding executor file (`query/executor_*.cpp`) traverses the AST and performs the requested operations.
3.  **Storage Interaction:** The executor interacts with the storage engine (`storage/lsm_tree.cpp` and `storage/collection.cpp`) to retrieve, store, or delete data.

## Key Data Structures (AST)

The Abstract Syntax Tree is defined in `query/ast.h`. Key structures include:

-   `SelectStatement`: Represents a `SELECT` query, including fields, collection, `WHERE` clause, `JOIN` clause, etc.
-   `JoinClause`: Represents a `JOIN` operation, including the join type, the joined collection, and the `ON` condition.
-   `Expression`: A variant type that can represent any part of an expression (e.g., an identifier, a literal value, a binary operation).

## Implementation Strategies and Assumptions

### Table Aliases

-   **Parsing:** The parser (`parser.cpp`) is designed to recognize table aliases in `FROM` and `JOIN` clauses, with or without the `AS` keyword.
-   **Storage:** The aliases are stored in the `SelectStatement` and `JoinClause` structs in the AST.
-   **Execution:** The query executor uses these aliases to create qualified field names (e.g., `alias.field_name`) when processing `JOIN`s.

### JOIN Implementation

-   **Combined Documents:** When performing a `JOIN`, the executor creates a temporary, in-memory `Document` for each pair of documents being joined.
-   **Qualified Names:** The fields in this temporary document are "qualified" by prefixing them with the table alias (e.g., `orders.item`, `customers.name`). This is achieved by string concatenation in the `combine_documents` function.
-   **`_id` Handling:** The `_id` of each document is explicitly added to the combined document with its qualified name (e.g., `c._id`) to make it available for the `ON` clause evaluation.
-   **Evaluation:** The `ON` clause expression is then evaluated against this temporary combined document.

### Deletions and Tombstones

-   **Tombstones:** When a document is deleted, it is not immediately removed from the in-memory data structure (`Memtable`). Instead, it is marked with a "tombstone" by setting its corresponding document pointer to `nullptr`.
-   **`scan` Behavior:** The `scan` operation, which is used to retrieve all documents from a collection, has been designed to **ignore** tombstones. It will only return documents that have not been deleted. This is crucial for the correctness of aggregate functions like `COUNT(*)`.
