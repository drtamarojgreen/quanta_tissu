# Phase III Development Plan: Advanced Architecture & Features

This document provides a detailed plan for the tasks required to complete **Phase 3** of the [QuantaTissu Project Roadmap](./roadmap.md). The goal of this phase is to modernize the model's architecture with state-of-the-art components, dramatically improve its performance, and expand its usability.

## 1. Overview

The "Advanced Architecture & Features" phase focuses on replacing the initial, simple components with more efficient and powerful alternatives found in modern large language models. This phase is critical for making the model both faster and more capable, paving the way for it to tackle more complex tasks.

## 2. Key Components & Implementation Steps

### 2.1. Architectural Modernization
-   **Objective**: Replace foundational components with more efficient and powerful alternatives to improve model quality and training stability.
-   **Tasks**:
    -   **RMSNorm**:
        -   Create an `RMSNorm` class as a direct, simpler alternative to `LayerNorm`.
        -   Implement the root mean square normalization formula.
        -   Add a configuration flag in `config.py` to allow the model to be initialized with either `LayerNorm` or `RMSNorm`.
    -   **SwiGLU Feed-Forward Network**:
        -   Create a `SwiGLU` class to replace the standard `FeedForward` (ReLU-based) network.
        -   Implement the three weight matrices required for the gated linear unit.
        -   Implement the SwiGLU formula: `(x @ W_gate * sigmoid(x @ W_gate)) @ W_down`.
    -   **Rotary Positional Embeddings (RoPE)**:
        -   Create a `RoPE` class or utility functions to replace the `PositionalEncoding` layer.
        -   Pre-compute the rotary frequency matrices based on the model's dimension and a base period.
        -   Modify the `MultiHeadAttention` layer to apply these rotary embeddings directly to the Query (Q) and Key (K) vectors before the attention calculation. This correctly applies positional information at each layer, rather than just once at the input.

### 2.2. Performance Optimization for Inference
-   **Objective**: Drastically reduce latency and memory usage during text generation, making the model practical for interactive use.
-   **Tasks**:
    -   **KV Caching**:
        -   Modify the `TransformerBlock`'s `__call__` method to accept and return a `cache` object.
        -   This cache will store the Key (K) and Value (V) tensors computed for all previous tokens.
        -   During generation, the model will only perform the forward pass for the *newest* token.
        -   The new K and V vectors will be concatenated with the cached K and V vectors before the attention calculation.
        -   Update the `predict` method to manage and pass this cache between each step of token generation.

### 2.3. Model Scalability
-   **Objective**: Enable the creation of deeper, more capable models by stacking transformer blocks.
-   **Tasks**:
    -   **Stacked Transformer Blocks**:
        -   Modify the `QuantaTissu` class to accept an `n_layers` hyperparameter from the configuration.
        -   Instead of a single `self.transformer` instance, create a list of `TransformerBlock` instances.
        -   Update the `forward` method to sequentially loop through the list of blocks, passing the output of one block as the input to the next.

### 2.4. Advanced Tokenization
-   **Objective**: Move from a toy vocabulary to a robust, subword-based tokenizer capable of handling real-world code and natural language.
-   **Tasks**:
    -   **BPE Tokenizer Implementation**:
        -   Implement the core Byte-Pair Encoding (BPE) algorithm. This includes logic for finding the most frequent byte pair and merging it.
    -   **Tokenizer Training Script**:
        -   Create a script (`scripts/train_tokenizer.py`) that can train the BPE tokenizer on a given text corpus (e.g., a large `.txt` file).
        -   The script should save the learned vocabulary and merge rules to a file.
    -   **Integration**:
        -   Update the `Tokenizer` class to be able to load a trained BPE model and use it for tokenization and detokenization.

### 2.5. Tooling and Interoperability
-   **Objective**: Make the trained model accessible to other applications and users through standard interfaces.
-   **Tasks**:
    -   **API Server**:
        -   Create a simple web server using Flask or FastAPI in a new script (`scripts/run_api.py`).
        -   The server should load a trained `QuantaTissu` model.
        -   It should expose a `/generate` endpoint that accepts a JSON payload with a prompt and generation parameters (e.g., max new tokens) and returns the generated text.
    -   **Interactive Demo**:
        -   Create a simple web-based UI using Gradio or Streamlit (`scripts/run_demo.py`).
        -   The demo should provide a text box for a user to enter a prompt and display the model's generated output in real-time.

## 3. Definition of Done

Phase III will be considered complete when:

-   The model can be configured to use any combination of the new architectural components (RMSNorm, SwiGLU, RoPE).
-   The inference process for generating long sequences is significantly faster due to the implementation of KV Caching.
-   The model can be initialized with an arbitrary number of layers (`n_layers`).
-   A BPE tokenizer can be successfully trained on a sample corpus and used by the model for inference.
-   The model's generation capabilities are successfully exposed via both a local API server and an interactive web demo.