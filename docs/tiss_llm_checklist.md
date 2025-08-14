# Tiss LLM Implementation Checklist

This document tracks the remaining engineering tasks for the QuantaTissu language model and the TissLang execution environment.

## Phase 1: TissLang Execution Engine

The TissLang parser is complete, but the execution engine that runs the parsed scripts is the next critical step. This phase focuses on building the interpreter as specified in `docs/TissLang_plan.md`.

### 1.1. Core Engine and State Management
- [ ] **`State` Class**: Implement a data class to hold execution state (`last_run_result`, `variables`, `is_halted`).
- [ ] **`ToolRegistry` Class**: Create a registry to map command types (e.g., "RUN", "WRITE") to their corresponding Python functions.
- [ ] **`ExecutionEngine` Class**: Build the main engine that iterates through the AST and dispatches commands using the tool registry.
- [ ] **Command Dispatch Loop**: Implement the core logic in the `ExecutionEngine` to traverse the AST, including nested blocks like `STEP`.

### 1.2. Command Implementation (Tooling)
- [ ] **`run_command` Tool**: Implement the logic to execute a shell command, capture its `stdout`, `stderr`, and `exit_code`, and store it in the `State` object.
- [ ] **`write_file` Tool**: Implement the logic to write content to a specified file path.
- [ ] **`read_file` Tool**: Implement the logic to read a file's content into a variable in the `State` object.
- [ ] **`assert_condition` Tool**: Implement the logic to evaluate assertion statements against the current state.

### 1.3. Security and Sandboxing
- [ ] **Path Scoping**: Enforce strict path validation to prevent file access outside the project root.
- [ ] **Command Sandboxing**: Integrate a basic sandboxing mechanism for the `run_command` tool to prevent dangerous operations.

## Phase 2: Core Model Enhancements

This phase focuses on completing and improving the core Transformer model based on the project roadmap.

- [ ] **Causal Attention Mask**: Implement and apply the causal attention mask within the `MultiHeadAttention` layer to ensure correct autoregressive behavior during training and generation.
- [ ] **KV Caching**: Implement a KV Cache mechanism to accelerate generative inference by reusing key and value projections from previous tokens.
- [ ] **CLI Interface**: Develop a command-line interface using `argparse` to allow for easy interaction with the model for generation tasks.
- [ ] **Model Checkpointing**: Add functionality to save and load model weights, which is a prerequisite for training.

## Phase 3: Training Pipeline

This phase involves building the components necessary to train the model from scratch, as outlined in the roadmap.

- [ ] **Optimizer Implementation**: Implement an Adam or AdamW optimizer.
- [ ] **Loss Function**: Integrate a cross-entropy loss function.
- [ ] **Training Loop**: Create a complete training script that iterates over a dataset, performs forward and backward passes, and updates model weights.
- [ ] **Gradient Clipping**: Implement gradient clipping to prevent exploding gradients during training.
- [ ] **Learning Rate Scheduler**: Add a learning rate scheduler to adjust the learning rate during training.

## Phase 4: Advanced Architecture & Features

This phase focuses on long-term goals to modernize the model's architecture.

- [ ] **Architectural Modernization**:
    - [ ] **RoPE (Rotary Positional Embeddings)**: Replace sinusoidal positional encodings with RoPE.
    - [ ] **SwiGLU**: Replace the standard ReLU-based FFN with a SwiGLU activation.
    - [ ] **RMSNorm**: Replace `LayerNorm` with `RMSNorm` for better performance.
- [ ] **Advanced Tokenization**: Implement a subword-based tokenizer like BPE (Byte-Pair Encoding) to replace the current simple word-based tokenizer.

## Phase 5: Tooling and Developer Experience

This phase focuses on improving the usability of TissLang.

- [ ] **Syntax Highlighting**: Develop a syntax highlighting extension for a major editor (e.g., VS Code).
- [ ] **Linter/Static Analysis**: Create a linter to check `.tiss` files for syntax errors and potential issues before execution.
- [ ] **Dry Run Mode**: Add a `--dry-run` flag to the execution engine to simulate a run without executing commands.
