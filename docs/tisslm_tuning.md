# Tuning the TissLM Model

This document provides a comprehensive guide to tuning the TissLM model. Proper tuning is essential for achieving optimal performance, whether you are aiming for higher accuracy, faster training, or more efficient resource utilization.

This guide covers three main areas of tuning:
1.  **Tokenizer Tuning**: Adjusting the vocabulary and tokenization process.
2.  **Model & Training Hyperparameter Tuning**: Configuring the model architecture and the training process.
3.  **Inference-Time Tuning**: Adjusting parameters for text generation.

---

## 1. Tokenizer Tuning

Before training the TissLM model, you need a tokenizer that converts raw text into a sequence of token IDs. TissLM uses a Byte-Pair Encoding (BPE) tokenizer. The quality and size of the tokenizer's vocabulary can significantly impact the model's performance.

The tokenizer is trained using the `quanta_tissu/tisslm/train_bpe.py` script.

### Key Parameters

You can control the tokenizer training process using the following command-line arguments:

*   `--corpus_path`: **(Required)** The path to the raw text file (`.txt`) that will be used to train the tokenizer. A larger, more representative corpus will result in a better tokenizer.
    *   Default: `corpus/resiliency_research.txt`
*   `--vocab_size`: The desired number of tokens in the vocabulary.
    *   **Impact**: A larger vocabulary can represent text more efficiently (fewer tokens per sentence) but increases the model's embedding layer size and computational complexity. A smaller vocabulary is more computationally efficient but may result in more out-of-vocabulary words or longer token sequences.
    *   Default: `1024`
*   `--save_prefix`: The file path prefix for saving the trained tokenizer. The script will save two files: `{save_prefix}.vocab.json` and `{save_prefix}.merges.txt`.
    *   Default: `models/trained_tokenizer`

### How to Train a New Tokenizer

To train a new tokenizer, run the `train_bpe.py` script from the project root directory with your desired parameters.

**Example:**

Let's say you have a new corpus file named `my_corpus.txt` and you want to train a tokenizer with a vocabulary size of 2048.

```bash
python quanta_tissu/tisslm/train_bpe.py \
    --corpus_path /path/to/my_corpus.txt \
    --vocab_size 2048 \
    --save_prefix models/my_new_tokenizer
```

After running this command, you will have a new tokenizer saved as `models/my_new_tokenizer.vocab.json` and `models/my_new_tokenizer.merges.txt`. You will then need to update the model's configuration to use this new tokenizer.

---

## 2. Model and Hyperparameter Tuning

This is the core of the tuning process, where you adjust the model's architecture and the training procedure. These parameters are primarily defined in `quanta_tissu/tisslm/config.py` but can be overridden by command-line arguments in the `run_training.py` script.

### 2.1. Model Architecture (`model_config`)

These parameters define the structure and size of the Transformer model. Changing them has a significant impact on both performance and computational cost.

*   `d_model` (int): The dimensionality of the model's embeddings and hidden states.
    *   **Impact**: This is a primary driver of model size. Larger values increase the model's capacity to learn complex patterns but also increase memory usage and computation time.
    *   Default: `32`
*   `n_layers` (int): The number of Transformer blocks to stack.
    *   **Impact**: More layers give the model more depth to learn hierarchical features. This increases model capacity but also the computational cost and risk of overfitting.
    *   Default: `2`
*   `num_heads` (int): The number of attention heads in the Multi-Head Attention layers.
    *   **Impact**: More heads allow the model to focus on different parts of the input sequence simultaneously. `d_model` must be divisible by `num_heads`.
    *   Default: `4`
*   `d_ff` (int): The dimensionality of the inner layer of the Feed-Forward Networks.
    *   **Impact**: Typically, this is set to `4 * d_model`. Increasing it adds capacity to the model at the cost of more parameters.
    *   Default: `128`

### 2.2. Training Hyperparameters (`training_config`)

These parameters control the training loop and optimization process.

*   `learning_rate` (float): The initial learning rate for the optimizer.
    *   **Impact**: This is one of the most critical hyperparameters. Too high, and the training may diverge; too low, and it may be too slow or get stuck in a local minimum. The `run_training.py` script uses a cosine decay scheduler, so this is the *maximum* learning rate.
    *   Default: `1e-4`
*   `batch_size` (int): The number of sequences processed in a single training step.
    *   **Impact**: Larger batch sizes lead to more stable gradients and can speed up training but require more memory.
    *   Default: `1`
*   `num_epochs` (int): The number of times the entire training dataset is processed.
    *   **Impact**: More epochs can lead to better performance, but also increase the risk of overfitting.
    *   Default: `5`
*   `weight_decay` (float): The weight decay regularization term for the AdamW optimizer.
    *   **Impact**: Helps prevent overfitting by penalizing large weights.
    *   Default: `0.01`

### 2.3. Generation / Inference Parameters (`generation_config`)

These parameters control how text is generated from the trained model. They do not affect training but are crucial for output quality.

*   `default_method` (str): The default sampling strategy. Can be "greedy", "top_k", "nucleus", or "random".
*   `temperature` (float): Controls the randomness of the output.
    *   **Impact**: Higher values (e.g., > 1.0) make the output more random and creative. Lower values (e.g., < 1.0) make it more focused and deterministic.
    *   Default: `1.0`
*   `top_k` (int): In top-k sampling, the model samples from the `k` most likely next tokens.
    *   **Impact**: A smaller `top_k` restricts the model to more probable words, making the output less random.
    *   Default: `10`
