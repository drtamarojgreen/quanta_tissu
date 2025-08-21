import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
sys.path.insert(0, project_root)

import numpy as np
import argparse

from .model import QuantaTissu
from .tokenizer import Tokenizer
from ..config import model_config

def generate_text(model: QuantaTissu, tokenizer: Tokenizer, prompt: str, length: int, method: str, temperature: float, top_k: int, top_p: float) -> str:
    """
    Generates text of a specified length, starting with a prompt.
    """
    # Tokenize the initial prompt
    prompt_token_ids = tokenizer.tokenize(prompt)
    if not prompt_token_ids.any(): # Check if the array is not empty
        print("Warning: Prompt is empty or contains only unknown tokens.", file=sys.stderr)
        return ""

    # Use the new efficient generate method
    generated_ids = model.generate(
        prompt_token_ids,
        n_new_tokens=length,
        method=method,
        temperature=temperature,
        top_k=top_k,
        top_p=top_p
    )

    # Detokenize the generated IDs and append to the original prompt.
    generated_text = tokenizer.detokenize(np.array(generated_ids))
    return prompt + generated_text

def main():
    """
    Main function to parse arguments and run the text generation.
    """
    parser = argparse.ArgumentParser(description="Generate text using the QuantaTissu model.")
    parser.add_argument(
        "--length",
        type=int,
        default=10,
        help="The number of tokens to generate."
    )
    parser.add_argument(
        "--prompt",
        type=str,
        default="hello",
        help="The initial prompt to start generation."
    )
    parser.add_argument(
        "--checkpoint_path",
        type=str,
        required=True,
        help="Path to the model checkpoint (.npz file)."
    )
    # Arguments for inference tuning
    parser.add_argument("--method", type=str, default="nucleus", help="Generation method: greedy, top_k, nucleus, random.")
    parser.add_argument("--temperature", type=float, default=0.8, help="Controls randomness. Higher is more random.")
    parser.add_argument("--top_k", type=int, default=20, help="K for top-k sampling.")
    parser.add_argument("--top_p", type=float, default=0.9, help="P for nucleus sampling.")

    args = parser.parse_args()

    # --- Initialize Tokenizer ---
    # The Tokenizer class automatically loads the default tokenizer files.
    # Make sure you have run tisslm/train_bpe.py first.
    try:
        tokenizer = Tokenizer()
    except FileNotFoundError as e:
        print(f"Error: Tokenizer files not found. {e}", file=sys.stderr)
        print("Please run `python3 -m quanta_tissu.tisslm.train_bpe` to train and save the tokenizer.", file=sys.stderr) # Updated path
        sys.exit(1)

    # --- Initialize Model ---
    # Update vocab_size in the model config based on the loaded tokenizer
    model_config["vocab_size"] = tokenizer.get_vocab_size()

    np.random.seed(42)  # for reproducibility
    model = QuantaTissu(model_config)

    # --- Load Checkpoint ---
    if not os.path.exists(args.checkpoint_path):
         print(f"Error: Checkpoint file not found at '{args.checkpoint_path}'", file=sys.stderr)
         print("Please run `python3 -m quanta_tissu.tisslm.run_training` to train and save a model checkpoint.", file=sys.stderr)
         sys.exit(1)
    model.load_weights(args.checkpoint_path)


    generated_text = generate_text(
        model,
        tokenizer,
        args.prompt,
        args.length,
        args.method,
        args.temperature,
        args.top_k,
        args.top_p
    )
    print(generated_text)

if __name__ == "__main__":
    main()