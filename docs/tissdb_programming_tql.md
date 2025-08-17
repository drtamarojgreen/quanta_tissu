# TissDB Query Language (TQL) Programming Manual

This document provides a guide to using the TissDB Query Language (TQL), a language designed for interacting with the TissDB. TQL is based on SQL and provides functionalities for data retrieval and manipulation.

## 1. SELECT Statement

The `SELECT` statement is used to query the database and retrieve data that matches criteria that you specify.

### Syntax

```sql
SELECT select_list FROM table_name [WHERE where_clause] [GROUP BY groupby_clause];
```

- `select_list`: Can be `*` to select all columns, a list of columns, or a list of aggregate functions.
- `table_name`: The name of the table to query.
- `where_clause` (Optional): Filters records based on a condition.
- `groupby_clause` (Optional): Groups rows that have the same values in specified columns into summary rows.

### Examples

**Select all columns from a table:**

```sql
SELECT * FROM users;
```

**Select specific columns:**

```sql
SELECT name, email FROM users;
```

**Using a WHERE clause:**

```sql
SELECT name FROM users WHERE age > 30;
```

#### Advanced WHERE Clause Operators

TQL supports a variety of operators to create more complex filtering conditions.

-   **`AND`, `OR`, `NOT`**: Combine multiple conditions.
    ```sql
    -- Select users who are older than 30 and live in the USA
    SELECT name FROM users WHERE age > 30 AND country = 'USA';

    -- Select users who are either older than 30 or live in the USA
    SELECT name FROM users WHERE age > 30 OR country = 'USA';

    -- Select users who do not live in the USA
    SELECT name FROM users WHERE NOT country = 'USA';
    ```

-   **`LIKE`**: Perform pattern matching on string data. The `%` character is a wildcard for zero or more characters.
    ```sql
    -- Select users whose name starts with 'J'
    SELECT name FROM users WHERE name LIKE 'J%';
    ```

-   **`IN`**: Check if a value matches any value in a list.
    ```sql
    -- Select users from the USA or Canada
    SELECT name, country FROM users WHERE country IN ('USA', 'Canada');
    ```

-   **`BETWEEN`**: Check if a value is within a certain range (inclusive).
    ```sql
    -- Select users whose age is between 25 and 35
    SELECT name, age FROM users WHERE age BETWEEN 25 AND 35;
    ```

### Ordering Results with `ORDER BY`

The `ORDER BY` clause is used to sort the result set in ascending or descending order.

-   `ASC`: Ascending order (default).
-   `DESC`: Descending order.

```sql
-- Select all users, ordered by age in descending order
SELECT name, age FROM users ORDER BY age DESC;

-- Select all users, ordered by country (ascending) and then by name (descending)
SELECT name, country FROM users ORDER BY country ASC, name DESC;
```

### Limiting Results with `LIMIT`

The `LIMIT` clause is used to restrict the number of rows returned by a query.

```sql
-- Select the 5 youngest users
SELECT name, age FROM users ORDER BY age ASC LIMIT 5;
```

**Using aggregate functions:**

```sql
SELECT COUNT(id), AVG(age) FROM users;
```

**Using GROUP BY:**

```sql
SELECT department, COUNT(id) FROM employees GROUP BY department;
```

## 2. INSERT Statement

The `INSERT` statement is used to add new records to a table.

### Syntax

```sql
INSERT INTO table_name [(column_list)] VALUES (value_list);
```

- `table_name`: The name of the table.
- `column_list` (Optional): The columns to insert data into.
- `value_list`: The values to be added.

### Examples

**Insert a new record:**

```sql
INSERT INTO users (name, email, age) VALUES ('John Doe', 'john.doe@example.com', 28);
```

**Insert a new record without specifying columns (requires values for all columns in order):**

```sql
INSERT INTO users VALUES (1, 'Jane Smith', 'jane.smith@example.com', 32);
```

## 3. UPDATE Statement

The `UPDATE` statement is used to modify existing records in a table.

### Syntax

```sql
UPDATE table_name SET set_clause [WHERE where_clause];
```

- `table_name`: The name of the table.
- `set_clause`: One or more `column = value` pairs.
- `where_clause` (Optional): Specifies which records should be updated. If omitted, all records will be updated.

### Examples

**Update a single record:**

```sql
UPDATE users SET email = 'john.d@new-example.com' WHERE name = 'John Doe';
```

**Update multiple records:**

```sql
UPDATE users SET age = age + 1 WHERE country = 'USA';
```

## 4. DELETE Statement

The `DELETE` statement is used to remove existing records from a table.

### Syntax

```sql
DELETE FROM table_name [WHERE where_clause];
```

- `table_name`: The name of the table.
- `where_clause` (Optional): Specifies which records should be deleted. If omitted, all records will be deleted.

### Examples

**Delete a single record:**

```sql
DELETE FROM users WHERE name = 'John Doe';
```

**Delete all records from a table:**

```sql
DELETE FROM users;
```

## 5. Supported Data Types

TissDB supports a variety of data types for storing data in documents.

| Data Type     | Description                                                                 |
|---------------|-----------------------------------------------------------------------------|
| `STRING`      | A UTF-8 encoded string.                                                     |
| `NUMBER`      | A 64-bit floating-point number, capable of storing integers and decimals.   |
| `BOOLEAN`     | A `true` or `false` value.                                                  |
| `DATETIME`    | A point in time, stored with high precision.                                |
| `BINARY DATA` | A sequence of bytes, suitable for storing raw data like images or files.    |
| `ELEMENTS`    | A nested list of elements, allowing for the creation of complex, structured documents. |

## 6. Limitations and Future Features

TissDB is under active development, and some SQL features are not yet implemented.

### Currently Unsupported Features

-   **`JOIN` Operations**: The current version of TissDB does not support `JOIN` operations for combining data from multiple tables in a single query. This is planned for a future release.
-   **DDL Statements**: Data Definition Language (DDL) statements like `CREATE TABLE`, `ALTER TABLE`, and `DROP TABLE` are not yet supported. Schema management must be handled outside of the query language for now.

These features are on the development roadmap and will be added in future versions of TissDB.
