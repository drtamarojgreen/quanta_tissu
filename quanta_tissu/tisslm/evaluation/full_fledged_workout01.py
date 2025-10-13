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

# --- Expanded Test Configurations ---
KV_CACHE_TEST_SCENARIOS = [
    {"prompt": "The laws of physics state that", "tokens": 80, "description": "Short physics prompt"},
    {"prompt": "In the beginning of the universe, there was nothing but darkness and void", "tokens": 200, "description": "Medium cosmology prompt"},
    {"prompt": "The development of artificial intelligence has progressed rapidly in recent years, with breakthroughs in machine learning, natural language processing, and computer vision leading to", "tokens": 400, "description": "Long AI development prompt"},
    {"prompt": "Once upon a time in a distant galaxy far from Earth, there lived a civilization of beings who had mastered the art of interstellar travel and communication across vast distances", "tokens": 600, "description": "Extended narrative prompt"},
]

GENERATION_METHODS = [
    {"method": "greedy", "params": {}},
    {"method": "nucleus", "params": {"top_p": 0.9, "temperature": 0.8}},
    {"method": "nucleus", "params": {"top_p": 0.7, "temperature": 1.2}},
]

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

def run_single_kv_cache_test(model, tokenizer, prompt, n_new_tokens, method_config):
    """Run a single KV cache test with detailed analysis."""
    test_results = {
        "prompt": prompt,
        "n_new_tokens": n_new_tokens,
        "method": method_config["method"],
        "params": method_config["params"]
    }
    
    # --- 1. Baseline Generation (No Cache) ---
    start_time_no_cache = time.time()
    
    # Manually simulate token-by-token generation without a cache
    generated_tokens_no_cache = []
    current_tokens = tokenizer.tokenize(prompt).tolist()
    
    # First forward pass for the prompt
    prompt_array = np.array([current_tokens])
    logits, _, _ = model.forward(prompt_array, start_pos=0)
    
    for i in range(n_new_tokens):
        # Predict next token from the last logit
        last_logit = logits[:, -1, :]
        
        # Apply generation method
        if method_config["method"] == "greedy":
            probs = softmax(last_logit)
            next_token = int(np.argmax(probs))
        elif method_config["method"] == "nucleus":
            temp = method_config["params"].get("temperature", 1.0)
            top_p = method_config["params"].get("top_p", 0.9)

            probs = softmax(last_logit, temperature=temp)[0]


            
            # Nucleus sampling
            sorted_indices = np.argsort(probs)[::-1]


            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)
            cutoff_idx = np.where(cumulative_probs >= top_p)[0][0]
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
            nucleus_probs = np.zeros_like(probs)
            nucleus_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]
            nucleus_probs /= np.sum(nucleus_probs)
            next_token = np.random.choice(len(probs), p=nucleus_probs)
        
        generated_tokens_no_cache.append(next_token)
        
        # Prepare for next iteration: input is just the new token
        next_token_array = np.array([[next_token]])
        # The start_pos tells the model to recompute context from scratch
        logits, _, _ = model.forward(next_token_array, start_pos=len(current_tokens) + i)
    end_time_no_cache = time.time()
    time_no_cache = end_time_no_cache - start_time_no_cache
    
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
    logits_cache, _, _ = model.forward(prompt_array_cache, kv_cache=kv_cache, start_pos=0)
    
    for i in range(n_new_tokens):
        # Predict next token from the last logit
        last_logit_cache = logits_cache[:, -1, :]
        
        # Apply same generation method as no-cache version
        if method_config["method"] == "greedy":
            probs_cache = softmax(last_logit_cache)
            next_token_cache = int(np.argmax(probs_cache))
        elif method_config["method"] == "nucleus":
            temp = method_config["params"].get("temperature", 1.0)
            top_p = method_config["params"].get("top_p", 0.9)
            probs_cache = softmax(last_logit_cache, temperature=temp)[0]
            
            # Nucleus sampling
            sorted_indices = np.argsort(probs_cache)[::-1]
            sorted_probs = probs_cache[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)
            cutoff_idx = np.where(cumulative_probs >= top_p)[0][0]
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
            nucleus_probs = np.zeros_like(probs_cache)
            nucleus_probs[tokens_to_keep_indices] = probs_cache[tokens_to_keep_indices]
            nucleus_probs /= np.sum(nucleus_probs)
            next_token_cache = np.random.choice(len(probs_cache), p=nucleus_probs)
        
        generated_tokens_cache.append(next_token_cache)
        
        # Prepare for next iteration: input is the new token, but now we pass the cache
        next_token_array_cache = np.array([[next_token_cache]])
        # The start_pos and cache tell the model to append to the existing context
        logits_cache, _, _ = model.forward(
            next_token_array_cache,
            kv_cache=kv_cache,
            start_pos=len(current_tokens_cache) + i
        )
    
    end_time_cache = time.time()
    time_cache = end_time_cache - start_time_cache
    
    # --- 3. Analysis ---
    test_results.update({
        "time_no_cache": time_no_cache,
        "time_cache": time_cache,
        "generated_tokens_no_cache": generated_tokens_no_cache,
        "generated_tokens_cache": generated_tokens_cache,
        "generated_text_no_cache": tokenizer.detokenize(np.array(generated_tokens_no_cache)),
        "generated_text_cache": tokenizer.detokenize(np.array(generated_tokens_cache)),
        "correctness": (generated_tokens_no_cache == generated_tokens_cache),
        "speedup_percent": ((time_no_cache - time_cache) / time_no_cache * 100) if time_no_cache > 0 else 0,
        "cache_efficiency": time_cache / time_no_cache if time_no_cache > 0 else 1.0
    })
    
    return test_results

