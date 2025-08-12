# TissDB File Structure

## 1. Introduction

TissDB is a database concept that is currently under development. As such, its "file structure" is not a single, self-contained directory but rather a collection of files across the repository that represent its different facets: conceptual design, a practical Python implementation of a related knowledge store, and C++-based utilities.

This document provides an overview of the key files and directories related to TissDB.

## 2. Conceptual Design (`docs/`)

The core vision, planning, and design documents for TissDB are located in the `docs/` directory. These files are the best source of information for understanding the long-term goals and architecture of TissDB.

-   `docs/tissdb_vision.md`: Describes the high-level vision for TissDB as a NoSQL database based on the principles of "TissLang," a paradigm that models data as living tissue.
-   `docs/tissdb_plan.md`: Provides a more concrete development plan for TissDB, detailing a phased implementation approach, a RESTful API, a query language (TissQL), and a C++-based architecture.
-   `docs/tissdb_integration.md`: Explains how TissDB is intended to integrate with the broader Quanta ecosystem.
-   `docs/tissdb_api_schema.json`: A JSON schema that defines the proposed structure for the TissDB REST API.

## 3. Python Implementation (`quanta_tissu/quanta_tissu/`)

While the formal `tissdb` plan calls for a C++ implementation, a related and functional piece of the knowledge storage infrastructure exists in Python.

-   `quanta_tissu/quanta_tissu/knowledge_base.py`: This file contains the `KnowledgeBase` class, an in-memory vector-based knowledge store. It uses embeddings to store and retrieve documents based on semantic similarity. This serves as a practical implementation of some of the core ideas behind TissDB's knowledge management capabilities.

## 4. C++ Graph Visualization Utility (`quanta_tissu/knowledge_app/`)

This directory contains a small, standalone C++ application for visualizing graphs in the console. It is not the TissDB database itself, but rather a utility that could be used to display the graph-based data structures mentioned in the TissDB vision.

-   `quanta_tissu/knowledge_app/graph_logic.h` & `graph_logic.cpp`: Define and implement the `GraphLogic` class, which handles the rendering of nodes and edges to a text-based canvas.
-   `quanta_tissu/knowledge_app/main.cpp`: The entry point for the C++ application, which runs the graph visualization.

## 5. Tests (`tests/`)

The tests provide insight into how the implemented components are expected to function.

-   `tests/test_knowledge_base.py`: Contains unit tests for the `KnowledgeBase` class. These tests demonstrate how to add documents to the knowledge base and retrieve them using queries. They are a valuable resource for understanding the practical usage of the Python `KnowledgeBase`.

## 6. Hypothetical C++ Implementation Structure

While the current functional data store is in Python, the `docs/tissdb_plan.md` outlines a comprehensive C++ implementation. Based on that plan, a future C++ implementation would likely have a modular structure like the one below:

```
tissdb/
├── api/
│   ├── http_server.h
│   └── http_server.cpp      // Handles REST API requests and responses
├── query/
│   ├── parser.h
│   ├── parser.cpp           // Parses TissQL queries into an AST
│   ├── planner.h
│   ├── planner.cpp          // Creates an execution plan from the AST
│   └── executor.h
│   └── executor.cpp         // Executes the plan against the storage engine
├── storage/
│   ├── lsm_tree.h
│   ├── lsm_tree.cpp         // Core LSM-tree implementation
│   ├── memtable.h
│   ├── memtable.cpp         // In-memory write buffer
│   ├── wal.h
│   ├── wal.cpp              // Write-Ahead Log for durability
│   ├── indexer.h
│   └── indexer.cpp          // B-Tree indexing logic
├── common/
│   ├── document.h           // Definition of the document data structure
│   └── types.h              // Common data types and constants
├── include/
│   └── tissdb.h             // Public API header for a potential libtissdb
└── tests/
    ├── test_api.cpp
    ├── test_query.cpp
    └── test_storage.cpp
```
