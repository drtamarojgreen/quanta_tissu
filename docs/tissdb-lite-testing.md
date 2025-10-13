# TissDB-Lite Testing Guide

This document provides a comprehensive guide to the testing suite for `tissdb-lite`, covering both its internal unit tests and the integration tests for database migration.

## Purpose of the Tests

The tests for `tissdb-lite` serve several critical purposes:

*   **Functionality Verification**: Ensure that all methods of the `TissDBLite` class behave as expected, handling various inputs and edge cases correctly.
*   **Regression Prevention**: Guard against unintended side effects or breakage of existing features when new functionalities are added or changes are made.
*   **Migration Reliability**: Verify the seamless and accurate transfer of data between `tissdb`, `tissdb-lite` (in-memory), and Wix Data collections.
*   **"From Scratch" Validation**: Demonstrate the robustness of the implementation without relying on external testing frameworks or libraries.

## Unit Tests (Internal Functionality)

The unit tests focus on the core functionalities of the `TissDBLite` class, particularly its in-memory database operations and query parsing capabilities.

### Tested Functionalities

*   **Collection Management**: Creation and initial state of collections.
*   **CRUD Operations**: `insert`, `bulkInsert`, `find`, `update`, and `remove` for in-memory collections.
*   **Query Engine**: Comprehensive testing of the TissLang-like query parser (`_tokenize`, `_toRpn`, `_createFilterFunction`) with various operators (`=`, `!=`, `>`, `>=`, `<`, `<=`), logical connectors (`AND`, `OR`), and parentheses for grouping.
*   **Data Export**: Verification of the `exportCollection` method.
*   **Error Handling**: Assertions for expected errors when interacting with non-existent collections or items, or when providing malformed queries.

### Assertion Mechanism

All assertions are made using custom helper functions (`assert` and `assertThrows`) implemented directly within the test file, adhering to the "no external libraries" constraint.

*   `assert(description, condition)`: Checks if a given `condition` is true. Logs success or failure with a descriptive message.
*   `assertThrows(description, func, expectedError)`: Executes a function `func` and asserts that it throws an error containing `expectedError` in its message.

### How to Run Unit Tests

The unit tests are part of the main test script. To run them, execute the following command from the project root:

```bash
node lite/test.js
```

The output will show a series of `✅ SUCCESS` or `❌ FAILURE` messages for each assertion, followed by a summary.

## Migration Tests (Integration with TissDB)

These tests verify the data migration processes between `tissdb-lite` and an actual running `tissdb` server.

### Interaction with Actual TissDB Server

Unlike the unit tests, migration tests interact with a live `tissdb` instance. This requires the `tissdb` server to be running and accessible.

### Configuration

The `tissdb` server connection details and authentication token are configured via environment variables or directly within the `lite/test.js` file.

*   `TISSDB_HOST`: The hostname or IP address of the `tissdb` server (default: `localhost`).
*   `TISSDB_PORT`: The port on which the `tissdb` server is listening (default: `9876`).
*   `TISSDB_AUTH_TOKEN`: (Required) The authorization token needed to authenticate with the `tissdb` server. This is crucial for successful communication.

**Example of running with environment variables:**

```bash
TISSDB_HOST=your_tissdb_host TISSDB_PORT=your_tissdb_port TISSDB_AUTH_TOKEN=your_auth_token node lite/test.js
```

### Test Setup and Cleanup

Each migration test run includes automated setup and cleanup steps on the `tissdb` server to ensure a clean testing environment:

1.  **Cleanup**: Deletes any existing test collections (`tissdb_migration_test_1`, `tissdb_migration_test_2`) from previous runs.
2.  **Collection Creation**: Creates new, empty test collections on `tissdb`.
3.  **Population**: Populates `tissdb_migration_test_1` with sample data.

### Migration Scenarios Tested

*   **TissDB to TissDB-Lite**: Data is fetched from `tissdb` and bulk-inserted into an in-memory `tissdb-lite` collection.
*   **TissDB-Lite to TissDB**: Data is exported from an in-memory `tissdb-lite` collection and inserted into a `tissdb` collection.

### Current Known Issue: Authentication

As of the last test run, all interactions with the `tissdb` server result in a `401 Authorization header missing.` error. This indicates that the `tissdb` server requires an `Authorization` header, which is currently not being provided by the `tissdbHttpClient`.

**Resolution**: To resolve this, ensure that the `TISSDB_AUTH_TOKEN` environment variable is set with a valid token, and the `tissdbHttpClient` is updated to include an `Authorization: Bearer <token>` header in its requests.

### How to Run Migration Tests

The migration tests are integrated into the main test script. Ensure your `tissdb` server is running and configured correctly, then execute:

```bash
node lite/test.js
```

Monitor the console output for detailed logs of HTTP requests and responses, which are crucial for debugging any further issues.
