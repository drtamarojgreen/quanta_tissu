
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
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz") # Assuming the 50k checkpoint
CORPUS_SENTIMENTS_PATH = os.path.join(project_root, "data", "corpus_sentiments.cat")

def get_word_sentiment(word, sentiment_analyzer):
    """
    Gets the sentiment score for a single word from the analyzer's lexicon.
    """
    return sentiment_analyzer.sentiment_scores.get(word.lower(), 0.0)

def main():
    parser = argparse.ArgumentParser(description="Generate text and create a sentiment heatmap.")
    parser.add_argument(
        "--prompt",
        type=str,
        default="The meaning of life is",
        help="The initial prompt to start generation."
    )
    parser.add_argument(
        "--length", 
        type=int, 
        default=100, 
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
        "--target_sentiment",
        type=str,
        choices=['positive', 'negative', 'neutral'],
        default='neutral',
        help="Target sentiment for comparison (positive, negative, or neutral)."
    )
    args = parser.parse_args()

    # --- Load Tokenizer and Model ---
    print("--- Loading tokenizer and model ---")
    tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # --- Initialize Sentiment Analyzer ---
    print(f"--- Initializing Sentiment Analyzer from {CORPUS_SENTIMENTS_PATH} ---")
    sentiment_analyzer = SentimentAnalyzer(tokenizer, CORPUS_SENTIMENTS_PATH)
    if not sentiment_analyzer.sentiment_scores:
        print("Error: Sentiment lexicon not loaded. Cannot generate heatmap.")
        sys.exit(1)

    # --- Generate Text ---
    print(f"\n--- Generating Text with prompt: '{args.prompt}' ---")
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

    # --- Generate Sentiment Heatmap ---
    print("\n--- Sentiment Heatmap Analysis ---")
    words = re.findall(r'\b\w+\b', generated_text)
    
    heatmap_data = []
    total_sentiment = 0.0
    scored_words_count = 0

    for word in words:
        score = get_word_sentiment(word, sentiment_analyzer)
        heatmap_data.append({'word': word, 'sentiment': score})
        if score != 0.0: # Only count non-neutral words for average
            total_sentiment += score
            scored_words_count += 1

    # Print heatmap data
    for item in heatmap_data:
        print(f"  {item['word']}: {item['sentiment']:.2f}")

    # Calculate overall sentiment and deviation
    overall_average_sentiment = total_sentiment / scored_words_count if scored_words_count > 0 else 0.0
    
    target_score = 0.0
    if args.target_sentiment == 'positive':
        target_score = 1.0 # Assuming positive words have score > 0
    elif args.target_sentiment == 'negative':
        target_score = -1.0 # Assuming negative words have score < 0

    deviation = abs(overall_average_sentiment - target_score)

    print("\n--- Overall Sentiment Report ---")
    print(f"Overall Average Sentiment: {overall_average_sentiment:.2f}")
    print(f"Target Sentiment: {args.target_sentiment} (Score: {target_score:.2f})")
    print(f"Deviation from Target: {deviation:.2f}")
    print("----------------------------------")

if __name__ == "__main__":
    main()