*   `top_p` (float): In nucleus sampling, the model samples from the smallest set of tokens whose cumulative probability exceeds `p`.
    *   **Impact**: A lower `top_p` (e.g., 0.9) makes the output more deterministic by cutting off the long tail of low-probability tokens.
    *   Default: `0.9`

### 2.4. Knowledge Base Parameters (`knowledge_base_config`)

These parameters are specific to the Retrieval-Augmented Generation (RAG) capabilities of the system.

*   `retrieval_k` (int): The number of documents to retrieve from the knowledge base to augment the prompt.
    *   Default: `3`
*   `feedback_alpha` (float): The learning rate for updating document relevance scores based on feedback.
    *   Default: `0.1`

---

## 3. The Training and Tuning Workflow

Now that you understand the key parameters, let's walk through the process of launching and customizing a training run.

### Two Methods for Tuning

There are two primary ways to change the hyperparameters for a training run:

1.  **Modifying `config.py` (Persistent)**:
    *   **When to use**: For changes that you want to be the new default.
    *   **How**: Directly edit the values in the `quanta_tissu/tisslm/config.py` file. This is a straightforward way to manage your base configuration.

2.  **Using Command-Line Arguments (Experimental)**:
    *   **When to use**: For experimenting with different values for a specific run without changing your default configuration.
    *   **How**: The `run_training.py` script accepts command-line arguments that override the values in `config.py`. This is the recommended method for systematic tuning and experimentation.

### Running the Training Script

The main script for training the model is `quanta_tissu/tisslm/run_training.py`.

**Key Command-Line Arguments:**

In addition to the hyperparameters in `config.py`, the script has its own set of important arguments:

*   `--corpus_path`: Path to the training corpus directory.
*   `--lr`: Overrides `training_config["learning_rate"]`.
*   `--batch_size`: Overrides `training_config["batch_size"]`.
*   `--epochs`: Overrides `training_config["num_epochs"]`.
*   `--warmup_steps`: The number of steps for the learning rate to ramp up. Important for training stability.
*   `--max_grad_norm`: The maximum value for gradient clipping. Helps prevent exploding gradients.
*   `--checkpoint_dir`: The directory to save model checkpoints.
*   `--resume_from`: Path to a specific checkpoint file to resume training from.
*   `--save_every`: How often (in steps) to save a checkpoint.

### Example: Launching a Custom Training Run

Imagine you want to run an experiment with a higher learning rate and a larger batch size, and you want to save checkpoints every 50 steps.

You would run the following command from the project's root directory:

```bash
python quanta_tissu/tisslm/run_training.py \
    --lr 3e-4 \
    --batch_size 4 \
    --epochs 10 \
    --warmup_steps 100 \
    --max_grad_norm 1.0 \
    --checkpoint_dir ./checkpoints/experiment_01 \
    --save_every 50
```

This command starts a new training run that:
*   Uses a learning rate of `3e-4`.
*   Uses a batch size of `4`.
*   Runs for `10` epochs.
*   Saves checkpoints to a dedicated directory for this experiment.

---

## 4. Practical Guidance and Strategies

### Key Training Components

*   **Optimizer (`AdamW`)**: TissLM uses the AdamW optimizer, which is a common and effective choice for training Transformer models. It is a variant of Adam that decouples weight decay from the gradient update, which can lead to better generalization.
*   **LR Scheduler (`CosineDecayWithWarmup`)**: The learning rate is not fixed during training. It starts low, "warms up" to its maximum value (`lr`), and then smoothly decays in a cosine curve. This warmup phase helps stabilize training at the beginning, and the decay helps the model settle into a good minimum.

### Common Tuning Scenarios

**Scenario 1: "I have limited computing resources (low RAM/VRAM)."**

*   **Goal**: Train the best possible model without running out of memory.
*   **Strategy**:
    1.  **Reduce Model Size**: Decrease `d_model`, `n_layers`, and `d_ff` in `model_config`. This is the most effective way to reduce memory usage.
    2.  **Decrease Batch Size**: Use a smaller `--batch_size` (e.g., `1` or `2`). This will make training slower but will significantly reduce memory consumption per step.
    3.  **Reduce Sequence Length**: If possible, consider a smaller `max_len` in `tokenizer_config`.

**Scenario 2: "Training is too slow."**

*   **Goal**: Speed up the training process.
*   **Strategy**:
    1.  **Increase Batch Size**: If you have available memory, increasing `--batch_size` is the most direct way to process more data in parallel and speed up training.
    2.  **Increase Learning Rate**: A slightly higher `--lr` might help the model converge faster, but be cautious as it can also make training unstable.
    3.  **Use a Smaller Model**: A smaller model (lower `d_model`, `n_layers`) will inherently train faster.

**Scenario 3: "My model is not learning (loss is flat) or is unstable (loss is NaN)."**

*   **Goal**: Stabilize training and ensure the model learns effectively.
*   **Strategy**:
    1.  **Lower the Learning Rate**: This is the most common cause of instability. Try reducing `--lr` by a factor of 10 (e.g., from `1e-4` to `1e-5`).
    2.  **Check Gradient Clipping**: Ensure `--max_grad_norm` is set to a reasonable value (e.g., `1.0`). This prevents gradients from exploding.
    3.  **Use Warmup**: Make sure `--warmup_steps` is set to a reasonable number (e.g., 50-100). A proper warmup is crucial for stability.
    4.  **Simplify the Model**: Try a smaller model to see if it trains successfully. If it does, the issue might be related to the complexity of the larger model.
