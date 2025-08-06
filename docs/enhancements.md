# QuantaTissu Enhancements

This document lists potential enhancements for the QuantaTissu application, categorized by area of improvement. These represent next steps to move the project from a simple educational tool to a more capable and robust system.

## Core Model and Inference

### 1.1. Advanced Sampling Strategies
-   **Status**: Implemented.
-   **Top-k Sampling**: Instead of greedy decoding, select the next token from the `k` most likely candidates. This introduces more variety in the generated text.
-   **Nucleus Sampling (Top-p)**: Select the next token from the smallest set of tokens whose cumulative probability exceeds a threshold `p`. This provides a more dynamic and often higher-quality alternative to top-k sampling.
-   **Temperature Scaling**: Add a `temperature` parameter to the softmax function during sampling. `T > 1` increases randomness (more creative), while `T < 1` decreases it (more deterministic).

### 1.2. Performance and Efficiency
-   **Batched Inference**: Modify the model's forward pass to accept a batch of prompts simultaneously. This significantly improves computational efficiency by leveraging matrix operations on multiple inputs at once.
-   **KV Caching**: During generative inference, cache the Key (K) and Value (V) matrices from the attention layers for previous tokens. This avoids redundant computation and dramatically speeds up the generation of long sequences.

### 1.3. Architectural Improvements
-   **Multiple Transformer Blocks**: Implemented. The model can now stack `N` transformer blocks, configured via `config.py`.
-   **Hyperparameter Configuration**: Implemented. All core model hyperparameters are now centralized in a `model_config` dictionary in `quanta_tissu/quanta_tissu/config.py`.
-   **Pre-trained Weights**: Implement functionality to load pre-trained model weights from a file (e.g., a `.npz` or `.safetensors` file), allowing the model to perform useful tasks without being trained from scratch each time.

## Training and Data

### 2.1. Training Pipeline
-   **Training Loop**: Implement a complete training loop that iterates over a dataset, performs forward and backward passes, and updates model parameters.
-   **Optimizer**: Integrate an optimizer like Adam or AdamW to perform gradient-based weight updates. This will require implementing the backpropagation algorithm.
-   **Loss Function**: Implement the cross-entropy loss function to measure the difference between the model's predictions and the target tokens.
-   **Causal Attention Mask**: Ensure a proper causal (look-ahead) mask is applied during training to prevent the model from "cheating" by looking at future tokens.

### 2.2. Tokenization
-   **Byte-Pair Encoding (BPE)**: Replace the simple word-based tokenizer with a subword tokenizer like BPE. This allows the model to handle a much larger vocabulary, out-of-vocabulary words, and morphological variations gracefully.
-   **Trainable Tokenizer**: Add the capability to train a

-   **Training Loop**: Implement a training loop to train the model on a given dataset.
-   **Optimizer**: Add an optimizer (e.g., Adam) to update the model weights during training.
-   **Loss Function**: Implement a cross-entropy loss function.

## 5. Project Structure

-   **Code Organization**: Split the code into multiple files for better organization and maintainability (e.g., `model.py`, `tokenizer.py`, `config.py`).
-   **Dependency Management**: Add a `requirements.txt` file to list the project dependencies.
-   **Unit Tests**: Add unit tests to ensure the correctness of the individual components.
