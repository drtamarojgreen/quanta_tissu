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
