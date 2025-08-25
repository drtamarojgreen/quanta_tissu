import logging
import numpy as np

from typing import List, Dict, Any

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.optimizer import AdamW
# from quanta_tissu.tisslm.core.data import Dataset





def inspect_model_parameters(model):
    """
    Inspects model parameters (weights and biases) for common issues.

    Args:
        model: The language model.
    """
    logging.info("\n--- Inspecting Model Parameters ---")
    for name, param in model.parameters().items(): # Assuming model.parameters() returns a dict of named params
        logging.info(f"Parameter: {name}")
        logging.info(f"  Shape: {param.shape}")
        logging.info(f"  Mean: {np.mean(param)}")
        logging.info(f"  Std Dev: {np.std(param)}")
        logging.info(f"  Min: {np.min(param)}")
        logging.info(f"  Max: {np.max(param)}")
        logging.info(f"  NaNs: {np.sum(np.isnan(param))}")
        logging.info(f"  Infs: {np.sum(np.isinf(param))}")

        # Check for dead neurons (e.g., all zeros or very small values)
        if np.all(param == 0):
            logging.warning(f"  WARNING: Parameter {name} is all zeros (dead).")
        elif np.std(param) < 1e-6:
            logging.warning(f"  WARNING: Parameter {name} has very small standard deviation (might be dead).")

def check_gradients(model):
    """
    Checks gradients for vanishing or exploding issues.

    Args:
        model: The language model.
    """
    logging.info("\n--- Checking Gradients ---")
    for name, param in model.parameters().items():
        if param.grad is not None:
            logging.info(f"Gradient for {name}:")
            logging.info(f"  Shape: {param.grad.shape}")
            logging.info(f"  Mean: {np.mean(param.grad)}")
            logging.info(f"  Std Dev: {np.std(param.grad)}")
            logging.info(f"  Min: {np.min(param.grad)}")
            logging.info(f"  Max: {np.max(param.grad)}")
            logging.info(f"  NaNs: {np.sum(np.isnan(param.grad))}")
            logging.info(f"  Infs: {np.sum(np.isinf(param.grad))}")

            if np.mean(np.abs(param.grad)) < 1e-8:
                logging.warning(f"  WARNING: Gradient for {name} is very small (vanishing gradient?).")
            elif np.mean(np.abs(param.grad)) > 1e+3:
                logging.warning(f"  WARNING: Gradient for {name} is very large (exploding gradient?).")
        else:
            logging.info(f"Gradient for {name}: None (not computed or zeroed)")

# You can add more functions for specific debugging, e.g.,
# - analyze_activations(model, input_data)
# - visualize_attention_patterns(model, input_data)
