
import os
import sys
import argparse
import numpy as np

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
SENTIMENT_LEXICON_PATH = os.path.join(project_root, "data", "sentiments.cat")

def main():
    parser = argparse.ArgumentParser(description="Generate text with sentiment biasing.")
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
        "--sentiment",
        type=str,
        choices=['positive', 'negative'],
        required=True,
        help="Desired sentiment for generation (positive or negative)."
    )
    parser.add_argument(
        "--sentiment_strength",
        type=float,
        default=0.8,
        help="Strength of the sentiment bias (0.0 to 1.0)."
    )
    args = parser.parse_args()

    # --- Load Tokenizer and Model ---
    print("--- Loading tokenizer and model ---")
    tokenizer = Tokenizer(tokenizer_prefix=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # --- Initialize Sentiment Analyzer and get bias ---
    print(f"--- Initializing Sentiment Analyzer from {SENTIMENT_LEXICON_PATH} ---")
    sentiment_analyzer = SentimentAnalyzer(tokenizer, SENTIMENT_LEXICON_PATH)
    sentiment_bias = sentiment_analyzer.get_sentiment_bias(args.sentiment, args.sentiment_strength)
    print(f"Generated bias for {len(sentiment_bias)} unique token IDs for '{args.sentiment}' sentiment.")

    # --- Generate Text with Sentiment Bias ---
    print(f"\n--- Generating {args.sentiment} text... ---")
    generated_text = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt=args.prompt,
        length=args.length,
        method=args.method,
        temperature=args.temperature,
        top_k=args.top_k,
        top_p=args.top_p,
        sentiment_bias=sentiment_bias # Pass the sentiment bias
    )
    print(f"Raw Generated Text: '{generated_text}'")

if __name__ == "__main__":
    main()
