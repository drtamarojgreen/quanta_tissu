# QuantaTissu

QuantaTissu is a minimal transformer-based language model inference engine built **from scratch** in Python using only NumPy. It implements basic tokenization, embeddings, positional encoding, multi-head self-attention, feed-forward layers, and output projection.

This project serves two purposes:
1.  As an educational tool to understand the inner workings of a transformer model.
2.  As the foundational first step towards building a sophisticated, agentic coding assistant.

## Project Vision: An Agentic Coding Assistant

The long-term vision for QuantaTissu is to evolve it from a foundational language model into an autonomous agent that can interact with a development environment to write, debug, and test code. This involves enhancing the core model and then building agentic capabilities on top of it.

The roadmap is divided into two main phases:
1.  **Foundational Model Enhancement**: Scaling up the model, implementing a full-scale training pipeline with backpropagation, and establishing rigorous evaluation benchmarks.
2.  **Agentic Capabilities Development**: Integrating tools for file system operations and shell command execution, implementing a planning and reasoning engine, and creating a feedback loop for self-correction.

More details can be found in `docs/agent_strategies.md`.

## Ecological Awareness

QuantaTissu aims to be a demonstration of how AI can be developed and used in an environmentally conscious manner. This involves a three-pronged approach to ecological awareness:

1.  **Optimize the model for lower energy consumption.**
2.  **Generate code that is more energy-efficient.**
3.  **Have the AI agent reflect on the environmental impact of the code it writes.**

More details on this initiative can be found in `docs/ecological_awareness.md`.

## Features

- Tiny toy vocabulary tokenizer
- Positional encoding (sinusoidal)
- Single transformer decoder block
- Batched inference support
- Greedy next-token prediction

## Requirements

- Python 3.7+
- NumPy

## Usage

```bash
python quanta_tissu.py
```

## How it Works

The model takes a sequence of tokens as input and predicts the next token in the sequence. Here's a step-by-step breakdown of the process:

1.  **Tokenization**: The input text is converted into a sequence of token IDs using a simple vocabulary.
2.  **Embeddings**: Each token ID is mapped to a dense vector representation called an embedding.
3.  **Positional Encoding**: Sinusoidal positional encodings are added to the embeddings to give the model information about the position of each token in the sequence.
4.  **Transformer Block**: The sequence of embeddings is processed by a transformer block, which consists of:
    *   A multi-head self-attention layer, which allows the model to weigh the importance of different tokens in the input sequence when processing each token.
    *   A layer normalization step.
    *   A feed-forward neural network.
    *   Another layer normalization step.
5.  **Output Projection**: The output of the transformer block is passed through a final linear layer (output projection) to produce a vector of logits for each token in the input sequence.
6.  **Prediction**: The model uses a greedy approach to predict the next token. It selects the token with the highest logit value from the last position in the output sequence.

## Code Overview

The `quanta_tissu.py` file contains all the components of the model:

-   **`Tokenizer`**: A class that encapsulates the vocabulary and provides `tokenize` and `detokenize` methods.
-   **`softmax(x)`**: The core softmax activation function for converting logits to probabilities.
-   **`LayerNorm`**: A layer normalization class to stabilize hidden states.
-   **`scaled_dot_product_attention(Q, K, V)`**: The fundamental attention mechanism.
-   **`MultiHeadAttention`**: The complete multi-head self-attention layer, which runs several attention computations in parallel.
-   **`FeedForward`**: A standard position-wise feed-forward network (two linear layers with a ReLU activation).
-   **`TransformerBlock`**: A single decoder block that encapsulates multi-head attention and a feed-forward network, each followed by a residual connection and layer normalization.
-   **`PositionalEncoding`**: A class to generate and add sinusoidal positional information to the input embeddings.
-   **`QuantaTissu`**: The main model class that orchestrates all components, from input embeddings to the final output projection layer.
-   **`if __name__ == "__main__"`**: A simple demonstration of initializing the model, tokenizing a prompt, adding a batch dimension, and predicting the next token.

A detailed implementation plan can be found in `docs/plan.md`.

## Testing

The project includes a testing plan to ensure the correctness of individual components and the overall behavior of the application. The testing strategy includes:

-   **Unit Tests**: To verify the correctness of individual components like the tokenizer, core math functions, attention mechanism, and transformer block.
-   **Behavior-Driven Development (BDD) Tests**: To ensure the application behaves as expected from a user's perspective, with scenarios for text generation and handling of unknown words.

The full test plan is available in `docs/test.md`.

## Limitations

This is a toy model and is not suitable for any real-world applications. It has several limitations:

-   **No Training**: The model weights are randomly initialized and are not trained on any data. This means the model's predictions are essentially random.
-   **Tiny Vocabulary**: The vocabulary is extremely small and can only handle a few predefined words.
-   **Greedy Decoding**: The model uses a simple greedy decoding strategy, which may not produce the most optimal sequence of tokens.
-   **Single Transformer Block**: The model has only a single transformer block, which limits its ability to learn complex patterns.

## Future Work
This project serves as a foundation. The `docs/enhancements.md` file contains a detailed list of potential improvements. Key areas include:

-   **Training Pipeline**: Implement a full training loop, including a cross-entropy loss function, an Adam/AdamW optimizer, and backpropagation to train the model on a real dataset.
-   **Advanced Inference**: Move beyond greedy decoding by implementing more sophisticated sampling strategies like **top-k**, **nucleus (top-p)**, and **temperature scaling** to generate more diverse and coherent text.
-   **Architectural Enhancements**:
    -   Generalize the model to stack multiple transformer blocks.
    -   Implement performance optimizations like **KV Caching** to accelerate generation.
    -   Add support for loading **pre-trained weights**.
    -   Upgrade components with modern alternatives like **SwiGLU**, **RMSNorm**, and **RoPE**.
-   **Better Tokenization**: Replace the basic tokenizer with a subword-based method like **Byte-Pair Encoding (BPE)** to handle a larger vocabulary and unknown words effectively.
-   **Usability and Tooling**: Improve the project structure, add a proper CLI, and create visualization and interactive demo tools.

## Project Structure

Currently, all code resides in a single file for simplicity. A more scalable structure would be:

```
quanta_tissu/
├── quanta_tissu/
│   ├── __init__.py
│   ├── model.py         # Contains TransformerBlock, QuantaTissu
│   ├── layers.py        # Contains Attention, FeedForward, LayerNorm
│   ├── tokenizer.py     # Tokenizer implementation
│   └── config.py        # Model hyperparameters
├── scripts/
│   └── run_inference.py # Example usage script
├── docs/
├── tests/
├── README.md
└── requirements.txt
```
