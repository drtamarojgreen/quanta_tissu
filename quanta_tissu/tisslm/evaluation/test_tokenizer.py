import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer

def main():
    # Hardcoded parameters as requested
    # checkpoint_path = "checkpoints/checkpoint_step_160000.npz" # Not used in this tokenizer test
    tokenizer_relative_prefix = "models/tokenizers/revised_tokenizer"

    tokenizer_full_prefix = os.path.join(project_root, tokenizer_relative_prefix)

    try:
        tokenizer = Tokenizer(tokenizer_prefix=tokenizer_full_prefix)

        test_phrase = "hello world"
        token_ids = tokenizer.tokenize(test_phrase)
        detokenized_text = tokenizer.detokenize(token_ids)

        print(f"Original phrase: {test_phrase}")
        print(f"Token IDs: {token_ids.tolist()}")
        print(f"Detokenized text: {detokenized_text}")

    except FileNotFoundError as e:
        print(f"Error: Tokenizer files not found at {tokenizer_full_prefix}. {e}", file=sys.stderr)
        print("Please ensure the tokenizer is trained and available at the specified path.", file=sys.stderr)
    except Exception as e:
        print(f"An unexpected error occurred: {e}", file=sys.stderr)

if __name__ == "__main__":
    main()