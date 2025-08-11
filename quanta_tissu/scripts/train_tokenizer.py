import os
import argparse
import sys

# Add project root to path to allow importing 'quantatissu'
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quantatissu.tokenizer import BPETokenizer

def main():
    """Main function to train and save the BPE tokenizer."""
    parser = argparse.ArgumentParser(
        description="Train a BPE tokenizer on a text corpus."
    )
    parser.add_argument(
        "--corpus_path",
        type=str,
        default="corpus/resiliency_research.txt",
        help="Path to the training corpus text file."
    )
    parser.add_argument(
        "--vocab_size",
        type=int,
        default=1024,
        help="The desired vocabulary size for the tokenizer."
    )
    parser.add_argument(
        "--save_prefix",
        type=str,
        default="trained_tokenizer",
        help="Prefix for the saved tokenizer files (vocab and merges)."
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print progress during training."
    )

    args = parser.parse_args()

    print(f"Loading corpus from: {args.corpus_path}")
    try:
        with open(args.corpus_path, "r", encoding="utf-8") as f:
            text = f.read()
    except FileNotFoundError:
        print(f"Error: Corpus file not found at {args.corpus_path}")
        return
    
    print(f"Corpus loaded. Training tokenizer with vocab size: {args.vocab_size}")
    
    # Initialize and train the tokenizer
    tokenizer = BPETokenizer()
    tokenizer.train(text, args.vocab_size, verbose=args.verbose)
    
    print(f"\nTraining complete. Saving tokenizer to files with prefix: {args.save_prefix}")
    
    # In a real project, you would save the tokenizer's state here.
    # For this prototype, we'll just demonstrate its use.
    print("\n--- Testing Tokenizer ---")
    sample_text = "This is a test of the new tokenizer, demonstrating resilience."
    encoded = tokenizer.encode(sample_text)
    decoded = tokenizer.decode(encoded)
    
    print(f"Original: '{sample_text}'")
    print(f"Encoded: {encoded}")
    print(f"Decoded: '{decoded}'")
    
    assert sample_text == decoded
    print("\nTest successful: Decoded text matches original.")

if __name__ == "__main__":
    main()