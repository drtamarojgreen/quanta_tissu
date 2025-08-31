import os
import sys
import argparse
import numpy as np

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.config import model_config

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_1000.npz")

def main():
    """Main function to generate text using the fine-tuned model."""
    parser = argparse.ArgumentParser(description="Generate text using the final fine-tuned test model.")
    parser.add_argument(
        "--prompt",
        type=str,
        default="The meaning of life is",
        help="The initial prompt to start generation."
    )
    parser.add_argument(
        "--method", 
        type=str, 
        default="nucleus", 
        help="Generation method: greedy, nucleus, dynamic_token_revision, bayesian_word_expansion."
    )
    parser.add_argument(
        "--temperature", 
        type=float, 
        default=0.8, 
        help="Controls randomness. Higher is more random."
    )
    parser.add_argument(
        "--top_k", 
        type=int, 
        default=20, 
        help="K for top-k sampling."
    )
    parser.add_argument(
        "--top_p", 
        type=float, 
        default=0.9, 
        help="P for nucleus sampling."
    )
    parser.add_argument(
        "--length", 
        type=int, 
        default=50, 
        help="The number of new tokens to generate."
    )

    args = parser.parse_args()

    # --- Load Tokenizer and Model ---
    print("--- Loading tokenizer and model ---")
    if not os.path.exists(FINAL_CHECKPOINT_PATH):
        raise FileNotFoundError(f"Final checkpoint not found at {FINAL_CHECKPOINT_PATH}. Please run the training script first.")

    try:
        tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
    except FileNotFoundError:
        raise FileNotFoundError(f"Tokenizer not found at {TOKENIZER_SAVE_PREFIX}. Please run the training script first.")

    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # --- Generate Text ---
    print(f"\n--- Generating Text with method: '{args.method}' ---")
    print(f"Prompt: '{args.prompt}'")

    generated_text = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt=args.prompt,
        length=args.length,
        method=args.method,
        temperature=args.temperature,
        top_k=args.top_k,
        top_p=args.top_p
    )
    
    print("\n--- Generated Text ---")
    print(generated_text)
    print("----------------------")

if __name__ == "__main__":
    main()
