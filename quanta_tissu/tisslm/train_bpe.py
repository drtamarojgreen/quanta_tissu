import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
sys.path.insert(0, project_root)

import argparse
import glob # Added import

from .bpe_trainer import BPETokenizer # Corrected import
from .config import system_config

def main():
    """Main function to train and save the BPE tokenizer."""
    parser = argparse.ArgumentParser(
        description="Train a BPE tokenizer on a text corpus."
    )
    parser.add_argument(
        "--corpus_path",
        type=str,
        default=os.path.join(system_config["_project_root"], "corpus"), # Changed default to directory
        help="Path to the training corpus directory (or a single text file)." # Updated help message
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
        default=system_config["bpe_tokenizer_prefix"],
        help="Prefix for the saved tokenizer files (vocab and merges)."
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print progress during training."
    )

    args = parser.parse_args()

    print(f"Loading corpus from: {args.corpus_path}")
    full_text = ""
    if os.path.isdir(args.corpus_path):
        # Read all .txt files from the directory
        txt_files = glob.glob(os.path.join(args.corpus_path, "*.txt"))
        if not txt_files:
            print(f"Error: No .txt files found in the corpus directory: {args.corpus_path}")
            return
        for file_path in txt_files:
            try:
                with open(file_path, "r", encoding="utf-8", errors="replace") as f:
                    full_text += f.read() + "\n" # Add newline to separate content from different files
            except Exception as e:
                print(f"Warning: Could not read file {file_path}: {e}. Skipping.")
    elif os.path.isfile(args.corpus_path):
        # Read a single file
        try:
            with open(args.corpus_path, "r", encoding="utf-8", errors="replace") as f:
                full_text = f.read()
        except FileNotFoundError:
            print(f"Error: Corpus file not found at {args.corpus_path}")
            return
    else:
        print(f"Error: Corpus path is neither a file nor a directory: {args.corpus_path}")
        return

    if not full_text.strip():
        print("Error: Corpus is empty after loading.")
        return
    
    print(f"Corpus loaded. Training tokenizer with vocab size: {args.vocab_size}")
    
    # Initialize and train the tokenizer
    tokenizer = BPETokenizer()
    tokenizer.train(full_text, args.vocab_size, verbose=args.verbose)
    
    print(f"\nTraining complete. Saving tokenizer to files with prefix: {args.save_prefix}")
    
    # Ensure the directory for saving exists
    save_dir = os.path.dirname(args.save_prefix)
    os.makedirs(save_dir, exist_ok=True)
    tokenizer.save(args.save_prefix)
    
    print(f"\n--- Testing Saved and Loaded Tokenizer ---")
    
    # Create a new tokenizer instance and load the saved state
    loaded_tokenizer = BPETokenizer()
    loaded_tokenizer.load(args.save_prefix)
    
    print(f"Loaded tokenizer with vocab size: {len(loaded_tokenizer.vocab)}")
    
    # Test encoding and decoding with the loaded tokenizer
    sample_text = "This is a test of the new tokenizer, demonstrating resilience."
    encoded = loaded_tokenizer.encode(sample_text)
    decoded = loaded_tokenizer.decode(encoded)
    
    print(f"Original: '{sample_text}'")
    print(f"Encoded: {encoded}")
    print(f"Decoded: '{decoded}'")
    
    assert sample_text == decoded
    print("\nTest successful: Decoded text matches original.")
    
    # Verify that the loaded tokenizer is identical to the trained one
    assert tokenizer.vocab == loaded_tokenizer.vocab
    assert tokenizer.merges == loaded_tokenizer.merges
    print("Verification successful: Loaded tokenizer matches the trained one.")

if __name__ == "__main__":
    main()
