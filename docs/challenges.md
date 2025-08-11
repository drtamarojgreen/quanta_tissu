# QuantaTissu Project Challenges

This document outlines the primary challenges in developing the QuantaTissu project and getting it "up and running" as a capable, robust, and safe system. The challenges span the core model's implementation, the agentic framework, the training infrastructure, and essential operational tooling.

---

## 1. Challenge Area: Core Model Implementation (`QuantaTissu`)

The foundational language model is a "from scratch" implementation in NumPy, which presents significant challenges in functionality and performance.

-   **No Training Mechanism**: The model weights are randomly initialized and are not trained on any data. A complete training pipeline, including backpropagation, optimizers, and loss functions, must be built from the ground up.
-   **Minimalist Architecture**: The model consists of only a single transformer block, which severely limits its capacity to learn complex patterns.
-   **Toy Vocabulary**: The tokenizer uses a tiny, fixed vocabulary, making it incapable of handling real-world text. It lacks a subword tokenization strategy like BPE.
-   **Basic Decoding**: The model only supports greedy decoding, which is the simplest and often lowest-quality text generation strategy. It lacks more advanced sampling methods like top-k, nucleus, or temperature scaling.
-   **Lack of Modern Components**: The architecture does not include modern, standard improvements like RoPE, SwiGLU, or RMSNorm, which are common in state-of-the-art models.

## 2. Challenge Area: Agentic Framework (`TissLang`)

The `TissLang` component, designed to direct the agent, involves two major and complex subsystems: the parser and the execution engine.

-   **Parsing Complexity**: As detailed in `docs/tisslang_challenges.md`, building a robust parser for `TissLang` is a significant challenge. Key difficulties include:
    -   **Stateful Parsing**: The parser must track context (e.g., whether it is inside a `STEP` block).
    -   **Heredoc and String Management**: Correctly parsing multi-line strings for the `WRITE` command is error-prone.
    -   **Rich Error Reporting**: Providing meaningful, context-aware error messages is difficult.
    -   **Scalability**: The parser must be designed to accommodate future language features like `IF/ELSE` and variables without requiring a complete rewrite.
-   **Execution Engine Implementation**: The plan calls for an execution engine to interpret the parser's output (the AST), which is a major challenge in itself.
    -   **State Management**: The engine must track the state of the execution, such as the output of the last `RUN` command, for use in `ASSERT` statements.
    -   **Tool Integration**: It needs to be securely and robustly integrated with file system I/O and shell command execution.
    -   **Security**: The `RUN` command is a powerful and dangerous tool. Executing shell commands introduces significant security risks of command injection that must be mitigated, likely via sandboxing.

## 3. Challenge Area: Training and Data Infrastructure

The project currently has **no training or data processing infrastructure**. Building this is a massive undertaking.

-   **Complete Training Loop**: A full training loop needs to be implemented, including a cross-entropy loss function, an Adam/AdamW optimizer, backpropagation through all model layers, and a learning rate scheduler.
-   **Data Tokenization**: The current simple tokenizer needs to be replaced with a subword-based method like Byte-Pair Encoding (BPE) to handle large, real-world vocabularies. This includes building or integrating a trainable tokenizer.
-   **Gradient Verification**: The correctness of the backpropagation implementation must be verified, likely through numerical gradient checking, which is a complex debugging task.

## 4. Challenge Area: Performance and Scalability

The current model is a toy and lacks any of the performance optimizations that are standard for modern language models. Making it practical requires addressing a vast landscape of advanced engineering challenges.

-   **Inference Speed**: The model lacks crucial inference optimizations like **KV Caching**, which dramatically speeds up token generation.
-   **Memory Usage**: There are no memory-saving techniques like **quantization** (e.g., INT8, INT4) or **Flash Attention**.
-   **Scalability**: The project has no support for distributed training (e.g., Data Parallelism, Model Parallelism) or efficient fine-tuning methods (e.g., LoRA, QLoRA), making it impossible to scale to larger models or datasets.

## 5. Challenge Area: Usability and Tooling

The project is missing fundamental tools and structure that are necessary for development and use.

-   **No Testing Framework**: While a test plan exists in `docs/test.md`, no unit tests, integration tests, or BDD tests have been implemented. This makes it impossible to verify the correctness of the complex mathematical components or the behavior of the system.
-   **Lack of a CLI**: There is no command-line interface for interacting with the model, making it difficult to use or experiment with.
-   **No Dependency Management**: A formal `requirements.txt` file is missing, making the environment setup ambiguous.

## 6. Challenge Area: Ethics, Safety, and Governance

The project currently has no mechanisms to address the critical issues of AI safety and ethics. This is a major challenge that must be addressed before the model can be used in any real-world context.

-   **No Content Filtering**: There are no filters to detect or prevent the generation of toxic, harmful, or biased content.
-   **No Bias Detection**: The model has not been evaluated for social biases, and no mitigation techniques have been implemented.
-   **Lack of Transparency**: There are no plans or tools for creating model cards, datasheets, or other transparency artifacts to document the model's behavior and limitations.
-   **No Safety Alignment**: The project lacks any form of alignment tuning (e.g., RLHF, DPO) to make the model more helpful and harmless.
