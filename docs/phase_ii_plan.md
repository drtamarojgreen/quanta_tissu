# Phase II Development Plan: Full Training Implementation

This document provides a detailed plan for the tasks required to complete **Phase 2** of the [QuantaTissu Project Roadmap](./roadmap.md). The goal of this phase is to build a complete, end-to-end training pipeline for the model.

## 1. Overview

The "Full Training Implementation" phase involves implementing the core components required for training a neural network: backpropagation, a loss function, an optimizer, and a training loop. This phase will transform QuantaTissu from an inference-only engine into a model that can be trained on new data.

## 2. Key Components & Implementation Steps

### 2.1. Backpropagation
-   **Objective**: Implement the backward pass for all existing layers to compute gradients.
-   **Tasks**:
    -   Derive the gradients for the `FeedForward` layer.
    -   Derive the gradients for the `MultiHeadAttention` layer, including the scaled dot-product attention.
    -   Derive the gradients for the `LayerNorm` layer.
    -   Implement a `backward()` method in each layer class.
    -   Ensure the chain rule is correctly applied through the `TransformerBlock`.

### 2.2. Loss Function
-   **Objective**: Implement the cross-entropy loss function.
-   **Tasks**:
    -   Create a `CrossEntropyLoss` class or function.
    -   The function should take the model's final logits and the target token IDs as input.
    -   It should compute the negative log-likelihood loss.
    -   Implement the backward pass for the loss function to compute the initial gradient.

### 2.3. Optimizer
-   **Objective**: Implement a standard gradient-based optimizer.
-   **Tasks**:
    -   Create an `AdamW` optimizer class.
    -   The optimizer should take the model's parameters and a learning rate as input.
    -   Implement the `step()` method to update the model's weights based on the computed gradients.
    -   Include logic for handling weight decay as per the AdamW algorithm.

### 2.4. Training Loop
-   **Objective**: Create a script to manage the entire training process.
-   **Tasks**:
    -   Create a new script, e.g., `scripts/run_training.py`.
    -   The script should handle:
        -   Loading and preparing a dataset.
        -   Iterating over the data in batches.
        -   For each batch:
            -   Performing a forward pass.
            -   Calculating the loss.
            -   Performing a backward pass to get gradients.
            -   Updating weights using the optimizer.
            -   Zeroing out gradients for the next iteration.

### 2.5. Training Stability & Validation
-   **Objective**: Add features to ensure stable training and to monitor progress.
-   **Tasks**:
    -   **Gradient Clipping**: Implement logic to clip gradients by norm to prevent exploding gradients.
    -   **Learning Rate Scheduler**: Implement a simple learning rate scheduler, such as cosine decay with warmup.
    -   **Logging**: Add logging to track key metrics like training loss and perplexity.
    -   **Gradient Checking**: Implement a numerical gradient checking utility to verify the correctness of the backpropagation implementation.

### 2.6. Model Persistence
-   **Objective**: Implement the ability to save and load model weights.
-   **Tasks**:
    -   Implement `save_weights()` and `load_weights()` methods in the `QuantaTissu` class.
    -   Use a format like NumPy's `.npz` or `.safetensors` for saving the weights.
    -   Integrate this into the training loop to save checkpoints periodically.

## 3. Definition of Done

Phase II will be considered complete when it is possible to run the `run_training.py` script on a sample dataset, observe the loss decreasing over time, and save a trained model checkpoint to disk.
