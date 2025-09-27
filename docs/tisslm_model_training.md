# Training TissLM Models and Tokenizers

This document provides a detailed guide on how to create your own TissLM tokenizers and model checkpoints using the provided training pipeline. The primary script for this process is `quanta_tissu/tisslm/pipelines/full_training_steps.py`.

## Overview

The training process consists of two main stages:
1.  **Tokenizer Training**: A Byte-Pair Encoding (BPE) tokenizer is trained on a text corpus.
2.  **Model Training**: A simple language model is trained on the tokenized corpus, with checkpoints saved periodically.

## Requirements

- Python 3.x
- PyTorch
- A directory containing your text corpus in `.txt` files.

## Scripts and Parameters

The entire process is orchestrated by the `full_training_steps.py` script. Below is a detailed explanation of the script and the parameters you can configure.

### Running the Training Script

To start the training, you can run the script directly from the root of the repository:

```bash
python3 quanta_tissu/tisslm/pipelines/full_training_steps.py
```

The script will use the default parameters defined within it. You can modify these parameters directly in the script to suit your needs.

### Key Parameters in `full_training_steps.py`

The script's behavior is controlled by several parameters defined in the `if __name__ == "__main__":` block and the `train_model` function definition.

#### Main Configuration Parameters

These are the high-level parameters you will most likely want to change:

-   `CORPUS_PATH`
    -   **Description**: The path to the directory containing your training data. The script will load all files ending in `.txt` from this directory.
    -   **Default**: `./corpus`
    -   **Usage**: Create a directory (e.g., `my_corpus`) and place all your `.txt` files inside it. Then, set `CORPUS_PATH = "./my_corpus"`.

-   `SAVE_DIR`
    -   **Description**: The directory where the trained tokenizer and model checkpoints will be saved. The script will create this directory if it doesn't exist.
    -   **Default**: `./training_output`
    -   **Usage**: All artifacts from a training run will be placed here.

-   `VOCAB_SIZE`
    -   **Description**: The target vocabulary size for the BPE tokenizer. This determines how many unique tokens the tokenizer will learn from the corpus.
    -   **Default**: `5000`
    -   **Usage**: A larger `VOCAB_SIZE` may capture more language detail but can increase model size.

#### Model Training Parameters

These parameters, found in the `train_model` function call, control the specifics of the model training loop:

-   `seq_len`
    -   **Description**: The sequence length, which defines how many tokens are in each input sample for the model.
    -   **Default**: `128`
    -   **Usage**: This is the context window of the model during training.

-   `batch_size`
    -   **Description**: The number of sequences to process in parallel during training.
    -   **Default**: `32`
    -   **Usage**: Adjust this based on your available GPU memory. Larger batch sizes can lead to more stable training.

-   `epochs`
    -   **Description**: The number of times the training loop will iterate over the entire dataset.
    -   **Default**: `5`

-   `lr` (Learning Rate)
    -   **Description**: The learning rate for the Adam optimizer.
    -   **Default**: `1e-3` (0.001)

-   `checkpoint_every`
    -   **Description**: The number of training steps after which a model checkpoint should be saved.
    -   **Default**: `1000`
    -   **Usage**: Checkpoints are saved to the `SAVE_DIR` with the format `checkpoint_step_{step}.pt`. This is crucial for resuming training or using intermediate models.

## Output Files

After running the script, the `SAVE_DIR` will contain the following:

-   **`tokenizer/`**: A directory containing the trained tokenizer files (`vocab.json` and `merges.txt`).
-   **`checkpoint_step_*.pt`**: One or more model state dictionary files, saved at intervals defined by `checkpoint_every`.
-   **`final_model.pt`**: The final state dictionary of the model after all training epochs are complete.

These files are all you need to load and use your custom-trained TissLM model.