def analyze_kv_cache_performance(all_results):
    """Analyze performance across all KV cache tests."""
    analysis = {
        "total_tests": len(all_results),
        "successful_tests": len([r for r in all_results if r.get("correctness", False)]),
        "average_speedup": np.mean([r.get("speedup_percent", 0) for r in all_results]),
        "average_cache_efficiency": np.mean([r.get("cache_efficiency", 1.0) for r in all_results]),
        "speedup_by_length": {},
        "speedup_by_method": {},
        "correctness_rate": 0
    }
    
    # Calculate correctness rate
    correct_tests = len([r for r in all_results if r.get("correctness", False)])
    analysis["correctness_rate"] = correct_tests / len(all_results) if all_results else 0
    
    # Analyze by token length
    length_groups = {}
    for result in all_results:
        length = result["n_new_tokens"]
        if length not in length_groups:
            length_groups[length] = []
        length_groups[length].append(result["speedup_percent"])
    
    for length, speedups in length_groups.items():
        analysis["speedup_by_length"][length] = {
            "average": np.mean(speedups),
            "min": np.min(speedups),
            "max": np.max(speedups),
            "count": len(speedups)
        }
    
    # Analyze by method
    method_groups = {}
    for result in all_results:
        method = result["method"]
        if method not in method_groups:
            method_groups[method] = []
        method_groups[method].append(result["speedup_percent"])
    
    for method, speedups in method_groups.items():
        analysis["speedup_by_method"][method] = {
            "average": np.mean(speedups),
            "min": np.min(speedups),
            "max": np.max(speedups),
            "count": len(speedups)
        }
    
    return analysis

def run_kv_cache_test(model, tokenizer):
    """Evaluates KV Cache correctness and performance with comprehensive testing."""
    report = ["\n--- Test 1: Enhanced KV Cache Evaluation ---"]
    report.append(f"Testing {len(KV_CACHE_TEST_SCENARIOS)} scenarios with {len(GENERATION_METHODS)} generation methods")
    
    all_results = []
    
    # Run all test combinations
    for scenario_idx, scenario in enumerate(KV_CACHE_TEST_SCENARIOS):
        report.append(f"\n  Scenario {scenario_idx + 1}: {scenario['description']}")
        report.append(f"  Prompt: '{scenario['prompt'][:50]}...'")
        report.append(f"  Tokens to generate: {scenario['tokens']}")
        
        for method_idx, method_config in enumerate(GENERATION_METHODS):
            report.append(f"\n    Method {method_idx + 1}: {method_config['method']} {method_config['params']}")
            
            try:
                # Set random seed for reproducible results in stochastic methods
                if method_config["method"] != "greedy":
                    np.random.seed(42 + scenario_idx * 10 + method_idx)
                
                test_result = run_single_kv_cache_test(
                    model, tokenizer, 
                    scenario["prompt"], 
                    scenario["tokens"], 
                    method_config
                )
                
                all_results.append(test_result)
                
                # Report individual test results
                report.append(f"      No Cache Time: {test_result['time_no_cache']:.4f}s")
                report.append(f"      Cached Time:   {test_result['time_cache']:.4f}s")
                report.append(f"      Speedup:       {test_result['speedup_percent']:.2f}%")
                report.append(f"      Correctness:   {'✓' if test_result['correctness'] else '✗'}")
                
                if not test_result['correctness']:
                    report.append(f"      [WARNING] Output mismatch detected!")
                    report.append(f"        No Cache: '{test_result['generated_text_no_cache']}'")
                    report.append(f"        Cached:   '{test_result['generated_text_cache']}'")
                
            except Exception as e:
                report.append(f"      [ERROR] Test failed: {str(e)}")
                # Add a failed result for analysis
                all_results.append({
                    "prompt": scenario["prompt"],
                    "n_new_tokens": scenario["tokens"],
                    "method": method_config["method"],
                    "params": method_config["params"],
                    "error": str(e),
                    "correctness": False,
                    "speedup_percent": 0,
                    "cache_efficiency": 1.0
                })
    
    # --- Overall Analysis ---
    if all_results:
        analysis = analyze_kv_cache_performance(all_results)
        
        report.append(f"\n--- KV Cache Performance Analysis ---")
        report.append(f"  Total tests run: {analysis['total_tests']}")
        report.append(f"  Successful tests: {analysis['successful_tests']}")
        report.append(f"  Correctness rate: {analysis['correctness_rate']:.1%}")
        report.append(f"  Average speedup: {analysis['average_speedup']:.2f}%")
        report.append(f"  Average cache efficiency: {analysis['average_cache_efficiency']:.3f}")
        
        # Performance by token length
        report.append(f"\n  Performance by Token Length:")
        for length, stats in analysis["speedup_by_length"].items():
            report.append(f"    {length} tokens: {stats['average']:.2f}% avg speedup (min: {stats['min']:.2f}%, max: {stats['max']:.2f}%)")
        
        # Performance by method
        report.append(f"\n  Performance by Generation Method:")
        for method, stats in analysis["speedup_by_method"].items():
            report.append(f"    {method}: {stats['average']:.2f}% avg speedup (tests: {stats['count']})")
        
        # Recommendations
        report.append(f"\n  Recommendations:")
        if analysis['correctness_rate'] < 1.0:
            report.append(f"    ⚠️  KV cache correctness issues detected - investigate implementation")
        if analysis['average_speedup'] > 10:
            report.append(f"    ✅ KV cache provides significant performance improvement")
        elif analysis['average_speedup'] > 0:
            report.append(f"    ℹ️  KV cache provides modest performance improvement")
        else:
            report.append(f"    ⚠️  KV cache may have performance overhead - investigate")
    
    return report
