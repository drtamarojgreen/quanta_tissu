import logging
import numpy as np
import matplotlib.pyplot as plt
from typing import List, Dict, Any

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.optimizer import AdamW
# from quanta_tissu.tisslm.core.data import Dataset

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def plot_training_metrics(losses: List[float], lrs: List[float], grad_norms: List[float], save_path: str = None):
    """
    Plots training loss, learning rate, and gradient norm over steps.

    Args:
        losses: List of loss values per step.
        lrs: List of learning rates per step.
        grad_norms: List of gradient norms per step.
        save_path: Optional path to save the plot. If None, displays the plot.
    """
    steps = range(len(losses))

    fig, axs = plt.subplots(3, 1, figsize=(10, 12))

    axs[0].plot(steps, losses)
    axs[0].set_title('Training Loss over Steps')
    axs[0].set_xlabel('Steps')
    axs[0].set_ylabel('Loss')
    axs[0].grid(True)

    axs[1].plot(steps, lrs)
    axs[1].set_title('Learning Rate over Steps')
    axs[1].set_xlabel('Steps')
    axs[1].set_ylabel('Learning Rate')
    axs[1].grid(True)

    axs[2].plot(steps, grad_norms)
    axs[2].set_title('Gradient Norm over Steps')
    axs[2].set_xlabel('Steps')
    axs[2].set_ylabel('Gradient Norm')
    axs[2].grid(True)

    plt.tight_layout()
    if save_path:
        plt.savefig(save_path)
        logging.info(f"Training metrics plot saved to {save_path}")
    else:
        plt.show()
        logging.info("Training metrics plot displayed.")

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
