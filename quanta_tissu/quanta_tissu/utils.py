import numpy as np
import os
import logging

def save_checkpoint(model, optimizer, epoch, step, checkpoint_dir):
    """Saves model and optimizer state to a checkpoint file."""
    if not os.path.exists(checkpoint_dir):
        os.makedirs(checkpoint_dir)

    filename = os.path.join(checkpoint_dir, f"checkpoint_step_{step}.npz")

    # Get model state
    model_params = model.parameters()
    model_state = {f"param_{i}": p.value for i, p in enumerate(model_params)}

    # Get optimizer state, saving each array individually
    optimizer_m_state = {f"optimizer_m_{i}": arr for i, arr in enumerate(optimizer.m)}
    optimizer_v_state = {f"optimizer_v_{i}": arr for i, arr in enumerate(optimizer.v)}

    # Combine all state
    full_state = {
        **model_state,
        **optimizer_m_state,
        **optimizer_v_state,
        "num_params": np.array(len(model_params)),
        "optimizer_t": np.array(optimizer.t),
        "epoch": np.array(epoch),
        "step": np.array(step),
    }

    np.savez(filename, **full_state)
    logging.info(f"Saved checkpoint to {filename}")

def load_checkpoint(model, optimizer, file_path):
    """Loads model and optimizer state from a checkpoint file."""
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Checkpoint file not found: {file_path}")

    data = np.load(file_path, allow_pickle=True)

    num_params = int(data["num_params"])

    # Load model state
    model_params = model.parameters()
    for i in range(num_params):
        model_params[i].value = data[f"param_{i}"]

    # Load optimizer state
    optimizer.m = [data[f"optimizer_m_{i}"] for i in range(num_params)]
    optimizer.v = [data[f"optimizer_v_{i}"] for i in range(num_params)]
    optimizer.t = int(data["optimizer_t"])

    # Load training progress
    epoch = int(data["epoch"])
    step = int(data["step"])

    logging.info(f"Loaded checkpoint from {file_path}. Resuming from epoch {epoch}, step {step}.")
    return epoch, step
