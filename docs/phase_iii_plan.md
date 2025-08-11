# Phase III Development Plan: Model Enhancement and Usability

This document provides a detailed plan for the tasks required to complete **Phase 3** of the [QuantaTissu Project Roadmap](./roadmap.md). The goal of this phase is to evolve the model from a "trainable toy" into a capable, efficient, and usable system by implementing modern architectural components and performance optimizations.

## 1. Overview

With the training pipeline established in Phase II, this phase focuses on three key areas:
1.  **Architectural Modernization**: Upgrading the core model with state-of-the-art components.
2.  **Performance Optimization**: Drastically improving inference speed.
3.  **Usability and Capability**: Enabling the model to handle real-world data and providing better tools for interaction.

## 2. Key Components & Implementation Steps

### 2.1. Core Model Architecture Enhancement
-   **Objective**: Upgrade the model's architecture with modern, high-performance components to improve its learning capacity and efficiency.
-   **Tasks**:
    -   **Implement `RMSNorm`**: Replace the existing `LayerNorm` with Root Mean Square Normalization for better performance and stability.
    -   **Implement `SwiGLU` Activation**: Upgrade the `FeedForward` network to use the Swish-Gated Linear Unit (SwiGLU) activation function, which often yields better results than standard ReLU.
    -   **Implement Rotary Position Embeddings (RoPE)**: Replace the basic absolute positional embeddings with RoPE to better capture relative positional information, a standard in modern LLMs.
    -   **Enable Multi-Layer Stacking**: Refactor the `QuantaTissu` class to support a configurable number of `TransformerBlock` layers, moving beyond the single-block limitation.

### 2.2. Real-World Data Handling (Tokenization)
-   **Objective**: Replace the "toy vocabulary" with a robust, subword-based tokenizer capable of handling a large, real-world corpus.
-   **Tasks**:
    -   **Implement BPE Tokenizer**: Build a `Byte-Pair Encoding (BPE)` tokenizer class from scratch.
    -   **Create Tokenizer Training Script**: Develop a script (`scripts/train_tokenizer.py`) that loads text from the `corpus/` directory and trains the BPE tokenizer to create a vocabulary file.
    -   **Integrate Tokenizer**: Replace the old tokenizer in the data handling pipeline, ensuring the model's embedding layer is resized to match the new vocabulary size.

### 2.3. Performance Optimization for Inference
-   **Objective**: Make text generation practical and fast by implementing a key-value cache.
-   **Tasks**:
    -   **Implement KV Cache**: Modify the `MultiHeadAttention` layer to include a cache for the key (K) and value (V) projections. During generation, this cache will store past values so they don't need to be recomputed for every new token.
    -   **Update Generation Logic**: Refactor the `generate()` method to accept and pass the KV cache between token generation steps, ensuring it's used correctly.

### 2.4. Advanced Generation and Usability
-   **Objective**: Improve the quality and controllability of the generated output and provide a user-friendly interface for interaction.
-   **Tasks**:
    -   **Implement Advanced Sampling**: Extend the `generate()` method to support more sophisticated decoding strategies beyond greedy search:
        -   Temperature Scaling
        -   Top-k Sampling
        -   Nucleus Sampling (Top-p)
    -   **Create Generation CLI**: Build a command-line interface (`scripts/run_generation.py`) using `argparse` that allows a user to load a trained model checkpoint and generate text with various sampling parameters.

### 2.5. Foundational Agentic Framework (`TissLang`)
-   **Objective**: Begin scaffolding the `TissLang` agentic framework, which is a long-term goal of the project.
-   **Tasks**:
    -   **Develop Initial Parser**: Create the first version of the `TissLang` parser, focusing on correctly parsing `STEP` blocks and `WRITE` commands with heredocs.
    -   **Define AST**: Define the Abstract Syntax Tree (AST) data structures that the parser will produce.

## 3. Definition of Done

Phase III will be considered complete when:

-   The `QuantaTissu` model can be configured with multiple layers and initialized with `RMSNorm`, `SwiGLU`, and `RoPE`.
-   A BPE tokenizer can be successfully trained on the local `corpus/` and used for a training run.
-   The `run_generation.py` script can generate text from a checkpoint, and using the KV cache results in a measurable speedup.
-   The generation script allows the user to specify temperature, top-k, and top-p sampling.
-   A basic `TissLang` script can be fed to the new parser, which produces a valid AST without errors.