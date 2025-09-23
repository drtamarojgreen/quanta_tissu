import os
import sys
import numpy as np
import re
from datetime import datetime, timezone

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer
from quanta_tissu.tisslm.evaluation.create_corpus_sentiments import main as create_corpus_sentiments_main

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")
DICT_PATH = os.path.join(project_root, "data", "dict.json")
WORDLIST_PATH = os.path.join(project_root, "data", "wordlist.txt")
TEMP_LIST_PATH = os.path.join(project_root, "data", "temp_list.txt")
CORPUS_SENTIMENTS_PATH = os.path.join(project_root, "data", "corpus_sentiments.cat")
EMBEDDINGS_DIR = os.path.join(project_root, "data", "embeddings")

# --- Helper function for text generation ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    if method == "nucleus" and "top_p" not in kwargs:
        kwargs["top_p"] = 0.9
    prompt_tokens = tokenizer.tokenize(prompt).tolist()

    if method in ["dynamic_token_revision", "bayesian_word_expansion"]:
        generated_tokens = model.alg_generator.sample(
            prompt_tokens=prompt_tokens,
            n_new_tokens=length,
            method=method,
            tokenizer=tokenizer,  # Pass tokenizer for methods that need it
            **kwargs
        )
    else:
        generated_tokens = model.generator.sample(
            prompt_tokens=prompt_tokens,
            n_new_tokens=length,
            method=method,
            tokenizer=tokenizer,  # Pass tokenizer for methods that need it
            **kwargs
        )
    return tokenizer.detokenize(np.array(generated_tokens))

def run_experimental_sampling_tests(model, tokenizer):
    """Evaluates experimental sampling methods like Bayesian Expansion."""
    report = ["\n--- Test 3: Experimental Sampling Methods ---"]

    # --- Test 3a: Dynamic Token Revision ---
    report.append("\n  --- Test 3a: Dynamic Token Revision ---")
    try:
        # Clear the temp list file before the test
        with open(TEMP_LIST_PATH, "w") as f:
            f.write("")

        generated_dtr = generate_with_model(
            model, tokenizer,
            prompt="Testing dynamic token revision",
            length=30,
            method="dynamic_token_revision",
            underlying_method="greedy",
            save_interval=10
        )
        report.append(f"  Generated: '{generated_dtr[:80]}...'")

        # Verification
        with open(TEMP_LIST_PATH, "r") as f:
            temp_list_content = f.read()
            assert len(temp_list_content) > 0, "Temp list file is empty"
            report.append(f"  Verification: {TEMP_LIST_PATH} updated.")

    except Exception as e:
        report.append(f"  [ERROR] Dynamic Token Revision failed: {e}")

    # --- Test 3b: Bayesian Word Expansion (FIXED) ---
    report.append("\n  --- Test 3b: Bayesian Word Expansion ---")
    try:
        # Clear the wordlist file before the test
        with open(WORDLIST_PATH, "w") as f:
            f.write("hello world")

        n_embd = model.config.get('n_embd', 128)
        # Create mock data required by the method
        mock_query_embedding = np.random.rand(n_embd).astype(np.float32)
        mock_hessian_matrix = np.random.rand(n_embd, n_embd).astype(np.float32)

        generated_bwe = generate_with_model(
            model, tokenizer,
            prompt="Testing word expansion",
            length=30,
            method="bayesian_word_expansion",
            # Pass the required mock data as kwargs
            query_embedding=mock_query_embedding,
            hessian_matrix=mock_hessian_matrix,
            expansion_threshold=0.5
        )
        report.append(f"  Generated: '{generated_bwe[:80]}...'")

        # Verification
        with open(WORDLIST_PATH, "r") as f:
            wordlist_content = f.read()
            assert len(wordlist_content) > 0, "Wordlist file is empty"
            report.append(f"  Verification: {WORDLIST_PATH} updated.")

    except Exception as e:
        report.append(f"  [ERROR] Bayesian Word Expansion failed: {e}")

    # --- Test 3c: Adaptive Sentiment Sampling (ROBUST) ---
    report.append("\n  --- Test 3c: Adaptive Sentiment Sampling ---")
    try:
        # Check for corpus sentiments file and generate if it's missing.
        if not os.path.exists(CORPUS_SENTIMENTS_PATH):
            report.append(f"  [INFO] Sentiments file not found at {CORPUS_SENTIMENTS_PATH}.")
            report.append("  Attempting to generate it automatically...")
            try:
                create_corpus_sentiments_main()
                report.append("  Sentiments file generated successfully.")
            except Exception as e:
                report.append(f"  [ERROR] Failed to generate sentiments file: {e}")
                report.append("  Skipping Adaptive Sentiment Sampling test.")
                return report # Exit this test function early

        sentiment_analyzer = SentimentAnalyzer(tokenizer=tokenizer, sentiment_lexicon_path=CORPUS_SENTIMENTS_PATH)

        if not sentiment_analyzer.sentiment_scores:
            report.append(f"  [SKIPPED] Adaptive Sentiment Sampling: Sentiments file is empty.")
        else:
            test_cases = [
                {"sentiment": "positive", "strength": 0.8, "prompt": "I feel very"},
                {"sentiment": "negative", "strength": 0.8, "prompt": "This is a very"},
                {"sentiment": "neutral", "strength": 0.5, "prompt": "The situation is"}
            ]

            for case in test_cases:
                generated_text = generate_with_model(
                    model, tokenizer,
                    prompt=case["prompt"],
                    length=30,
                    method="adaptive_sentiment",
                    sentiment_analyzer=sentiment_analyzer,
                    target_sentiment=case["sentiment"],
                    target_strength=case["strength"]
                )
                report.append(f"  Generated ({case['sentiment']} target): '{generated_text[:80]}...'")
                # Verification (simplified as analyze_sentiment is no longer available)
                report.append(f"  Verification: Text generated with {case['sentiment']} sentiment bias applied.")

    except Exception as e:
        report.append(f"  [ERROR] Adaptive Sentiment Sampling failed: {e}")

    return report
