import subprocess
import os
import sys
import tempfile
from pathlib import Path

# --- Path Setup ---
# Ensures that the script can find the necessary modules and scripts
# by adding the project's root directory to the Python path.
try:
    script_path = Path(__file__).resolve()
    # This assumes the script is in quanta_tissu/tisslm/pipelines
    project_root = script_path.parent.parent.parent.parent
except NameError:
    # Fallback for interactive environments or when __file__ is not defined
    project_root = Path(os.getcwd())

sys.path.insert(0, str(project_root))

# Define key paths
PIPELINES_DIR = project_root / "quanta_tissu" / "tisslm" / "pipelines"
CORE_DIR = project_root / "quanta_tissu" / "tisslm" / "core"
# Based on `run_custom_training.py`, the corpus is expected at the project root.
CORPUS_PATH = project_root / "corpus"

# Scripts to be called
TRAIN_BPE_SCRIPT = CORE_DIR / "train_bpe.py"
RUN_TRAINING_SCRIPT = PIPELINES_DIR / "run_custom_training.py"

# --- Configurations to Test ---
# A list of dictionaries, where each dictionary defines a set of
# hyperparameters for a single training run.
# NOTE: Using small epochs and batch sizes for demonstration purposes.
CONFIGURATIONS = [
    {
        "lr": 1e-3,
        "batch_size": 8,
        "epochs": 1,
        "seq_len": 64,
        "comment": "High learning rate, small batch",
    },
    {
        "lr": 1e-4,
        "batch_size": 16,
        "epochs": 1,
        "seq_len": 128,
        "comment": "Lower learning rate, medium batch",
    },
    {
        "lr": 1e-5,
        "batch_size": 8,
        "epochs": 2,
        "seq_len": 128,
        "comment": "Very low learning rate, 2 epochs",
    },
    # --- Additional Configurations ---
    {
        "lr": 5e-4,
        "batch_size": 16,
        "epochs": 2,
        "seq_len": 128,
        "weight_decay": 0.01,
        "comment": "Standard LR with weight decay",
    },
    {
        "lr": 1e-4,
        "batch_size": 32,
        "epochs": 1,
        "seq_len": 256,
        "comment": "Large batch size and sequence length",
    },
    {
        "lr": 8e-5,
        "batch_size": 8,
        "epochs": 3,
        "seq_len": 128,
        "warmup_steps": 100,
        "comment": "Longer warmup, 3 epochs",
    },
    {
        "lr": 2e-5,
        "batch_size": 16,
        "epochs": 2,
        "seq_len": 256,
        "weight_decay": 0.05,
        "comment": "Low LR, high seq len, with weight decay",
    },
    {
        "lr": 1e-4,
        "batch_size": 8,
        "epochs": 1,
        "seq_len": 64,
        "max_grad_norm": 0.5,
        "comment": "Tighter gradient clipping",
    },
]

def train_tokenizer(tokenizer_prefix: Path):
    """
    Trains a BPE tokenizer using the train_bpe.py script.
    """
    print("--- [Setup] Training BPE Tokenizer ---")
    if not TRAIN_BPE_SCRIPT.exists():
        print(f"Error: Tokenizer training script not found at {TRAIN_BPE_SCRIPT}")
        sys.exit(1)
    if not CORPUS_PATH.exists():
        print(f"Error: Corpus directory not found at {CORPUS_PATH}. Please ensure it exists.")
        sys.exit(1)

    command = [
        sys.executable,
        str(TRAIN_BPE_SCRIPT),
        "--corpus_path", str(CORPUS_PATH),
        "--save_prefix", str(tokenizer_prefix),
        "--vocab_size", "1024",
    ]
    print(f"Executing: {' '.join(command)}")
    result = subprocess.run(command, capture_output=True, text=True, check=False)

    if result.returncode != 0:
        print("Error during tokenizer training:")
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)

    print("Tokenizer training successful.")
    print(result.stdout)


def run_training_config(config: dict, tokenizer_prefix: Path, run_number: int):
    """
    Runs a single training configuration using the run_custom_training.py script.
    """
    print(f"\n--- [Run {run_number}/{len(CONFIGURATIONS)}] Testing Config: {config.get('comment', '')} ---")
    if not RUN_TRAINING_SCRIPT.exists():
        print(f"Error: Training script not found at {RUN_TRAINING_SCRIPT}")
        return

    # Use a dedicated checkpoint directory for each run
    checkpoint_dir = project_root / "checkpoints" / f"run_{run_number}_{config['lr']}_{config['batch_size']}"
    os.makedirs(checkpoint_dir, exist_ok=True)

    command = [
        sys.executable,
        str(RUN_TRAINING_SCRIPT),
        "--tokenizer_path", str(tokenizer_prefix),
        "--corpus_path", str(CORPUS_PATH),
        "--epochs", str(config["epochs"]),
        "--batch_size", str(config["batch_size"]),
        "--seq_len", str(config["seq_len"]),
        "--lr", str(config["lr"]),
        "--checkpoint_dir", str(checkpoint_dir),
        "--save_every", "50", # Save more frequently for short test runs
    ]

    # Add optional parameters from the config dictionary
    if "weight_decay" in config:
        command.extend(["--weight_decay", str(config["weight_decay"])])
    if "warmup_steps" in config:
        command.extend(["--warmup_steps", str(config["warmup_steps"])])
    if "max_grad_norm" in config:
        command.extend(["--max_grad_norm", str(config["max_grad_norm"])])

    print(f"Executing: {' '.join(command)}")
    result = subprocess.run(command, capture_output=True, text=True, check=False)

    if result.returncode != 0:
        print(f"--- [Run {run_number}] FAILED ---")
        print("Error during model training:")
        print(result.stdout)
        print(result.stderr)
    else:
        print(f"--- [Run {run_number}] SUCCESS ---")
        print(result.stdout)


def main():
    """
    Main function to orchestrate tokenizer training and model configuration testing.
    """
    # Create a temporary directory for tokenizer assets
    with tempfile.TemporaryDirectory() as temp_dir:
        tokenizer_prefix = Path(temp_dir) / "test_tokenizer"

        # 1. Train the tokenizer
        train_tokenizer(tokenizer_prefix)

        # 2. Run training for each configuration
        for i, config in enumerate(CONFIGURATIONS):
            run_training_config(config, tokenizer_prefix, i + 1)

    print("\n--- All configurations tested. ---")


if __name__ == "__main__":
    main()
