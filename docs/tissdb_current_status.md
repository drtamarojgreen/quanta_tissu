# TissDB: Current Status and Development Roadmap

## 1. Introduction

This document provides a clear overview of the current status of the TissDB project as of Q3 2025. It is intended to be a single source of truth for developers and stakeholders, clarifying the project's vision, the concrete implementation plan, and the actual work that has been completed so far.

There is a significant distinction between the long-term conceptual vision for TissDB and the practical, phased implementation currently underway. This document aims to reconcile these two aspects.

## 2. The Long-Term Vision: A Self-Organizing Data Tissue

The foundational vision for TissDB is highly ambitious, modeling a database on the biological principles of living tissue. This concept, known as **TissLang**, envisions a system where data exists as "cellular atoms" that form "adhesion bonds" with each other, creating a self-healing, adaptive, and resilient data fabric.

Key aspects of this vision include:
- **Graph-based model:** Data cells as nodes and bonds as edges.
- **Decentralized architecture:** Peer-to-peer and fault-tolerant.
- **Active data structures:** Cells can contain their own logic.

This vision serves as the philosophical guide for the project's long-term development.

## 3. The Concrete Plan: A Phased C++ NoSQL Database

To realize this vision, a concrete development plan has been created, which outlines the creation of a high-performance C++ NoSQL database. This plan is the official roadmap for the TissDB implementation.

The key features of this planned database are:
- **Storage Engine**: A Log-Structured Merge-Tree (LSM-Tree) for high write throughput.
- **Data Model**: A flexible JSON-like document model.
- **Query Language**: TissQL, a SQL-like query language for interacting with data via a RESTful API.
- **Indexing**: B-Tree indexing for fast lookups.
- **Phased Rollout**: Development is broken into three main phases, starting with a Minimum Viable Product (MVP) and progressively adding features like replication, sharding, and advanced security.

## 4. Current Implementation Status (As of August 2025)

**The C++ NoSQL database described in the development plan is currently in the design and planning stage and has not yet been implemented.**

The "TissDB project" at present consists of the following separate components:

1.  **Conceptual Documentation (`docs/`)**: A rich set of documents outlining the vision (`tissdb_vision.md`) and the detailed implementation plan (`tissdb_plan.md`).

2.  **A Functional Python Knowledge Store (`quanta_tissu/quanta_tissu/knowledge_base.py`)**:
    - This is the primary piece of *functional* code related to the TissDB concept.
    - It is an **in-memory vector store**, not the planned C++ database.
    - It uses embeddings to store and retrieve documents based on semantic similarity, serving as a practical tool for knowledge management within the Quanta ecosystem.

3.  **A C++ Graph Visualization Utility (`quanta_tissu/knowledge_app/`)**:
    - This is a small, standalone console application for rendering graph structures.
    - It is a utility related to the *vision* of graph-based data but is not part of the database itself.

In summary, TissDB as a database does not yet exist. The project is currently a collection of design documents and related, but distinct, software utilities.

## 5. Immediate Next Steps: The Phase 1 MVP

The active development work will focus on implementing **Phase 1 (MVP)** as detailed in the `tissdb_plan.md`. This constitutes the immediate roadmap for the project. The key tasks are:

- **Task 1.2: Core Append-Only Storage Layer**:
  - Implement a Write-Ahead Log (WAL) and an in-memory `memtable`.
  - Implement the mechanism to flush the `memtable` to persistent segment files on disk.

- **Task 1.3: Basic REST API for Document CRUD**:
  - Implement a lightweight C++ HTTP server.
  - Create endpoints for basic Create, Read, Update, and Delete operations on documents.

- **Task 1.4: TissQL Parser for Basic SELECT**:
  - Define a formal grammar for a subset of TissQL.
  - Implement a parser that can execute simple `SELECT ... WHERE` queries via a full collection scan.

- **Task 1.5: Single-Field B-Tree Indexing**:
  - Implement or integrate a B-Tree library.
  - Create an API to add an index to a field.
  - Modify the write path and query planner to use the index.

Completion of these tasks will result in the first functional, single-node version of TissDB, forming the foundation for all future development.
