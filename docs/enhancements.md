# QuantaTissu Enhancements

This document lists potential enhancements for the QuantaTissu application, categorized by area of improvement. These represent next steps to move the project from a simple educational tool to a more capable and robust system.

Status Key: `[Not Started]` `[In Progress]` `[Done]`

## 1. Core Model and Inference

### 1.1. Architectural Improvements
-   **[Not Started] Multiple Transformer Blocks**: Generalize the model to allow stacking `N` transformer blocks, creating a deeper and more powerful model. This should be a configurable hyperparameter.
-   **[Not Started] Causal Attention Mask**: Implement and apply a causal (look-ahead) mask in the attention mechanism. This is crucial for correct autoregressive behavior during training and efficient inference.
-   **[Not Started] Dropout Layers**: Add dropout layers after embeddings, attention, and feed-forward layers for regularization to prevent overfitting.
-   **[Not Started] Hyperparameter Configuration**: Centralize all model hyperparameters (e.g., `d_model`, `num_heads`, `d_ff`, `n_layers`, `vocab_size`) into a configuration object or file (e.g., `config.py` or a YAML file).
-   **[Not Started] Modern Feed-Forward Networks (SwiGLU)**: Replace the standard ReLU-based FFN with a more modern Gated Linear Unit variant like SwiGLU, which is used in models like Llama.
-   **[Not Started] Alternative Normalization (RMSNorm)**: Implement RMSNorm as a simpler and often faster alternative to standard Layer Normalization.
-   **[Not Started] Alternative Positional Encodings (RoPE)**: Replace sinusoidal positional encodings with a more modern approach like Rotary Positional Embeddings (RoPE).

### 1.2. Inference and Sampling
-   **[Done] Batched Inference**: The model's forward pass already accepts a batch of prompts simultaneously.
-   **[Not Started] Advanced Sampling Strategies**: Move beyond greedy decoding.
    -   **Temperature Scaling**: Add a `temperature` parameter to the final softmax to control output randomness.
    -   **Top-k Sampling**: Select the next token from the `k` most likely candidates.
    -   **Nucleus Sampling (Top-p)**: Select from the smallest set of tokens whose cumulative probability exceeds a threshold `p`.
-   **[Not Started] KV Caching**: During generative inference, cache the Key (K) and Value (V) matrices from the attention layers for previous tokens. This avoids redundant computation and dramatically speeds up the generation of long sequences.

## 2. Training

### 2.1. Training Pipeline
-   **[Not Started] Loss Function**: Implement the cross-entropy loss function.
-   **[Not Started] Backpropagation**: Implement the backward pass for all layers to compute gradients.
-   **[Not Started] Optimizer**: Integrate an optimizer like Adam or AdamW to perform gradient-based weight updates.
-   **[Not Started] Training Loop**: Implement a complete training loop that iterates over a dataset, performs forward/backward passes, and updates parameters.
-   **[Not Started] Learning Rate Scheduler**: Implement a scheduler (e.g., cosine decay with warmup) to adjust the learning rate during training for better convergence.
-   **[Not Started] Gradient Clipping**: Add gradient clipping to prevent exploding gradients during training.
-   **[Not Started] Model Checkpointing**: Add functionality to save model state (weights, optimizer state) at regular intervals during training.
-   **[Not Started] Pre-trained Weights**: Implement functionality to save and load final model weights from a file (e.g., a `.npz` or `.safetensors` file).

### 2.2. Training Validation
-   **[Not Started] Gradient Checking**: Implement a numerical gradient checking utility to verify the correctness of the backpropagation implementation.
-   **[Not Started] Logging**: Integrate a simple logging mechanism to track metrics like loss and perplexity during training.

## 3. Data and Tokenization

-   **[Not Started] Byte-Pair Encoding (BPE)**: Replace the simple word-based tokenizer with a subword tokenizer like BPE to handle a much larger vocabulary and out-of-vocabulary words gracefully.
-   **[Not Started] Trainable Tokenizer**: Add the capability to train a tokenizer from a text corpus.

## 4. Usability and Tooling

### 4.1. Project Structure
-   **[Not Started] Code Organization**: Refactor the single script into the modular Python package structure suggested in the `README.md`.
-   **[Not Started] Dependency Management**: Create a `requirements.txt` file to formalize project dependencies.

### 4.2. User Interface
-   **[Not Started] Command-Line Interface (CLI)**: Implement a proper CLI using a library like `argparse` to allow users to easily pass prompts, specify model parameters (e.g., temperature, top-k), and control generation length.
-   **[Not Started] Attention Visualization**: Add a utility to generate and save visualizations of the attention maps to understand what the model is focusing on.
-   **[Not Started] Interactive Demo**: Create a simple web-based interactive demo using a library like Gradio or Streamlit.

## 5. Testing

-   **[Not Started] Comprehensive Unit Tests**: Implement the unit tests outlined in `docs/test.md` to cover edge cases and verify mathematical correctness.
-   **[Not Started] Integration Tests**: Add tests that verify the end-to-end workflow, from tokenizing a prompt to detokenizing the final output.
-   **[Not Started] BDD Test Implementation**: Implement the BDD scenarios from `docs/test.md` using a framework like `pytest-bdd`.

## 6. Deployment and Interoperability

-   **[Not Started] Model Serving**: Wrap the model in a simple web server (e.g., using Flask or FastAPI) to expose an inference API endpoint.
-   **[Not Started] ONNX Export**: Add functionality to export the model to the ONNX (Open Neural Network Exchange) format for interoperability and use with optimized runtimes.
