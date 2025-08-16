import sys
import os
import numpy as np
import argparse

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from quanta_tissu.quanta_tissu.model import QuantaTissu
from quanta_tissu.quanta_tissu.tokenizer import detokenize
from quanta_tissu.quanta_tissu.config import model_config, vocab
from quanta_tissu.quanta_tissu.model import QuantaTissu

from quanta_tissu.quanta_tissu.tokenizer import Tokenizer

def generate_text(prompt: str, length: int) -> str:
    """
    Generates text of a specified length, starting with a prompt.
    """
    np.random.seed(42)  # for reproducibility
    # NOTE: The tokenizer initialization is simplified here.
    # In a real application, it should be loaded from a file.
    tokenizer = Tokenizer(vocab)
    model = QuantaTissu(model_config)

    # Tokenize the initial prompt
    prompt_token_ids = tokenizer.tokenize(prompt)
    if not prompt_token_ids:
        print("Warning: Prompt is empty or contains only unknown tokens.", file=sys.stderr)
        return ""

    # Use the new efficient generate method
    generated_ids = model.generate(prompt_token_ids, n_new_tokens=length, method="greedy")

    # The full sequence is the prompt + generated tokens
    full_token_ids = prompt_token_ids + generated_ids

    # Detokenize the entire sequence of tokens
    return tokenizer.detokenize(full_token_ids)

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
    args = parser.parse_args()

    # Ensure the prompt is in the vocabulary
    if args.prompt not in vocab:
        print(f"Error: Prompt '{args.prompt}' not in vocabulary. Please use one of: {list(vocab.keys())}", file=sys.stderr)
        sys.exit(1)

    generated_text = generate_text(args.prompt, args.length)
    print(generated_text)

if __name__ == "__main__":
    main()
