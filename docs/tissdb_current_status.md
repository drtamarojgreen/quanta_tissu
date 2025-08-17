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

**The C++ NoSQL database described in the development plan has been implemented, and its Phase 1 (MVP) is now largely complete.**

The core of the TissDB project is the functional C++ database located in the `tissdb/` directory. This implementation serves as the foundational layer for the long-term TissLang vision.

The key completed features of the C++ database MVP include:
- **Core Storage Engine**: A functional storage layer based on a Log-Structured Merge-Tree (LSM-Tree), with a Write-Ahead Log (WAL), Memtables, and SSTables.
- **Collection Management**: The database supports the creation, deletion, and management of named collections to organize documents.
- **Document CRUD API**: A RESTful API is available for Creating, Reading, Updating, and Deleting documents within collections.
- **TissQL Query Engine**: A parser and executor for a subset of TissQL, allowing `SELECT ... WHERE` queries on collections.
- **Single-Field Indexing**: The database supports B-Tree indexing on single fields to accelerate queries.

While the core MVP is functional, there are still known limitations (e.g., no transaction support, in-memory B-Tree), which are documented in the `tissdb/README.md`. The other components, such as the Python Knowledge Store and the Graph Visualization utility, remain as separate, related tools.
2.  **A Functional Python Knowledge Store (`quanta_tissu/tisslm/knowledge_base.py`)**:
    - This is the primary piece of *functional* code related to the TissDB concept.
    - It is an **in-memory vector store**, not the planned C++ database.
    - It uses embeddings to store and retrieve documents based on semantic similarity, serving as a practical tool for knowledge management within the Quanta ecosystem.

## 5. Immediate Next Steps: Towards Phase 2

With the Phase 1 MVP substantially complete, active development work will now focus on the key features outlined for Phase 2. The immediate roadmap includes:

- **Compound Indexing**: Enhancing the indexing engine to support indexes on multiple fields.
- **Transactions**: Introducing support for multi-document ACID transactions.
- **Persistence for Indexes**: Modifying the B-Tree implementation to be persistent across restarts.
- **Replication and Sharding**: Designing and implementing strategies for data replication (for high availability) and sharding (for horizontal scaling).
- **Security Enhancements**: Building a security model with user authentication and Role-Based Access Control (RBAC).

Completion of these tasks will evolve TissDB from a functional MVP into a more robust, scalable, and secure database, bringing it closer to the long-term vision.
