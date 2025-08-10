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

## TissLang: The Agentic Language

To steer the agent, we are developing **TissLang**, a high-level, declarative language designed specifically for orchestrating agentic workflows. It allows developers to define complex tasks in a structured, human-readable format.

A TissLang script breaks a high-level goal into sequential steps, with commands to execute actions and assertions to verify outcomes.

### Core Concepts

-   **`TASK`**: Defines the overall objective.
-   **`STEP`**: A logical unit of work within the task.
-   **Commands**: Specific actions like `WRITE` (to the file system), `RUN` (a shell command), or `PROMPT_AGENT` (to invoke the LLM).
-   **`ASSERT`**: Verifies the outcome of commands (e.g., `ASSERT LAST_RUN.EXIT_CODE == 0`).
-   **Directives**: Metadata, like `@persona`, to guide the agent's behavior.
-   **Advanced Control Flow**: Features like `PARALLEL` for concurrent execution and `CHOOSE` for conditional logic.

### Example

Here is a simple TissLang script to create and test a Python file:

```tiss
#TISS! Language=Python

TASK "Create and test a simple Python hello world script"

STEP "Create the main application file" {
    WRITE "main.py" <<PYTHON
import sys

def main():
    print(f"Hello, {sys.argv[1]}!")

if __name__ == "__main__":
    main()
PYTHON
}

STEP "Run the script and verify its output" {
    RUN "python main.py TissLang"
    ASSERT LAST_RUN.EXIT_CODE == 0
    ASSERT LAST_RUN.STDOUT CONTAINS "Hello, TissLang!"
}
```

This structured approach is key to enabling the agent to handle complex, multi-step tasks autonomously.

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
python quanta_tissu/scripts/run_inference.py
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

A detailed implementation plan can be found in `docs/plan.md`.

## Testing

The project includes a testing plan to ensure the correctness of individual components and the overall behavior of the application. The testing strategy includes:

-   **Unit Tests**: To verify the correctness of individual components like the tokenizer, core math functions, attention mechanism, and transformer block.
-   **Behavior-Driven Development (BDD) Tests**: To ensure the application behaves as expected from a user's perspective, with scenarios for text generation and handling of unknown words.

The full test plan is available in `docs/test.md`. A `tests/` directory for these tests is planned but not yet implemented.

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

The project is organized as a Python package. The code is structured as follows:

```
quanta_tissu/
├── quanta_tissu/
│   ├── __init__.py      # Makes the directory a Python package
│   ├── model.py         # Contains TransformerBlock and QuantaTissu classes
│   ├── layers.py        # Contains Attention, FeedForward, LayerNorm
│   ├── tokenizer.py     # Tokenizer implementation
│   ├── config.py        # Model hyperparameters and vocabulary
│   └── knowledge_base.py # Simple vector store for RAG
├── scripts/
│   └── run_inference.py # Example usage script
├── docs/                # Project documentation
├── README.md
└── requirements.txt
```
