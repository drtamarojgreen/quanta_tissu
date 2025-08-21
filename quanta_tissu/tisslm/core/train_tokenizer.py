import argparse
import glob
import logging
import os
from tokenizers import ByteLevelBPETokenizer
from quanta_tissu.tisslm.config import system_config

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def train_bpe_tokenizer(corpus_path: str, vocab_size: int, save_path: str):
    """
    Trains a Byte-Level BPE tokenizer from a text corpus and saves it.
    This version reads files manually to handle potential encoding errors.
    """
    # Find all .txt files in the corpus directory
    files = glob.glob(os.path.join(corpus_path, "*.txt"))
    if not files:
        logging.error(f"No .txt files found in the specified corpus path: {corpus_path}")
        return
    logging.info(f"Found {len(files)} files for training the tokenizer.")

    # Define a generator to read files with error handling to prevent UTF-8 crashes.
    # The `errors="replace"` argument will substitute any invalid characters.
    def file_iterator():
        for file_path in files:
            try:
                with open(file_path, "r", encoding="utf-8", errors="replace") as f:
                    yield f.read()
            except Exception as e:
                logging.warning(f"Could not read file {file_path}: {e}. Skipping.")

    # 1. Initialize a tokenizer
    tokenizer = ByteLevelBPETokenizer()

    # 2. Train the tokenizer
    logging.info(f"Training tokenizer with vocab size {vocab_size}...")
    tokenizer.train_from_iterator(file_iterator(), vocab_size=vocab_size, min_frequency=2, special_tokens=[
        "<s>",
        "<pad>",
        "</s>",
        "<unk>",
        "<mask>",
    ])
    logging.info("Training complete.")

    # 3. Save the tokenizer
    os.makedirs(save_path, exist_ok=True)
    tokenizer_save_path = os.path.join(save_path, "bpe-tokenizer.json")
    tokenizer.save(tokenizer_save_path)
    logging.info(f"Tokenizer saved to {tokenizer_save_path}")

def main():
    parser = argparse.ArgumentParser(description="Train a BPE Tokenizer for QuantaTissu.")
    parser.add_argument("--corpus_path", type=str, default=os.path.join(system_config["_project_root"], "corpus"), help="Path to the training corpus directory.")
    parser.add_argument("--vocab_size", type=int, default=30000, help="The size of the vocabulary to train.")
    parser.add_argument("--save_path", type=str, default=os.path.join(system_config["_project_root"], "tokenizers"), help="Directory to save the trained tokenizer file.")
    args = parser.parse_args()

    train_bpe_tokenizer(args.corpus_path, args.vocab_size, args.save_path)

if __name__ == "__main__":
    main()