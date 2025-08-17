# TissDB

TissDB is a lightweight, high-performance NoSQL database built from scratch in C++. It is designed to be a simple, embeddable database for C++ applications.

## Features

*   **LSM-Tree Storage Engine:** TissDB uses a log-structured merge-tree (LSM-Tree) for high write throughput.
*   **JSON-like Document Model:** TissDB stores data in a flexible, JSON-like document model.
*   **TissQL Query Language:** TissDB provides a simple, SQL-like query language called TissQL for querying data.
*   **Collection Management:** Organize documents into named collections, similar to tables in a relational database.
*   **B-Tree Indexing:** TissDB supports B-Tree indexing for fast lookups.
*   **RESTful API:** TissDB provides a RESTful API for interacting with the database.

## Building and Running

To build TissDB, you will need a C++17 compiler and `make`. You can then build the project by running the `make` command in the `quanta_tissu/tissdb` directory:

```bash
make
```

This will create an executable file named `tissdb`. You can then run the database by executing the following command:

```bash
./tissdb
```

By default, the database will listen on port 8080 and store its data in a directory named `tissdb_data`.

## Current Limitations

*   **In-Memory B-Tree:** The B-Tree implementation is currently in-memory and is not yet persistent.
*   **No Transactions:** The database does not yet support transactions.
*   **No Replication or Sharding:** The database does not yet support replication or sharding.

## Documentation

For comprehensive documentation, including API guides, architecture details, and development information, please refer to the [TissDB Documentation Index](../docs/tissdb_index.md).