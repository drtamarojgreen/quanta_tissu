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

def generate_text(prompt: str, length: int) -> str:
    """
    Generates text of a specified length, starting with a prompt.
    """
    np.random.seed(42)  # for reproducibility
    model = QuantaTissu(model_config)

    # The model's knowledge base can be populated if desired, but for this
    # script, we will focus on pure generation based on the prompt.
    # model.knowledge_base.add_document("hello world .")

    generated_tokens = []
    current_prompt = prompt

    for _ in range(length):
        # Generate the next token
        next_token_id = model.generate_with_kb(current_prompt, generation_method="greedy")

        if next_token_id is None:
            # Stop if the model fails to generate a token
            break

        generated_tokens.append(next_token_id)

        # The next prompt is the token we just generated
        # This is a simple approach. A more advanced one would use a sliding window of context.
        current_prompt = detokenize([next_token_id])

    return detokenize(generated_tokens)

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
