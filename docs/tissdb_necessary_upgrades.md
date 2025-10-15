# TissDB Necessary Upgrades

This document lists necessary upgrades and bug fixes for the TissDB database server.

## 1. `map::at` crash on `SELECT COUNT(*)` on empty collection

**Issue:** The server crashes with a `std::map::at` exception when executing a `SELECT COUNT(*)` query on an empty collection.

**Details:** This was discovered in the `test_multiple_documents` C++ test. The test inserts 0 documents (because the `INSERT` queries fail), and then executes `SELECT COUNT(*)`. The server then crashes.

**Log:**
`[ERROR] [tissdb/api/http_server.cpp:615] Sending response: 500 Internal Server Error: map::at`

**Recommendation:** The database server code should be fixed to handle `COUNT(*)` on an empty collection gracefully, probably by returning 0.

## 2. Incorrect HTTP status code for missing database

**Issue:** The server returns a `500 Internal Server Error` when a `DELETE` request is made for a non-existent database. The correct HTTP status code should be `404 Not Found`.

**Details:** This was discovered during the setup of the C++ tests. The test runner tries to delete the database before creating it, and the 500 error was causing the test setup to fail.

**Log:**
`[ERROR] [tissdb/api/http_server.cpp:615] Sending response: 500 Internal Server Error: Database 'test_cpp_db' not found.`

**Recommendation:** The server should be modified to return a `404 Not Found` status code when a `DELETE` request is made for a database that does not exist.

## 3. `JOIN` query fails with 500 Internal Server Error

**Issue:** A `JOIN` query between two collections results in a `500 Internal Server Error`.

**Details:** This was discovered in the `test_advanced_queries` C++ test.

**Query:**
`SELECT c.name, o.item, o.amount FROM orders o JOIN customers c ON o.customer_id = c._id;`

**Recommendation:** The `JOIN` implementation in the database server needs to be investigated and fixed.

## 4. `GROUP BY` query returns incorrect results

**Issue:** A `GROUP BY` query with an aggregate function (`SUM`) does not return the aggregated value.

**Details:** This was discovered in the `test_advanced_queries` C++ test. The query returns the grouped-by column, but not the sum.

**Query:**
`SELECT customer_id, SUM(amount) FROM orders GROUP BY customer_id;`

**Response:**
`[{"_id":"cust1","customer_id":"cust1"},{"_id":"cust2","customer_id":"cust2"}]`

**Recommendation:** The `GROUP BY` implementation in the database server needs to be fixed to correctly compute and return aggregate function results.
