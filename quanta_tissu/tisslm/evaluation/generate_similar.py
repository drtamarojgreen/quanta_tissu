import os
import sys
import argparse
import json
import numpy as np
import re # Import regex for tokenization

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
DICT_PATH = os.path.join(project_root, "data", "dict.json")

def levenshtein_distance(s1, s2):
    """Calculates the Levenshtein distance between two strings from scratch."""
    if len(s1) < len(s2):
        return levenshtein_distance(s2, s1)

    if len(s2) == 0:
        return len(s1)

    previous_row = range(len(s2) + 1)
    for i, c1 in enumerate(s1):
        current_row = [i + 1]
        for j, c2 in enumerate(s2):
            insertions = previous_row[j + 1] + 1
            deletions = current_row[j] + 1
            substitutions = previous_row[j] + (c1 != c2)
            current_row.append(min(insertions, deletions, substitutions))
        previous_row = current_row
    
    return previous_row[-1]

def analyze_similarity(generated_text, dictionary):
    """
    For each word in the generated text, finds the most similar word
    in the provided dictionary based on Levenshtein distance.
    Returns a list of dictionaries, preserving the order of unique words.
    """
    if not dictionary:
        print("Warning: Dictionary is empty. Cannot perform similarity analysis.")
        return []

    # Tokenize the generated text while preserving order and handling duplicates
    ordered_unique_words = []
    seen_words = set()
    
    # Use a simple regex to find words, preserving their order
    # re.findall will return a list of all non-overlapping matches
    all_generated_words = re.findall(r'\b\w+\b', generated_text.lower())
    
    for word in all_generated_words:
        if word and word not in seen_words:
            ordered_unique_words.append(word)
            seen_words.add(word)
    
    analysis_results = [] # This will be a list of dicts, preserving order

    for word_lc in ordered_unique_words:
        min_distance = float('inf')
        closest_word = ""

        for dict_word in dictionary:
            distance = levenshtein_distance(word_lc, dict_word)
            if distance < min_distance:
                min_distance = distance
                closest_word = dict_word
            if distance == 0: # Perfect match, no need to search further
                break
        
        analysis_results.append({
            "generated_word": word_lc,
            "closest_word": closest_word,
            "distance": min_distance
        })
        
    return analysis_results

def main():
    """Main function to generate text and analyze word similarity."""
    parser = argparse.ArgumentParser(description="Generate text and find similar words from a dictionary.")
    parser.add_argument(
        "--prompt",
        type=str,
        default="The meaning of life is",
        help="The initial prompt to start generation."
    )
    parser.add_argument(
        "--length", 
        type=int, 
        default=20, 
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
    args = parser.parse_args()

    # --- Load Dictionary ---
    print(f"--- Loading dictionary from {DICT_PATH} ---")
    try:
        with open(DICT_PATH, 'r', encoding='utf-8') as f:
            dictionary = set(json.load(f))
        print(f"Successfully loaded {len(dictionary)} words.")
    except (IOError, json.JSONDecodeError) as e:
        raise FileNotFoundError(f"Could not load dictionary from {DICT_PATH}: {e}")

    # --- Load Tokenizer and Model ---
    print("--- Loading tokenizer and model ---")
    tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # --- Generate Text ---
    print(f"\n--- Generating Text... ---")
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
    print(f"Raw Generated Text: '{generated_text}'")

    # --- Analyze Similarity ---
    print("\n--- Similarity Analysis ---")
    analysis_results = analyze_similarity(generated_text, dictionary)

    # Print results in a readable format
    print(f"{ 'Generated Word':<20} | { 'Closest Dictionary Word':<25} | { 'Distance':<10}")
    print("-" * 70)
    for item in analysis_results:
        print(f"{item['generated_word']:<20} | {item['closest_word']:<25} | {item['distance']:<10}")

    # Print words in order of appearance
    print("\n--- Words in Order of Appearance ---")
    ordered_words_for_display = [item['generated_word'] for item in analysis_results]
    print(" ".join(ordered_words_for_display))
    print("------------------------------------")

if __name__ == "__main__":
    main()