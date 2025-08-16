# LLM Remaining Implementation Items

This document lists the remaining engineering tasks for the QuantaTissu language model and the TissLang execution environment. This list was generated after a review of the existing codebase.

## Phase 1: TissLang Execution Engine

- [ ] **Command Sandboxing**: The current `run_command` tool uses `shell=True`, which is a security risk. A proper sandboxing mechanism needs to be implemented to prevent dangerous operations.

## Phase 2: Core Model Enhancements

- [ ] **KV Caching**: Implement a KV Cache mechanism in the `MultiHeadAttention` layer to accelerate generative inference by reusing key and value projections from previous tokens.

## Phase 3: Training Pipeline

- [ ] **Gradient Clipping**: Implement gradient clipping in the training loop or optimizer to prevent exploding gradients during training.
- [ ] **Learning Rate Scheduler**: Add a learning rate scheduler to adjust the learning rate during training (e.g., cosine decay).

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
