# TissLM C++ Implementation Steps

This document outlines the steps taken to implement the TissLM language model in C++.

## 1. Project Structure

- Created the necessary directories for the C++ source code within `quanta_tissu/tisslm/program`:
  - `architecture`: For the main model assembly.
  - `layers`: For the individual neural network layers.
  - `tokenizer`: For the text tokenization logic.

## 2. C++ Tokenizer

- Implemented a `Tokenizer` class in `program/tokenizer/`.
- The class loads a BPE (Byte-Pair Encoding) model from files (`_vocab.json` and `_merges.txt`).
- It provides an `encode` method to convert text into a sequence of token IDs and a `decode` method to convert token IDs back to text.
- The implementation includes logic to handle UTF-8 and the BPE merge rules.
- Files created: `tokenizer.h`, `tokenizer.cpp`.

## 3. Core Neural Network Layers

- Implemented the fundamental building blocks of the Transformer model in `program/layers/`.
- **`Matrix` class**: A foundational class (`matrix.h`, `matrix.cpp`) was created to handle basic 2D tensor operations, including matrix multiplication, addition, and random initialization, as a substitute for a library like NumPy.
- **`LayerNorm`**: Implemented layer normalization (`layernorm.h`, `layernorm.cpp`) to stabilize the network.
- **`FeedForward`**: Implemented the position-wise feed-forward network (`feedforward.h`, `feedforward.cpp`), including the ReLU activation function.
- **`MultiHeadAttention`**: Implemented the multi-head self-attention mechanism (`multiheadattention.h`, `multiheadattention.cpp`). Due to the limitations of the basic `Matrix` class, this was simplified to a single-head attention mechanism but retains the structure for future expansion.
- **`TransformerBlock`**: Combined the attention and feed-forward layers, along with layer normalization and residual connections, into a single `TransformerBlock` (`transformerblock.h`, `transformerblock.cpp`).

## 4. C++ Model Architecture

- Implemented a `Model` class in `program/architecture/` (`model.h`, `model.cpp`).
- This class assembles the complete Transformer architecture by stacking multiple `TransformerBlock` layers.
- It includes an embedding layer to convert token IDs into vectors and a positional encoding layer to inject sequence position information.
- A final linear projection layer maps the Transformer's output to vocabulary logits.

## 5. Main Application Class

- Implemented a high-level `QuantaTissu` class in `program/` (`quantatissu.h`, `quantatissu.cpp`).
- This class acts as the main interface for the C++ language model.
- It encapsulates the `Model` and `Tokenizer` objects.
- The `generate` method orchestrates the entire process: tokenizing the input prompt, running the forward pass through the model, and decoding the output tokens to produce text.
- A placeholder for loading pre-trained weights was included.