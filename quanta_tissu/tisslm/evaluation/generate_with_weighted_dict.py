import os
import sys
import argparse
import json
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
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz") # Assuming the 50k checkpoint
OXFORD_DICT_PATH = os.path.join(project_root, "data", "dict.json")
CORPUS_DICT_PATH = os.path.join(project_root, "data", "corpus_dict.json")

def load_dictionary(file_path):
    """Loads a dictionary from a JSON file."""
    if not os.path.exists(file_path):
        print(f"Warning: Dictionary file not found at {file_path}. Skipping.")
        return set()
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return set(json.load(f))
    except (IOError, json.JSONDecodeError) as e:
        print(f"Error loading dictionary from {file_path}: {e}. Skipping.")
        return set()

def get_weighted_dictionary_bias(tokenizer, oxford_words, corpus_words, oxford_weight, corpus_weight):
    """
    Creates a bias dictionary for generation, prioritizing Oxford words.
    """
    bias_dict = {}
    
    # Add bias for words from the local corpus dictionary
    for word in corpus_words:
        token_ids = tokenizer.tokenize(word)
        for token_id in token_ids:
            bias_dict[token_id] = bias_dict.get(token_id, 0) + corpus_weight

    # Add bias for words from the Oxford dictionary
    # These will overwrite or add to existing biases, giving them higher priority
    for word in oxford_words:
        token_ids = tokenizer.tokenize(word)
        for token_id in token_ids:
            bias_dict[token_id] = bias_dict.get(token_id, 0) + oxford_weight

    return bias_dict

def main():
    parser = argparse.ArgumentParser(description="Generate text with weighted dictionary biasing.")
    parser.add_argument(
        "--prompt",
        type=str,
        default="The meaning of life is",
        help="The initial prompt to start generation."
    )
    parser.add_argument(
        "--length", 
        type=int, 
        default=50, 
        help="The number of new tokens to generate."
    )
    parser.add_argument(
        "--method", 
        type=str, 
        default="nucleus", 
        help="Generation method: greedy, nucleus, etc."
    )
    parser.add_argument(
        "--temperature", 
        type=float, 
        default=0.85, 
        help="Controls randomness for sampling methods."
    )
    parser.add_argument(
        "--top_k", 
        type=int, 
        default=40, 
        help="K for top-k sampling."
    )
    parser.add_argument(
        "--top_p", 
        type=float, 
        default=0.9, 
        help="P for nucleus sampling."
    )
    parser.add_argument(
        "--oxford_weight",
        type=float,
        default=0.8, # Default value
        help="Weight for words from the Oxford dictionary (data/dict.json)."
    )
    parser.add_argument(
        "--corpus_weight",
        type=float,
        default=0.4, # Default value
        help="Weight for words from the local corpus dictionary (data/corpus_dict.json)."
    )
    args = parser.parse_args()

    # --- Load Dictionaries ---
    print(f"--- Loading Oxford dictionary from {OXFORD_DICT_PATH} ---")
    oxford_words = load_dictionary(OXFORD_DICT_PATH)
    print(f"Loaded {len(oxford_words)} Oxford words.")

    print(f"--- Loading Corpus dictionary from {CORPUS_DICT_PATH} ---")
    corpus_words = load_dictionary(CORPUS_DICT_PATH)
    print(f"Loaded {len(corpus_words)} corpus words.")

    # --- Load Tokenizer and Model ---
    print("--- Loading tokenizer and model ---")
    tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # --- Get Weighted Dictionary Bias ---
    print("--- Calculating weighted dictionary bias ---")
    weighted_dict_bias = get_weighted_dictionary_bias(tokenizer, oxford_words, corpus_words, args.oxford_weight, args.corpus_weight)
    print(f"Generated bias for {len(weighted_dict_bias)} unique token IDs.")

    # --- Generate Text with Bias ---
    print(f"\n--- Generating Text with dictionary bias... ---")
    generated_text = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt=args.prompt,
        length=args.length,
        method=args.method,
        temperature=args.temperature,
        top_k=args.top_k,
        top_p=args.top_p,
        sentiment_bias=weighted_dict_bias # Using sentiment_bias parameter for general token biasing
    )
    print(f"Raw Generated Text: '{generated_text}'")

if __name__ == "__main__":
    main()