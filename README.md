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

## TissDB: A High-Performance NoSQL Database

TissDB is a lightweight, high-performance NoSQL database built from scratch in C++. It is designed to be a simple, embeddable database for C++ applications.

### Features

*   **LSM-Tree Storage Engine:** TissDB uses a log-structured merge-tree (LSM-Tree) for high write throughput.
*   **JSON-like Document Model:** TissDB stores data in a flexible, JSON-like document model.
*   **TissQL Query Language:** TissDB provides a simple, SQL-like query language called TissQL for querying data.
*   **B-Tree Indexing:** TissDB supports B-Tree indexing for fast lookups.
*   **RESTful API:** TissDB provides a RESTful API for interacting with the database.

More details can be found in `tissdb/README.md` and the documents in the `docs/` directory.

## Tissu Sinew: C++ Connector for TissDB

Tissu Sinew is a lightweight, high-performance C++ connector for TissDB. It provides a native C++ interface for applications to communicate with the database server, featuring a thread-safe client with connection pooling and RAII-based session management.

A comprehensive guide to building and using the connector, including API reference and code examples, can be found in `docs/tissu_sinew_plan.md`.

## Ecological Awareness

QuantaTissu aims to be a demonstration of how AI can be developed and used in an environmentally conscious manner. This involves a three-pronged approach to ecological awareness:

1.  **Optimize the model for lower energy consumption.**
2.  **Generate code that is more energy-efficient.**
3.  **Have the AI agent reflect on the environmental impact of the code it writes.**

More details on this initiative can be found in `docs/ecological_awareness.md`.

## Features

- Trainable Byte-Pair Encoding (BPE) tokenizer.
- Full training pipeline with a custom Adam optimizer and backpropagation from scratch.
- Configurable transformer model (d_model, n_heads, n_layers).
- Sinusoidal positional encoding.
- Multi-head self-attention with causal masking for correct autoregressive behavior.
- Batched inference with greedy, top-k, and nucleus sampling methods.
- Saving and loading of trained model weights and tokenizers.

## Requirements

- Python 3.7+
- NumPy, Regex
- See `requirements.txt` for specific versions.

## Usage

### Training

To train the model from scratch on the provided corpus:
```bash
python quanta_tissu/tisslm/train.py
```
This will train a new tokenizer and save it to `models/tokenizer.json`, and save the trained model weights to `models/quanta_tissu.npz`.

### Inference

To run inference with a trained model:
```bash
python quanta_tissu/scripts/run_inference.py --prompt "Your prompt here"
```

## How it Works

The model is a decoder-only transformer built from scratch using NumPy.

1.  **Tokenization**: Input text is converted into token IDs using a trained Byte-Pair Encoding (BPE) tokenizer.
2.  **Embeddings**: Token IDs are mapped to dense vectors.
3.  **Positional Encoding**: Sinusoidal encodings are added to give the model positional information.
4.  **Transformer Blocks**: The sequence is processed by a stack of N transformer blocks. Each block contains:
    *   A multi-head self-attention layer with a causal mask to ensure a token can only attend to previous tokens.
    *   A position-wise feed-forward network.
    *   Residual connections and layer normalization are applied around each sub-layer.
5.  **Output Projection**: The final output is passed through a linear layer to produce logits over the vocabulary.
6.  **Prediction**: The next token is predicted from the logits using a chosen sampling strategy (e.g., greedy, top-k, nucleus).

A detailed implementation plan can be found in `docs/plan.md`, and a list of potential enhancements is in `docs/enhancements.md`.

## Testing

The project includes a testing plan to ensure the correctness of individual components and the overall behavior of the application. The testing strategy includes:

-   **Unit Tests**: To verify the correctness of individual components like the tokenizer, core math functions, attention mechanism, and transformer block.
-   **Behavior-Driven Development (BDD) Tests**: To ensure the application behaves as expected from a user's perspective, with scenarios for text generation and handling of unknown words.

The full test plan is available in `docs/test.md`.

## Future Work
This project serves as a foundation. The `docs/enhancements.md` file contains a detailed list of potential improvements. Key areas include:

-   **Advanced Inference**: Implement performance optimizations like **KV Caching** to accelerate the generation of long sequences.
-   **Architectural Enhancements**:
    -   Upgrade components with modern alternatives like **SwiGLU**, **RMSNorm**, and **RoPE**.
    -   Implement dropout for regularization.
-   **Usability and Tooling**: Create visualization tools for attention maps and build an interactive web demo.

## Project Structure

The repository is organized into several key directories:

```
.
├── docs/              # Documentation, design docs, and specifications
├── quanta_tissu/      # Python-based language model (QuantaTissu)
│   ├── tisslm/        # Source code for the QuantaTissu model
│   └── scripts/       # Scripts to run and interact with the model
├── tissdb/            # C++ NoSQL database (TissDB)
│   ├── api/           # RESTful API implementation
│   ├── common/        # Shared data structures
│   ├── json/          # JSON parser
│   ├── query/         # TissQL query parser and executor
│   └── storage/       # LSM-Tree storage engine
├── tests/             # Unit and integration tests
│   ├── db/            # C++ tests for TissDB
│   └── features/      # BDD tests for TissLang
└── README.md
```
