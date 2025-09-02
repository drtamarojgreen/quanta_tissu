import os
import sys
import argparse
import json
import numpy as np
import re # Import regex for tokenization

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')), "test_tokenizer")
TEST_MODEL_DIR = os.path.join(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')), "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz") # Assuming the 50k checkpoint
DICT_PATH = os.path.join(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')), "data", "dict.json")

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.config import model_config

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

def get_match_quality(distance, word_length):
    """
    Categorizes match quality based on Levenshtein distance and word length.
    """
    if distance == 0:
        return "Exact Match"
    elif distance == 1:
        return "Close Match"
    elif distance <= word_length * 0.2: # Distance is 20% or less of word length
        return "Fair Match"
    else:
        return "Poor Match"

def analyze_similarity(generated_text, dictionary, top_n=3):
    """
    For each word in the generated text, finds the top_n most similar words
    in the provided dictionary based on Levenshtein distance.
    Returns a list of dictionaries, preserving the order of unique words.
    """
    if not dictionary:
        print("Warning: Dictionary is empty. Cannot perform similarity analysis.")
        return []

    ordered_unique_words = []
    seen_words = set()
    all_generated_words = re.findall(r'\b\w+\b', generated_text.lower())
    
    for word in all_generated_words:
        if word and word not in seen_words:
            ordered_unique_words.append(word)
            seen_words.add(word)
    
    analysis_results = [] # This will be a list of dicts, preserving order

    for gen_word_lc in ordered_unique_words:
        similarities = []
        for dict_word in dictionary:
            distance = levenshtein_distance(gen_word_lc, dict_word)
            similarities.append({
                "word": dict_word,
                "distance": distance
            })
        
        # Sort by distance and get top N
        similarities.sort(key=lambda x: x['distance'])
        top_n_matches = similarities[:top_n]

        # Add match quality to each top match
        for match in top_n_matches:
            match['quality'] = get_match_quality(match['distance'], len(gen_word_lc))
        
        analysis_results.append({
            "generated_word": gen_word_lc,
            "top_matches": top_n_matches
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
    parser.add_argument(
        "--top_n_similar",
        type=int,
        default=3,
        help="Number of top similar words to report for each generated word."
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
    analysis_results = analyze_similarity(generated_text, dictionary, top_n=args.top_n_similar)

    # Print results in a readable format
    print(f"{ 'Generated Word':<20} | { 'Closest Dictionary Word(s)':<40} | { 'Distance':<10} | { 'Quality':<15}")
    print("-" * 100)
    for item in analysis_results:
        gen_word = item['generated_word']
        for i, match in enumerate(item['top_matches']):
            if i == 0:
                print(f"{gen_word:<20} | {match['word'] + ' (' + str(match['distance']) + ')':<40} | {match['distance']:<10} | {match['quality']:<15}")
            else:
                print(f"{ ' ':<20} | {match['word'] + ' (' + str(match['distance']) + ')':<40} | {match['distance']:<10} | {match['quality']:<15}")

    # Print words in order of appearance
    print("\n--- Words in Order of Appearance ---")
    ordered_words_for_display = [item['generated_word'] for item in analysis_results]
    print(" ".join(ordered_words_for_display))
    print("------------------------------------")

if __name__ == "__main__":
    main()
