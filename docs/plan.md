# QuantaTissu Project Plan

This document outlines the plan for creating the QuantaTissu application, a minimal transformer-based language model inference engine.

## 1. Core Components

### 1.1. Tokenizer and Vocabulary

-   **Vocabulary**: Create a small, fixed vocabulary of common English words.
-   **Tokenizer**: Implement a simple word-based tokenizer that converts text to a sequence of token IDs.
-   **Detokenizer**: Implement a function to convert a sequence of token IDs back to text.

### 1.2. Transformer Building Blocks

-   **Embeddings**: Create an embedding layer to convert token IDs into dense vectors.
-   **Positional Encoding**: Implement sinusoidal positional encoding to inject position information into the embeddings.
-   **Scaled Dot-Product Attention**: Implement the core attention mechanism.
-   **Multi-Head Attention**: Implement multi-head attention by combining multiple scaled dot-product attention operations.
-   **Feed-Forward Network**: Implement a position-wise feed-forward network.
-   **Layer Normalization**: Implement layer normalization to stabilize the network.

## 2. Transformer Block

-   Combine the multi-head attention and feed-forward network to create a single transformer block.
-   Incorporate residual connections and layer normalization within the block.

## 3. Main Model

-   **QuantaTissu Class**: Create the main model class that encapsulates all the components.
-   **Forward Pass**: Implement the forward pass of the model, which takes token IDs as input and produces logits as output.
-   **Inference**: Implement a `predict` method for greedy next-token prediction.

## 4. Example Usage

-   Create a simple example script that demonstrates how to use the model to generate text.
-   The script should initialize the model, tokenize a prompt, and generate the next token.
