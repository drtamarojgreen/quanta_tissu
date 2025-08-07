# QuantaTissu Project Plan

This document outlines the development plan for **QuantaTissu**, a minimal transformer-based language model inference engine built from scratch using NumPy. The plan is divided into phases to ensure a structured and incremental development process.

## Phase 1: Foundational Components

The goal of this phase is to establish the basic data processing and mathematical utilities required for the model.

### 1.1. Tokenizer
-   **`Tokenizer` Class**: Create a class to encapsulate tokenization logic.
    -   **Vocabulary**: Inside the class, define a small, fixed vocabulary of common English words, including a special `<unk>` token.
    -   **`tokenize(text: str) -> np.ndarray`**: Implement a method that converts text to a NumPy array of token IDs.
    -   **`detokenize(token_ids: np.ndarray) -> str`**: Implement a method to convert an array of token IDs back to text.

### 1.2. Core Mgath and Normalization
-   **`softmax(x)`**: Implement the softmax function for converting logits to probabilities.
-   **`LayerNorm`**: Implement a layer normalization class to stabilize the network. It should handle learnable gain and bias parameters.

## Phase 2: Transformer Building Blocks

This phase focuses on building the core architectural components of the Transformer model.

### 2.1. Input Processing
-   **Embeddings**: Create an embedding layer (a simple lookup table) to convert token IDs into dense vectors of dimension `d_model`.
-   **Positional Encoding**: Implement a class to generate and add sinusoidal positional encodings to the input embeddings.

### 2.2. Attention Mechanism

-   **`scaled_dot_product_attention(Q, K, V, mask=None)`**: Implement the core attention mechanism. It should calculate `softmax(Q @ K.T / sqrt(d_k)) @ V`. Include an optional mask for causal attention (initially unused but good for future training).
-   **`MultiHeadAttention`**: Implement a multi-head attention class. This class will contain linear layers for Q, K, and V projections, and an output projection layer. It will manage splitting the inputs into multiple heads, applying scaled dot-product attention in parallel, and concatenating the results.

### 2.3. Feed-Forward Network
-   **`FeedForward`**: Implement a position-wise feed-forward network class. This will consist of two linear layers with a ReLU activation function in between.

## Phase 3: Assembling the Transformer Block

-   **`TransformerBlock`**: Create a class for the Transformer block, combining an instance of `MultiHeadAttention` and `FeedForward` with residual connections and layer normalization.

## Phase 4: Final Model Construction and Inference

-   **`QuantaTissu` Class**: Create the main model class that assembles all the components.
-   **Forward Pass**: Implement the `forward` method. It should be designed to handle **batched inputs** (e.g., `(batch_size, seq_len)`) and produce logits of shape `(batch_size, seq_len, vocab_size)`.
-   **Inference**: Implement a `predict` method for greedy next-token prediction that operates on a batch of token sequences.

## Phase 5: Demonstration and Documentation

-   **Example Script**: Create a main execution block (`if __name__ == "__main__":`) that demonstrates end-to-end functionality:
    -   Instantiate the `Tokenizer` and `QuantaTissu` model.
    -   Tokenize a sample prompt.
    -   Add a batch dimension to the tokens.
    -   Generate the next token and print the detokenized result.
-   **Documentation**: Write the initial `README.md`, `plan.md`, and `enhancements.md`.

## Phase 6: Agentic Control with TissLang

With the core inference engine established, this phase introduces **TissLang**, a dedicated command language for directing the QuantaTissu agent. This moves the project from a simple text generator to a system capable of executing complex, multi-step tasks in a structured and reviewable manner.

### 6.1. TissLang Specification and Parser
-   **Objective**: Define and parse the TissLang language.
-   **Tasks**:
    -   **Grammar Definition**: Specify the syntax for core commands like `TASK`, `STEP`, `RUN`, `WRITE`, `READ`, and `ASSERT`.
    -   **Parser Implementation**: Build a line-by-line parser that converts a `.tiss` script into a JSON-serializable Abstract Syntax Tree (AST).

### 6.2. Execution Engine
-   **Objective**: Create an interpreter to execute the parsed TissLang AST.
-   **Tasks**:
    -   **AST Walker**: Develop an engine that traverses the AST and invokes corresponding agent tools for each command.
    -   **State Management**: Implement a state object to track the results of commands (e.g., `LAST_RUN.STDOUT`) for use in `ASSERT` conditions.
    -   **Tool Integration**: Connect the engine to the agent's core capabilities for file I/O and shell command execution.
