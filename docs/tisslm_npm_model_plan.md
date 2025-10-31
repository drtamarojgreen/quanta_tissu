# TissLM Node.js Implementation Plan

This document outlines the detailed plan for re-implementing the TissLM C++ and Python models in Node.js. The new implementation will be self-contained within the `quanta_tissu/tisslm/js/` directory and will replicate the full functionality of the existing models.

## Project Structure

The Node.js project will be organized into the following modular directories:

- `core/`: Contains the fundamental numerical and linear algebra classes, primarily a `Matrix` or `Tensor` class.
- `tokenizer/`: The BPE (Byte Pair Encoding) tokenizer implementation.
- `database/`: A client for interacting with the existing `tissdb` server.
- `transformer/`: The components of the Transformer model architecture.
- `training/`: The training pipeline, including the optimizer, loss function, and data loaders.
- `generation/`: The text generation and inference logic.
- `api/`: The public-facing API for the Node.js model.

## Implementation Phases

The project will be developed in the following phases to ensure a structured and incremental build-out of the model's capabilities.

### Phase 1: Core Numerical Library

The foundation of the entire model is a robust numerical library.

- **Task:** Create a `Matrix` (or `Tensor`) class in `js/core/`.
- **Requirements:**
    - The class should handle multi-dimensional arrays efficiently.
    - Implement fundamental matrix operations:
        - Addition, subtraction, multiplication (element-wise and dot product).
        - Transposition.
        - Slicing and indexing.
        - Reshaping.
        - Activation functions (ReLU, Softmax).
- **Considerations:** Decide whether to use a third-party library like `ndarray` or build from scratch for maximum control and understanding. Given the "no new libraries" directive from the user's memory, this will be built from scratch.

### Phase 2: BPE Tokenizer

Re-implement the BPE tokenizer to process text data.

- **Task:** Create a `Tokenizer` class in `js/tokenizer/`.
- **Requirements:**
    - Load vocabulary and merge files (`_vocab.json`, `_merges.txt`).
    - Implement the `encode` function to convert text into a sequence of token IDs.
    - Implement the `decode` function to convert a sequence of token IDs back into text.
    - Match the pre-tokenization and merging logic of the existing C++ and Python tokenizers to ensure compatibility.

### Phase 3: Database Client

Create a client to communicate with the `tissdb` server.

- **Task:** Implement a `TissDBClient` class in `js/database/`.
- **Requirements:**
    - Implement methods for `GET`, `POST`, `PUT`, and `DELETE` requests to the `tissdb` API.
    - Handle authentication, including sending the static bearer token.
    - Implement methods for core database operations: `create_collection`, `add_document`, `get_document`, `query`.
    - Ensure the client matches the API conventions used by the Python BDD tests (e.g., URL structure).

### Phase 4: Transformer Model Architecture

Build the neural network components of the Transformer model.

- **Task:** Implement the Transformer architecture in `js/transformer/`.
- **Requirements:**
    - `Embedding`: A layer to convert token IDs into dense vectors.
    - `PositionalEncoding`: A layer to inject positional information into the embeddings.
    - `LayerNorm`: Layer Normalization.
    - `SelfAttention`: The self-attention mechanism, including multi-head attention.
    - `FeedForward`: The feed-forward network component.
    - `TransformerBlock`: A single block that combines self-attention, layer normalization, and a feed-forward network.
    - `TransformerModel`: The full model that stacks multiple `TransformerBlock`s.

### Phase 5: Training Pipeline

Develop the components necessary to train the model.

- **Task:** Implement the training pipeline in `js/training/`.
- **Requirements:**
    - `Dataset`: A class to load and batch training data from `tissdb` or files.
    - `Optimizer`: Implement an optimizer algorithm, such as Adam, to update model parameters.
    - `LossFunction`: Implement the Cross-Entropy Loss function.
    - `TrainingLoop`: The main loop that iterates over the dataset, performs forward and backward passes, and updates the model.

### Phase 6: Text Generation

Implement the logic for generating text (inference).

- **Task:** Create a `Generator` class in `js/generation/`.
- **Requirements:**
    - Implement the core `generate` method that takes a prompt and returns a generated sequence.
    - Implement sampling strategies, such as greedy decoding, and potentially more advanced methods like top-k sampling.
    - Manage the model's key-value cache for efficient generation.

### Phase 7: API and Integration

Define the public API for the Node.js model and integrate all components.

- **Task:** Design and implement the main API in `js/api/`.
- **Requirements:**
    - Create a simple, high-level API for the most common use cases: `tokenize`, `train`, `generate`.
    - Ensure all components (tokenizer, model, database client) are correctly integrated and work together.
    - Provide clear documentation and examples for using the API.
