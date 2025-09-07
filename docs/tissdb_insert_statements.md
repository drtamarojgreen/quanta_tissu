# TissDB INSERT Statement Implementation Requirements

This document outlines the current and future implementation requirements for the TissQL `INSERT` statement.

## 1. Current Implementation

The current implementation supports inserting a single row into a collection.

### Syntax

```sql
INSERT INTO collection_name (column1, column2, ...) VALUES (value1, value2, ...);
```

### Requirements

- The `INSERT INTO` keywords are mandatory.
- A valid `collection_name` must be provided.
- A parenthesized list of `columns` is required.
- The `VALUES` keyword is mandatory.
- A parenthesized list of `values` is required.
- The number of columns must match the number of values.
- Values can be string literals, numeric literals, boolean literals, or `NULL`.
- The statement must be terminated with a semicolon.

## 2. Future Enhancements (To be tested)

The following are potential enhancements to the `INSERT` statement. The test suite should be designed to accommodate these features in the future.

### 2.1. Multi-row Inserts

Support for inserting multiple rows in a single statement.

#### Syntax

```sql
INSERT INTO collection_name (column1, column2, ...) VALUES
(value1a, value1b, ...),
(value2a, value2b, ...),
...;
```

### 2.2. INSERT INTO ... SELECT

Support for inserting the results of a `SELECT` query into a collection.

#### Syntax

```sql
INSERT INTO collection_name (column1, column2, ...)
SELECT columnA, columnB, ...
FROM another_collection
WHERE ...;
```

### 2.3. Error Handling

The system should provide clear error messages for various invalid `INSERT` statements, including:
- Syntax errors (e.g., missing keywords, parentheses).
- Mismatched column and value counts.
- Inserting into a non-existent collection.
- Data type mismatches (if schema enforcement is added).
- Constraint violations (if constraints are added).
