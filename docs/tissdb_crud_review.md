# TissDB CRUD and Startup Issue Review

## 1. Summary of the Issue

A critical issue was identified that causes the TissDB server to crash during its startup sequence. The crash occurs when the server is recovering a database from its Write-Ahead Log (WAL) and the log contains duplicate entries for the creation of a specific collection.

While the initial request was to investigate general "CRUD issues," this startup failure represents a fundamental problem with the **Create** aspect of the database's lifecycle and its data integrity guarantees.

## 2. Root Cause Analysis

The TissDB server uses a `DatabaseManager` to manage multiple databases. On startup, the manager reads a `manifest.json` file and, for each database listed, creates an `LSMTree` object. The `LSMTree` constructor immediately calls a `recover()` method to restore its state by replaying its `wal.log` file.

The `LSMTree::recover()` function iterates through all entries in the WAL. If it finds a `LogEntryType::CREATE_COLLECTION` entry, it unconditionally calls the `LSMTree::create_collection()` method.

The bug occurs when the `wal.log` contains more than one `CREATE_COLLECTION` entry for the same collection name. The first entry succeeds, creating the collection in memory. When the recovery process encounters the second, duplicate entry, it calls `create_collection()` again. This function correctly identifies that the collection already exists in its in-memory map and throws a `std::runtime_error`. This exception is not caught within the recovery loop, causing it to propagate up and terminate the server startup process.

The core of the problem is that the **recovery process is not idempotent**. It cannot gracefully handle duplicate log entries, which may occur in various scenarios.

## 3. Location of the Bug

The logical flaw is located in the `LSMTree::recover()` function in the following file:

- **File:** `tissdb/storage/lsm_tree.cpp`

Specifically, it's the `case` statement for `LogEntryType::CREATE_COLLECTION` within the `for` loop that replays log entries.

## 4. Recommended Solution

To fix this issue, the `LSMTree::recover()` function should be modified to be idempotent. Before attempting to create a collection from a log entry, it should first check if the collection already exists in memory. If it does, it should log a warning and skip the creation rather than calling a function that will throw an exception.

### Proposed Code Change

**File:** `tissdb/storage/lsm_tree.cpp`

**Current Code in `LSMTree::recover()`:**
```cpp
        switch (entry.type) {
            // ... other cases
            case LogEntryType::CREATE_COLLECTION:
                create_collection(entry.collection_name, {}); // Assuming default schema
                break;
            // ... other cases
        }
```

**Recommended Change:**
```cpp
        switch (entry.type) {
            // ... other cases
            case LogEntryType::CREATE_COLLECTION:
                if (!collections_.count(entry.collection_name)) {
                    create_collection(entry.collection_name, {}); // Assuming default schema
                } else {
                    LOG_WARNING("Recovery: Attempted to re-create collection '" + entry.collection_name + "' which already exists. Skipping.");
                }
                break;
            // ... other cases
        }
```

This change will make the server startup process more robust and prevent crashes caused by this specific WAL inconsistency.
