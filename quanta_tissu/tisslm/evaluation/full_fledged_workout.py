
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
from quanta_tissu.tisslm.evaluation.generate_similar import analyze_similarity, levenshtein_distance # Reusing analysis
from quanta_tissu.tisslm.core.rules.enforcer import RuleEnforcer
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz") # Assuming the 50k checkpoint
DICT_PATH = os.path.join(project_root, "data", "dict.json")
WORDLIST_PATH = os.path.join(project_root, "data", "wordlist.txt")
TEMP_LIST_PATH = os.path.join(project_root, "data", "temp_list.txt")

# --- Test Configurations (subset of test_tune_prompt) ---
TEST_CONFIGURATIONS = [
    {"prompt": "The meaning of life is", "method": "greedy", "temperature": 1.0, "top_k": 1, "top_p": 1.0, "length": 50},
    {"prompt": "The future of humanity is", "method": "nucleus", "temperature": 0.7, "top_k": 50, "top_p": 0.85, "length": 50},
    {"prompt": "Once upon a time, in a land far away", "method": "nucleus", "temperature": 0.9, "top_k": 100, "top_p": 0.95, "length": 50},
]

def main():
    parser = argparse.ArgumentParser(description="Run a full evaluation workout on the trained model.")
    parser.add_argument(
        "--prompt",
        type=str,
        default="The meaning of life is",
        help="Default prompt for single generation tests."
    )
    args = parser.parse_args()

    report = []

    # --- Load Dictionary ---
    dictionary = set()
    if os.path.exists(DICT_PATH):
        try:
            with open(DICT_PATH, 'r', encoding='utf-8') as f:
                dictionary = set(json.load(f))
            report.append(f"Dictionary loaded: {len(dictionary)} words.")
        except (IOError, json.JSONDecodeError) as e:
            report.append(f"Warning: Could not load dictionary from {DICT_PATH}: {e}")
    else:
        report.append(f"Warning: Dictionary not found at {DICT_PATH}.")

    # --- Load Tokenizer and Model ---
    try:
        tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
        model_config["vocab_size"] = tokenizer.get_vocab_size()
        model = QuantaTissu(model_config)
        model.load_weights(FINAL_CHECKPOINT_PATH)
        report.append("Tokenizer and model loaded successfully.")
    except Exception as e:
        report.append(f"Error loading tokenizer or model: {e}")
        print("\n".join(report))
        sys.exit(1)

    # --- Test 1: Standard Generation Configurations ---
    report.append("\n--- Test 1: Standard Generation Configurations ---")
    for i, config in enumerate(TEST_CONFIGURATIONS):
        generated_text = generate_text(
            model=model,
            tokenizer=tokenizer,
            prompt=config['prompt'],
            length=config['length'],
            method=config['method'],
            temperature=config['temperature'],
            top_k=config['top_k'],
            top_p=config['top_p']
        )
        report.append(f"Config #{i+1} (Method: {config['method']}): '{generated_text[:100]}...'")

    # --- Test 2: Similarity Analysis ---
    report.append("\n--- Test 2: Similarity Analysis ---")
    sample_text_for_analysis = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt=args.prompt,
        length=50,
        method="nucleus",
        temperature=0.8,
        top_k=40,
        top_p=0.9
    )
    analysis_results = analyze_similarity(sample_text_for_analysis, dictionary)
    report.append(f"Sample text for analysis: '{sample_text_for_analysis[:100]}'...")
    if analysis_results:
        report.append("Top 5 similar words:")
        for i, item in enumerate(analysis_results[:5]): # Show only top 5 for brevity
            report.append(f"  - '{item['generated_word']}' -> '{item['closest_word']}' (Dist: {item['distance']})")
    else:
        report.append("No words found for similarity analysis.")

    # --- Test 3: Rule Enforcement ---
    report.append("\n--- Test 3: Rule Enforcement ---")
    raw_text_for_rules = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt="this is a test. it has some repeated words. words words. and missing punctuation",
        length=50,
        method="nucleus",
        temperature=0.8,
        top_k=40,
        top_p=0.9
    )
    enforcer_default = RuleEnforcer(strictness=0.5)
    cleaned_text_default = enforcer_default.apply_rules(raw_text_for_rules)
    report.append(f"Raw text for rules: '{raw_text_for_rules[:100]}'...")
    report.append(f"Cleaned (Default Strictness): '{cleaned_text_default[:100]}'...")

    enforcer_strict = RuleEnforcer(strictness=1.0)
    cleaned_text_strict = enforcer_strict.apply_rules(raw_text_for_rules)
    report.append(f"Cleaned (Strict Strictness): '{cleaned_text_strict[:100]}...'")

    # --- Test 4: Experimental Methods ---
    report.append("\n--- Test 4: Experimental Methods ---")
    # Dynamic Token Revision
    try:
        generated_dtr = model.sample(
            tokenizer.tokenize("Testing dynamic token revision").tolist(),
            n_new_tokens=30,
            method="dynamic_token_revision",
            underlying_method="greedy",
            save_interval=10
        )
        report.append(f"Dynamic Token Revision: Generated '{tokenizer.detokenize(np.array(generated_dtr))[:50]}...'")
        report.append(f"  Check {TEMP_LIST_PATH} for token counts.")
    except Exception as e:
        report.append(f"Error in Dynamic Token Revision: {e}")

    # Bayesian Word Expansion (requires query_embedding and hessian_matrix, which are not easily mocked)
    # This part will likely fail without proper setup, but demonstrates the call.
    # try:
    #     # Mock query_embedding and hessian_matrix for demonstration purposes
    #     # In a real scenario, these would come from a retrieval system
    #     mock_embedding_dim = model_config.get('n_embd', 128)
    #     mock_query_embedding = np.random.rand(mock_embedding_dim).tolist()
    #     mock_hessian_matrix = np.random.rand(mock_embedding_dim, mock_embedding_dim).tolist() # Convert to list

    #     generated_bwe = model.sample(
    #         tokenizer.tokenize("Testing word expansion").tolist(),
    #         n_new_tokens=30,
    #         method="bayesian_word_expansion",
    #         tokenizer=tokenizer, # Pass tokenizer for decoding
    #         query_embedding=mock_query_embedding,
    #         hessian_matrix=mock_hessian_matrix,
    #         expansion_threshold=0.5 # Lower threshold for testing
    #     )
    #     report.append(f"Bayesian Word Expansion: Generated '{tokenizer.detokenize(np.array(generated_bwe))[:50]}...'")
    #     report.append(f"  Check {WORDLIST_PATH} for new words.")
    # except Exception as e:
    #     report.append(f"Error in Bayesian Word Expansion: {e}")

    # --- Final Report ---
    print("\n" + "="*80)
    print("FULL FLEDGED WORKOUT REPORT")
    print("="*80)
    print("\n".join(report))
    print("\n" + "="*80)

if __name__ == "__main__":
    main()
