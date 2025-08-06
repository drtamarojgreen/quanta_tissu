# QuantaTissu Project Plan

This document outlines the development plan for **QuantaTissu**, a minimal transformer-based language model inference engine built from scratch using NumPy. The plan is divided into phases to ensure a structured and incremental development process.

## Phase 1: Foundational Components

The goal of this phase is to establish the basic data processing and mathematical utilities required for the model.

### 1.1. Tokenizer
-   **Vocabulary**: Define a small, fixed vocabulary of common English words, including a special `<unk>` token for unknown words.
-   **`tokenize(text: str) -> list[int]`**: Implement a simple word-based tokenizer that splits text by spaces and converts words to their corresponding token IDs.
-   **`detokenize(token_ids: list[int]) -> str`**: Implement a function to convert a sequence of token IDs back to a space-separated string.

### 1.2. Core Math and Normalization
-   **`softmax(x)`**: Implement the softmax function for converting logits to probabilities.
-   **`LayerNorm`**: Implement a layer normalization class to stabilize the network. It should handle learnable gain and bias parameters.

## Phase 2: Transformer Building Blocks

This phase focuses on building the core architectural components of the Transformer model.

### 2.1. Input Processing
-   **Embeddings**: Create an embedding layer (a simple lookup table) to convert token IDs into dense vectors of dimension `d_model`.
-   **Positional Encoding**: Implement a class to generate and add sinusoidal positional encodings to the input embeddings.

### 2.2. Attention Mechanism

-   **QuantaTissu Class**: Create the main model class that encapsulates all the components.
-   **Forward Pass**: Implement the forward pass of the model, which takes token IDs as input and produces logits as output.
-   **Inference**: Implement a `predict` method for greedy next-token prediction.

## 4. Example Usage

-   Create a simple example script that demonstrates how to use the model to generate text.
-   The script should initialize the model, tokenize a prompt, and generate the next token.
