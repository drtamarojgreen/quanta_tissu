import os
import sys
import argparse
import json
import numpy as np
import re

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.evaluation.generate_similar import levenshtein_distance # Reusing Levenshtein
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer

# ---
# Configuration ---
# ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz") # Assuming the 50k checkpoint
OXFORD_DICT_PATH = os.path.join(project_root, "data", "dict.json")
SENTIMENT_LEXICON_PATH = os.path.join(project_root, "data", "sentiments.cat")

def analyze_sentiment_similarity(generated_text, oxford_dictionary, sentiment_analyzer):
    """
    For each word in the generated text, finds the most similar word
    in the Oxford dictionary and reports its sentiment.
    """
    if not oxford_dictionary:
        print("Warning: Oxford Dictionary is empty. Cannot perform similarity analysis.")
        return []

    ordered_unique_words = []
    all_generated_words = re.findall(r'\b\w+\b', generated_text.lower())
    seen_words = set()
    for word in all_generated_words:
        if word and word not in seen_words:
            ordered_unique_words.append(word)
            seen_words.add(word)
    
    analysis_results = []

    for gen_word_lc in ordered_unique_words:
        min_distance = float('inf')
        closest_oxford_word = ""

        for dict_word in oxford_dictionary:
            distance = levenshtein_distance(gen_word_lc, dict_word)
            if distance < min_distance:
                min_distance = distance
                closest_oxford_word = dict_word
            if distance == 0:
                break
        
        # Get sentiment score for the closest Oxford word
        sentiment_score = sentiment_analyzer.sentiment_scores.get(closest_oxford_word, 0.0)

        analysis_results.append({
            "generated_word": gen_word_lc,
            "closest_oxford_word": closest_oxford_word,
            "distance": min_distance,
            "sentiment_score": sentiment_score
        })
        
    return analysis_results

def main():
    parser = argparse.ArgumentParser(description="Generate text, find similar Oxford words, and analyze sentiment.")
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
    args = parser.parse_args()

    # ---
    # Load Oxford Dictionary ---
    # ---
    print(f"--- Loading Oxford dictionary from {OXFORD_DICT_PATH} ---")
    oxford_dictionary = set()
    try:
        with open(OXFORD_DICT_PATH, 'r', encoding='utf-8') as f:
            oxford_dictionary = set(json.load(f))
        print(f"Successfully loaded {len(oxford_dictionary)} Oxford words.")
    except (IOError, json.JSONDecodeError) as e:
        print(f"Error loading Oxford dictionary from {OXFORD_DICT_PATH}: {e}. Proceeding without it.")

    # ---
    # Load Tokenizer and Model ---
    # ---
    print("--- Loading tokenizer and model ---")
    tokenizer = Tokenizer(tokenizer_prefix=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # ---
    # Initialize Sentiment Analyzer ---
    # ---
    print(f"--- Initializing Sentiment Analyzer from {SENTIMENT_LEXICON_PATH} ---")
    sentiment_analyzer = SentimentAnalyzer(tokenizer, SENTIMENT_LEXICON_PATH)

    # ---
    # Generate Text ---
    # ---
    print("\n--- Generating Text... ---")
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

    # ---
    # Analyze Sentiment Similarity ---
    # ---
    print("\n--- Sentiment Similarity Analysis ---")
    analysis_results = analyze_sentiment_similarity(generated_text, oxford_dictionary, sentiment_analyzer)

    # Print results in a readable format
    print(f"{ 'Generated Word':<20} | { 'Closest Oxford Word':<25} | { 'Distance':<10} | { 'Sentiment Score':<15}")
    print("-" * 80)
    for item in analysis_results:
        print(f"{item['generated_word']:<20} | {item['closest_oxford_word']:<25} | {item['distance']:<10} | {item['sentiment_score']:<15.2f}")

if __name__ == "__main__":
    main()
