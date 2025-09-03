import os
import sys
import numpy as np
import time
from datetime import datetime, timezone

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.layers import softmax

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")

# --- Helper function for text generation ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    if method == "nucleus" and "top_p" not in kwargs:
        kwargs["top_p"] = 0.9
    prompt_tokens = tokenizer.tokenize(prompt).tolist()
    generated_tokens = model.sample(
        prompt_tokens=prompt_tokens,
        n_new_tokens=length,
        method=method,
        tokenizer=tokenizer,  # Pass tokenizer for methods that need it
        **kwargs
    )
    return tokenizer.detokenize(np.array(generated_tokens))

def run_kv_cache_test(model, tokenizer):
    """Evaluates KV Cache correctness and performance."""
    report = ["\n--- Test 1: KV Cache Evaluation ---"]
    prompt = "The laws of physics state that"
    n_new_tokens = 20

    report.append(f"Prompt: '{prompt}'")
    report.append(f"Tokens to generate: {n_new_tokens}")

    # --- 1. Baseline Generation (No Cache) ---
    start_time_no_cache = time.time()

    # Manually simulate token-by-token generation without a cache
    generated_tokens_no_cache = []
    current_tokens = tokenizer.tokenize(prompt).tolist()

    # First forward pass for the prompt
    prompt_array = np.array([current_tokens])
    logits, _ = model.forward(prompt_array, start_pos=0)

    for i in range(n_new_tokens):
        # Predict next token from the last logit
        last_logit = logits[:, -1, :]
        probs = softmax(last_logit)
        next_token = int(np.argmax(probs))
        generated_tokens_no_cache.append(next_token)

        # Prepare for next iteration: input is just the new token
        next_token_array = np.array([[next_token]])
        # The start_pos tells the model to recompute context from scratch
        logits, _ = model.forward(next_token_array, start_pos=len(current_tokens) + i)

    end_time_no_cache = time.time()
    time_no_cache = end_time_no_cache - start_time_no_cache
    report.append(f"  No Cache Generation Time: {time_no_cache:.4f} seconds")
    report.append(f"  Generated (No Cache): {tokenizer.detokenize(np.array(generated_tokens_no_cache))}")

    # --- 2. Cached Generation ---
    start_time_cache = time.time()

    # Manually simulate token-by-token generation WITH a cache
    generated_tokens_cache = []
    current_tokens_cache = tokenizer.tokenize(prompt).tolist()

    # Initialize the cache object: a list of empty dicts, one for each transformer layer
    n_layers = model.config.get("n_layer", 12) # Default to 12 if not in config
    kv_cache = [{} for _ in range(n_layers)]

    # First forward pass for the prompt, populating the cache
    prompt_array_cache = np.array([current_tokens_cache])
    logits_cache, _ = model.forward(prompt_array_cache, kv_cache=kv_cache, start_pos=0)

    for i in range(n_new_tokens):
        # Predict next token from the last logit
        last_logit_cache = logits_cache[:, -1, :]
        probs_cache = softmax(last_logit_cache)
        next_token_cache = int(np.argmax(probs_cache))
        generated_tokens_cache.append(next_token_cache)

        # Prepare for next iteration: input is the new token, but now we pass the cache
        next_token_array_cache = np.array([[next_token_cache]])
        # The start_pos and cache tell the model to append to the existing context
        logits_cache, _ = model.forward(
            next_token_array_cache,
            kv_cache=kv_cache,
            start_pos=len(current_tokens_cache) + i
        )

    end_time_cache = time.time()
    time_cache = end_time_cache - start_time_cache
    report.append(f"  Cached Generation Time:   {time_cache:.4f} seconds")
    report.append(f"  Generated (Cached):   {tokenizer.detokenize(np.array(generated_tokens_cache))}")

    # --- 3. Comparison and Verification ---
    correctness = (generated_tokens_no_cache == generated_tokens_cache)
    report.append(f"\n  Correctness Check (tokens match): {correctness}")
    if time_no_cache > 0:
        speedup = (time_no_cache - time_cache) / time_no_cache * 100
        report.append(f"  Performance Improvement: {speedup:.2f}%")

    if not correctness:
        report.append("  [WARNING] KV Cache output differs from non-cached output.")

    return report
