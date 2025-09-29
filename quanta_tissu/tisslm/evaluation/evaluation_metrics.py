import numpy as np
import logging

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
# from quanta_tissu.tisslm.core.data import Dataset



def calculate_perplexity(model, dataset, loss_fn):
    """
    Calculates the perplexity of the model on a given dataset.

    Args:
        model: The language model (e.g., QuantaTissu instance).
        dataset: The dataset to evaluate on (e.g., Dataset instance).
        loss_fn: The loss function used for training (e.g., CrossEntropyLoss instance).

    Returns:
        The perplexity score.
    """
    total_loss = 0.0
    num_batches = 0

    # Set model to evaluation mode if applicable (e.g., for dropout/batch norm)
    # if hasattr(model, 'eval'):
    #     model.eval()

    for x_batch, y_batch in dataset:
        logits, _ = model.forward(x_batch)
        loss = loss_fn.forward(logits, y_batch)
        total_loss += loss
        num_batches += 1

    avg_loss = total_loss / num_batches
    perplexity = np.exp(avg_loss)

    # Set model back to training mode if applicable
    # if hasattr(model, 'train'):
    #     model.train()

    logging.info(f"Calculated Perplexity: {perplexity:.4f}")
    return perplexity

# You can add more evaluation metrics here, e.g., BLEU, ROUGE, etc.
# def calculate_bleu(...):
#    pass

def main():
    """
    Main function to demonstrate the usage of evaluation metrics.
    When run as a script, this will load a model and calculate its perplexity
    on a small dummy dataset.
    """
    import os
    import sys

    # --- Setup Project Root ---
    # This allows the script to find the 'quanta_tissu' module
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
    if project_root not in sys.path:
        sys.path.insert(0, project_root)

    # --- Imports ---
    # Import necessary components from the project
    from quanta_tissu.tisslm.core.model import QuantaTissu
    from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
    from quanta_tissu.tisslm.core.tokenizer import Tokenizer
    from quanta_tissu.tisslm.config import model_config

    # --- Configuration ---
    # Define paths to the test model and tokenizer
    TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
    TEST_MODEL_DIR = os.path.join(project_root, "test_model")
    TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
    FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")

    # --- Logging ---
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

    print("="*80)
    print("Running Evaluation Metrics Demonstration")
    print("="*80)

    # --- Load Model and Tokenizer ---
    try:
        print("Loading tokenizer...")
        tokenizer = Tokenizer(tokenizer_prefix=TOKENIZER_SAVE_PREFIX)
        model_config["vocab_size"] = tokenizer.get_vocab_size()

        print("Loading model...")
        model = QuantaTissu(model_config)
        model.load_weights(FINAL_CHECKPOINT_PATH)
        print("Model and tokenizer loaded successfully.")
    except Exception as e:
        print(f"FATAL: Could not load model or tokenizer. {e}")
        print("Please ensure that a test model and tokenizer exist.")
        print(f"Expected tokenizer at: {TOKENIZER_SAVE_PREFIX}.json")
        print(f"Expected model at: {FINAL_CHECKPOINT_PATH}")
        sys.exit(1)

    # --- Create Dummy Data ---
    # Create a simple generator to yield dummy batches of data
    def dummy_dataset_generator(batch_size=4, seq_len=16, num_batches=5, vocab_size=tokenizer.get_vocab_size()):
        print(f"\nCreating a dummy dataset with {num_batches} batches...")
        for _ in range(num_batches):
            # Generate random token IDs
            x = np.random.randint(0, vocab_size, size=(batch_size, seq_len))
            # The target 'y' is typically the input 'x' shifted by one position
            y = np.roll(x, -1, axis=-1)
            yield x, y

    # --- Initialize Loss Function ---
    loss_fn = CrossEntropyLoss()

    # --- Run Perplexity Calculation ---
    print("\nCalculating perplexity on the dummy dataset...")
    # Get the generator
    dummy_dataset = dummy_dataset_generator()
    perplexity = calculate_perplexity(model, dummy_dataset, loss_fn)

    print("\n" + "-"*40)
    print(f"Demonstration Complete.")
    print(f"Final Calculated Perplexity: {perplexity:.4f}")
    print("-"*40)


if __name__ == "__main__":
    main()