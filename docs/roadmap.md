# QuantaTissu Project Roadmap

This document outlines the strategic roadmap for the QuantaTissu project, detailing our progress and future direction.

For a granular, checklist-style view of all planned enhancements, please see [`enhancements.md`](./enhancements.md).

---

## Completed Milestones

We have successfully completed the foundational phases of the project, establishing a robust and functional baseline for the QuantaTissu language model and its ecosystem.

### 1. Foundational Model and Usability
-   **Core Architecture**: A configurable, N-layer transformer model was built from scratch.
-   **Code Structure**: The codebase was refactored into a modular Python package with centralized configuration.
-   **Inference Capabilities**: Advanced sampling methods (temperature, top-k, nucleus) and a user-friendly CLI have been implemented.
-   **Performance**: **KV Caching** is implemented, dramatically speeding up generative inference.

### 2. Full Training Pipeline
-   **End-to-End Training**: A complete training pipeline has been implemented from scratch, including backpropagation, an Adam optimizer, cross-entropy loss, and gradient clipping.
-   **Training Management**: The pipeline includes model checkpointing, a learning rate scheduler, and logging for key metrics.

### 3. Advanced Tokenization
-   **Byte-Pair Encoding (BPE)**: The original simple tokenizer was replaced with a trainable BPE tokenizer, enabling the model to handle a much larger vocabulary and unknown words gracefully.

---

## Next Steps: Future Enhancements

Our future work is focused on pushing the boundaries of the model's performance, efficiency, and capabilities. The next phases are organized into several key development tracks.

### 1. Architectural Modernization
The next priority is to upgrade the model's core components with modern, state-of-the-art alternatives that are common in leading open-source models.
-   **[Not Started] SwiGLU**: Replace the standard ReLU-based FFN with a SwiGLU variant.
-   **[Not Started] RMSNorm**: Implement RMSNorm as a faster alternative to LayerNorm.
-   **[Not Started] RoPE (Rotary Positional Embeddings)**: Replace sinusoidal positional encodings with RoPE.

### 2. Efficiency and Performance Optimization
To make the model faster and more resource-friendly, we will explore a range of optimization techniques.
-   **[Not Started] Quantization**: Implement post-training and quantization-aware training (INT8/INT4).
-   **[Not Started] Flash Attention**: Integrate a highly optimized attention algorithm.
-   **[Not Started] Distributed Training**: Implement strategies like Data Parallelism (Multi-GPU) and ZeRO.

### 3. Fine-Tuning and Alignment
To adapt the model for specific tasks and ensure its outputs are helpful and harmless, we will implement modern alignment techniques.
-   **[Not Started] PEFT**: Implement parameter-efficient fine-tuning methods like **LoRA** and **QLoRA**.
-   **[Not Started] Alignment**: Implement **Reinforcement Learning from Human Feedback (RLHF)** and/or **Direct Preference Optimization (DPO)**.

### 4. Usability and Tooling
To improve the developer and user experience, we will build better tools for interaction and analysis.
-   **[Not Started] API Server**: Expose the model via a RESTful API (using Flask or FastAPI).
-   **[Not Started] Interactive Demo**: Create a web-based demo using Gradio or Streamlit.
-   **[Not Started] Attention Visualization**: Develop tools to visualize attention maps.

### 5. Responsible AI and Governance
We will continue to build on our commitment to ethical AI by implementing features that promote safety, transparency, and fairness.
-   **[Not Started] Bias and Toxicity Mitigation**: Implement content filters and bias evaluation benchmarks.
-   **[Not Started] Transparency**: Automate the generation of model cards and datasheets.
-   **[Not Started] Explainability**: Integrate tools like SHAP or LIME to explain model predictions.
