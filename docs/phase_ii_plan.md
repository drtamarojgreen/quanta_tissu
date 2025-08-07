# Phase II Development Plan: Full Training Implementation

This document provides a detailed plan for the tasks required to complete **Phase 2** of the [QuantaTissu Project Roadmap](./roadmap.md). The goal of this phase is to build a complete, end-to-end training pipeline for the model.

## 1. Overview

The "Full Training Implementation" phase involves implementing the core components required for training a neural network: backpropagation, a loss function, an optimizer, and a training loop. This phase will transform QuantaTissu from an inference-only engine into a model that can be trained on new data.

## 2. Key Components & Implementation Steps

### 2.1. Parameter Management & Gradient Computation (Backpropagation)
-   **Objective**: Enable gradient computation for all model parameters by implementing the backward pass for all layers.
-   **Tasks**:
    -   **Parameter Collection**: Modify `QuantaTissu` and its sub-modules to have a `parameters()` method that recursively collects all trainable weights and their corresponding gradients.
    -   **Gradient Derivation**: Mathematically derive the gradients for the backward pass of each layer (`FeedForward`, `MultiHeadAttention`, `LayerNorm`).
    -   **`backward()` Implementation**: Implement a `backward(d_out)` method in each layer class. This method will compute the gradient with respect to the layer's inputs (`d_in`) and store the gradients for its own parameters (e.g., `self.W1.grad`).
    -   **Chain Rule Orchestration**: Ensure the `TransformerBlock`'s `backward` method correctly applies the chain rule, passing gradients sequentially through its sub-layers.

### 2.2. Loss Function
-   **Objective**: Implement a standard loss function for language model training.
-   **Tasks**:
    -   **`CrossEntropyLoss` Class**: Create a class with `forward` and `backward` methods.
    -   **Forward Pass**: The forward pass will compute the average negative log-likelihood loss over a batch.
    -   **Backward Pass**: The backward pass will compute the initial gradient of the loss with respect to the model's logits (`d_logits`), which is the starting point for backpropagation.

### 2.3. Optimizer
-   **Objective**: Implement a modern, effective optimizer to update model weights.
-   **Tasks**:
    -   **`AdamW` Class**: Create an optimizer class whose constructor accepts a list of model parameters and hyperparameters (`lr`, `betas`, `eps`, `weight_decay`).
    -   **`step()` Method**: Implement the core AdamW update rule, including bias correction and weight decay logic.
    -   **`zero_grad()` Method**: Implement a method to reset the `.grad` attribute of all parameters to zero before each new backpropagation pass.

### 2.4. Data Handling
-   **Objective**: Prepare a dataset for the training loop.
-   **Tasks**:
    -   **Dataset Class**: Create a simple `Dataset` class capable of loading a large text file.
    -   **Batching Logic**: Implement logic to create batches of input sequences (`x`) and target sequences (`y`, which are `x` shifted by one token).

### 2.5. Training Orchestration
-   **Objective**: Create a master script to manage the entire training process.
-   **Tasks**:
    -   **`scripts/run_training.py`**: Create the main training script.
        -   Use `argparse` to accept command-line arguments (e.g., dataset path, learning rate, batch size, number of epochs).
        -   Instantiate the model, tokenizer, loss function, and optimizer.
        -   Implement the main training loop that iterates over epochs and data batches.
        -   Inside the loop, orchestrate the full training cycle: `forward -> loss -> backward -> optimizer.step() -> optimizer.zero_grad()`.

### 2.6. Training Stability and Monitoring
-   **Objective**: Add essential features for robust training and progress tracking.
-   **Tasks**:
    -   **Gradient Clipping**: In the training loop, after the backward pass but before the optimizer step, add logic to clip gradients by their global norm to prevent exploding gradients.
    -   **Learning Rate Scheduler**: Implement a `CosineDecayWithWarmup` scheduler class that can be called after each optimizer step to adjust the learning rate.
    -   **Logging**: Use Python's `logging` module to print key metrics (loss, perplexity, learning rate) to the console at regular intervals.
    -   **Gradient Checking**: Implement a numerical gradient checking utility in a separate script (`scripts/check_gradients.py`) to verify the correctness of the backpropagation implementation against a known baseline.

### 2.7. Model Persistence
-   **Objective**: Implement the ability to save and load training state to allow for checkpointing and resumption.
-   **Tasks**:
    -   **`save_checkpoint()` Utility**: Create a function that saves the model's weights, optimizer state, and current epoch/step to a single file.
    -   **`load_checkpoint()` Utility**: Create a corresponding function to load this state back into the model and optimizer to resume training.
    -   Integrate checkpointing into the training loop to save progress periodically (e.g., every N steps or at the end of each epoch).

## 3. Definition of Done

Phase II will be considered complete when:

-   The `run_training.py` script can be executed with command-line arguments to start a training run.
-   The training loss demonstrably decreases over at least 100 steps on a sample dataset.
-   A model checkpoint (e.g., a `.npz` file) containing the model weights and optimizer state is successfully saved to disk.
-   A training run can be successfully resumed from a saved checkpoint.
