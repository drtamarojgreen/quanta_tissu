# QuantaTissu Project Roadmap

This document outlines the strategic roadmap for the development of QuantaTissu. It provides a high-level overview of the planned phases to evolve the project from an educational tool into a more robust and capable system.

For a granular, checklist-style view of all planned enhancements, please see [`enhancements.md`](./enhancements.md).

---

## Phase 1: Foundational Enhancements & Usability

The primary goal of this phase is to improve the project's structure, usability, and inference capabilities. This involves refactoring the codebase into a proper Python package and introducing more sophisticated generation techniques.

### Key Deliverables:
-   **Code Refactoring**: Reorganize the single-file script into a modular Python package structure.
-   **Centralized Configuration**: Move all model hyperparameters into a dedicated configuration file or class.
-   **Advanced Inference Sampling**: Implement advanced sampling strategies beyond greedy decoding, including Temperature Scaling, Top-k Sampling, and Nucleus (Top-p) Sampling.
-   **Command-Line Interface (CLI)**: Develop a user-friendly CLI using `argparse` to allow for easy interaction with the model.
-   **Causal Attention Masking**: Properly implement and apply the causal attention mask to ensure correct autoregressive generation.
-   **Dependency Management**: Formalize project dependencies in a `requirements.txt` file.

---

## Phase 2: Full Training Implementation

This phase focuses on building the entire training pipeline from scratch, enabling the model to be trained on custom datasets.

### Key Deliverables:
-   **Backpropagation**: Implement the backward pass for all layers to compute gradients.
-   **Loss Function**: Integrate a cross-entropy loss function.
-   **Optimizer**: Implement a gradient-based optimizer, such as Adam or AdamW.
-   **Training Loop**: Create a complete training script.
-   **Model Checkpointing**: Add functionality to save and load model weights.
-   **Training Stability**: Implement techniques like gradient clipping and a learning rate scheduler.
-   **Validation & Logging**: Add gradient checking and logging for training metrics.

---

## Phase 3: Advanced Architecture & Features

This phase aims to modernize the model's architecture with state-of-the-art components and dramatically improve its performance and capabilities.

### Key Deliverables:
-   **Stacked Transformer Blocks**: Generalize the model to support multiple, stacked transformer blocks.
-   **KV Caching**: Implement KV Caching to significantly accelerate generative inference.
-   **Architectural Modernization**:
    -   **RoPE (Rotary Positional Embeddings)**
    -   **SwiGLU**
    -   **RMSNorm**
-   **Advanced Tokenization**: Replace the basic tokenizer with a subword-based method like Byte-Pair Encoding (BPE).
-   **Tooling & Interoperability**:
    -   **API Server**
    -   **Interactive Demo**
    -   **ONNX Export**
