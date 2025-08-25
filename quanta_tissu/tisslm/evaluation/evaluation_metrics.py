import numpy as np
import logging

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
# from quanta_tissu.tisslm.core.data import Dataset

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

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
        logits = model.forward(x_batch)
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
