# TissDB

TissDB is a lightweight, high-performance NoSQL database built from scratch in C++. It is designed to be a simple, embeddable database for C++ applications.

## Features

*   **Multi-Database Support:** TissDB can manage multiple, isolated databases on a single server instance. Each database has its own collections and data.
*   **Collection Management:** Create, delete, and list collections within each database.
*   **Write-Ahead Log (WAL):** Data is written to a WAL to ensure durability and allow for recovery upon restart.
*   **JSON-like Document Model:** TissDB stores data in a flexible, JSON-like document model.
*   **TissQL Query Language:** TissDB provides a simple, SQL-like query language called TissQL for querying data.
*   **RESTful API:** TissDB provides a RESTful API for interacting with the database.

## Building and Running

To build TissDB, you will need a C++17 compiler and `make`. You can build the project by running the `make` command from the `tissdb` directory:

```bash
# From the tissdb/ directory
make
```

This will create an executable file named `tissdb`. You can then run the database by executing the following command:

```bash
./tissdb
```

By default, the database will listen on port 8080 and store all its data in a directory named `tissdb_data`.

## Current Limitations

*   **In-Memory Storage Model:** While a Write-Ahead Log ensures durability, the primary data structures (collections) are still held in-memory. The database must replay the WAL on startup to restore state.
*   **No Index Persistence:** B-Tree indexes are not yet persistent.
*   **In-Progress Transaction Support:** The API includes endpoints for transactions, but the implementation is not yet complete.
*   **No Replication or Sharding:** The database does not yet support replication or sharding.

## Documentation

For comprehensive documentation, including API guides, architecture details, and development information, please refer to the [TissDB Documentation Index](../docs/tissdb_index.md).