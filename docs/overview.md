# QuantaTissu Project Overview

This document provides a comprehensive overview of the QuantaTissu project, its components, current capabilities, and future direction.

## 1. Introduction

QuantaTissu is a multi-faceted project centered around building an autonomous AI software engineering agent. It is comprised of several key components built from scratch:

-   **QuantaTissu**: A transformer-based language model that serves as the agent's core reasoning engine.
-   **TissLang**: A high-level, declarative language for orchestrating the agent's actions in a structured and predictable manner.
-   **TissDB**: A high-performance NoSQL database providing a persistence layer.
-   **Tissu Sinew**: A native C++ connector for TissDB.

The project is designed to be both an educational tool for understanding the inner workings of AI systems and a foundation for a sophisticated, agentic coding assistant.

## 2. QuantaTissu (Language Model)

QuantaTissu is a decoder-only transformer model built in Python with NumPy. It forms the core of the agent's intelligence.

### Implemented Features

The model has evolved significantly from its initial design and now includes a range of modern and efficient features.

-   **Architecture**:
    -   Configurable `N`-layer transformer block architecture.
    -   Centralized hyperparameter configuration.
    -   Sinusoidal positional encodings.
    -   Multi-head self-attention with causal masking.
    -   Position-wise feed-forward networks.
-   **Tokenization**:
    -   A trainable Byte-Pair Encoding (BPE) subword tokenizer, capable of handling large vocabularies and out-of-vocabulary words.
-   **Inference**:
    -   Batched inference for processing multiple prompts simultaneously.
    -   **KV Caching**: Dramatically accelerates the generation of long sequences by caching attention keys and values.
    -   **Advanced Sampling**: Supports greedy decoding, temperature scaling, top-k, and nucleus (top-p) sampling.
-   **Training**:
    -   A full training pipeline implemented from scratch.
    -   **Optimizer**: Adam optimizer with gradient clipping.
    -   **Loss Function**: Cross-entropy loss.
    -   **Scheduler**: Learning rate scheduler with cosine decay and warmup.
    -   **Utilities**: Model checkpointing and logging for tracking metrics like loss and perplexity.

### Future Work

The following enhancements are planned to further improve the model's performance and capabilities:

-   **Architectural Upgrades**:
    -   Replace ReLU-based FFN with **SwiGLU**.
    -   Implement **RMSNorm** as an alternative to LayerNorm.
    -   Implement **Rotary Positional Embeddings (RoPE)**.
    -   Add dropout layers for regularization.
-   **Efficiency and Performance**:
    -   Explore quantization (INT8/INT4), Flash Attention, and other optimization techniques.
    -   Implement distributed training strategies (Data Parallelism, ZeRO).
-   **Fine-Tuning and Alignment**:
    -   Implement Parameter-Efficient Fine-Tuning (PEFT) methods like **LoRA/QLoRA**.
    -   Implement alignment techniques like **Reinforcement Learning from Human Feedback (RLHF)** and **Direct Preference Optimization (DPO)**.

## 3. TissLang (Agentic Language)

TissLang is a structured, human-readable language designed to direct the agent's actions safely and predictably.

### Implemented Features

-   **Core Syntax**:
    -   `TASK` and `STEP` blocks to define and organize workflows.
    -   **Commands**: `WRITE` (to files), `RUN` (shell commands), `READ` (file content).
    -   **Assertions**: `ASSERT` to verify the outcomes of commands (e.g., exit codes, stdout).
-   **Parser and Execution Engine**:
    -   A parser that converts TissLang scripts into an Abstract Syntax Tree (AST).
    -   An execution engine that walks the AST and invokes the corresponding agent tools.
    -   State management to track command results for use in assertions.

### Future Work

-   Introduce advanced control flow constructs like variables, `IF/ELSE` conditions, and sub-tasks.
-   Enable the QuantaTissu agent to generate its own TissLang scripts as a "plan of action" for human review.

## 4. TissDB (NoSQL Database)

TissDB is a lightweight, high-performance NoSQL database built in C++.

### Implemented Features

-   **Storage Engine**: A Log-Structured Merge-Tree (LSM-Tree) for high write throughput.
-   **Data Model**: Flexible, JSON-like document model.
-   **Query Language**: A simple, SQL-like query language named **TissQL**.
-   **API**: A RESTful API for client interaction.
-   **Durability**: A Write-Ahead Log (WAL) ensures data durability.
-   **Indexing**: Supports B-Tree indexing for fast lookups.

### Limitations and Future Work

-   The current implementation is primarily in-memory, relying on replaying the WAL on startup.
-   B-Tree indexes are not yet persistent across restarts.
-   Full ACID transaction support is in progress.
-   Future work will focus on improving persistence, transactionality, and feature completeness.

## 5. Tissu Sinew (C++ Connector)

Tissu Sinew is the official C++ connector for TissDB.

### Implemented Features

-   **Modern C++ API**: Designed for safety and ease of use, using RAII principles and smart pointers.
-   **Connection Pooling**: A thread-safe `TissuClient` manages a connection pool to reduce connection overhead.
-   **Session Management**: `TissuSession` objects provide a non-thread-safe session for a single logical thread of execution.
-   **Error Handling**: A clear exception hierarchy for robust error handling.

## 6. Ecological Awareness

A core principle of the project is to promote sustainable AI development. This is pursued through a three-pronged strategy:
1.  **Model Efficiency**: Optimizing the model for lower energy consumption using efficient architectures and algorithms.
2.  **Green Code Generation**: Training the agent to recognize and suggest energy-efficient code.
3.  **Impact Transparency**: Building tools within TissLang to estimate the carbon footprint of code changes, empowering developers to make environmentally conscious decisions.
