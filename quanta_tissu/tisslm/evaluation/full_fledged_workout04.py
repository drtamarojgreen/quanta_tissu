import os
import sys
import numpy as np
import time
import re
from collections import Counter

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.generation.config import GenerationConfig

# --- Expanded Test Configurations ---
TEST_CONFIGURATIONS = [
    # Greedy Method (baseline)
    {"prompt": "The definition of science is", "method": "greedy", "length": 60, "params": {}},

    # Nucleus Sampling: Temperature variations
    {"prompt": "The future of space exploration involves", "method": "nucleus", "length": 70, "params": {"temperature": 0.5, "top_p": 0.9}},
    {"prompt": "The future of space exploration involves", "method": "nucleus", "length": 70, "params": {"temperature": 0.8, "top_p": 0.9}},
    {"prompt": "The future of space exploration involves", "method": "nucleus", "length": 70, "params": {"temperature": 1.2, "top_p": 0.9}},

    # Nucleus Sampling: Top-p variations
    {"prompt": "A novel is a work of fiction that", "method": "nucleus", "length": 80, "params": {"temperature": 0.8, "top_p": 0.7}},
    {"prompt": "A novel is a work of fiction that", "method": "nucleus", "length": 80, "params": {"temperature": 0.8, "top_p": 0.9}},
    {"prompt": "A novel is a work of fiction that", "method": "nucleus", "length": 80, "params": {"temperature": 0.8, "top_p": 0.99}},

    # Different prompt types
    {"prompt": "To build a successful startup, one must first", "method": "nucleus", "length": 90, "params": {"temperature": 0.9, "top_p": 0.9}},
    {"prompt": "Once upon a time, in a land of dragons and magic,", "method": "nucleus", "length": 100, "params": {"temperature": 0.85, "top_p": 0.95}},
]

# --- Helper function for text generation ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    if method == "nucleus" and "top_p" not in kwargs:
        kwargs["top_p"] = 0.9
    prompt_tokens = tokenizer.tokenize(prompt)

    config = GenerationConfig(
        method=method,
        temperature=kwargs.get('temperature', 1.0),
        top_k=kwargs.get('top_k'),
        top_p=kwargs.get('top_p'),
        repetition_penalty=kwargs.get('repetition_penalty', 1.0),
    )

    generated_tokens, _ = model.generator.sample(
        [prompt_tokens],
        n_new_tokens=length,
        config=config,
        **kwargs
    )
    return tokenizer.detokenize(np.array(generated_tokens[0]))

def analyze_generated_text(text, prompt, method, target_length):
    """Analyze the quality and characteristics of generated text."""
    analysis = {
        "method": method,
        "prompt": prompt,
        "target_length": target_length,
        "generated_text": text,
        "word_count": len(text.split()),
        "char_count": len(text),
        "unique_words": len(set(text.lower().split())),
        "sentences": len(re.split(r'[.!?]+', text.strip())),
    }
    
    words = text.split()
    if words:
        analysis["lexical_diversity"] = len(set(words)) / len(words)
        analysis["avg_word_length"] = np.mean([len(word) for word in words])
        
        word_counts = {}
        for word in words:
            word_counts[word] = word_counts.get(word, 0) + 1
        repeated_words = {word: count for word, count in word_counts.items() if count > 1}
        analysis["repetition_ratio"] = len(repeated_words) / len(words)
        analysis["max_repetition"] = max(word_counts.values()) if word_counts else 0
    else:
        analysis["lexical_diversity"] = 0
        analysis["avg_word_length"] = 0
        analysis["repetition_ratio"] = 0
        analysis["max_repetition"] = 0
    
    return analysis

def analyze_standard_generation_performance(all_results):
    """Analyzes the performance of standard generation configurations."""
    report = ["\n--- Standard Generation Performance Analysis ---"]
    
    if not all_results:
        report.append("  No results to analyze.")
        return report

    # Group results by generation method
    method_groups = {}
    for result in all_results:
        method = result["method"]
        if method not in method_groups:
            method_groups[method] = []
        method_groups[method].append(result)

    report.append(f"  Analyzed {len(all_results)} generation(s) across {len(method_groups)} method(s).")

    for method, results in method_groups.items():
        avg_diversity = np.mean([r["lexical_diversity"] for r in results])
        avg_repetition = np.mean([r["repetition_ratio"] for r in results])
        avg_word_count = np.mean([r["word_count"] for r in results])
        
        report.append(f"\n  Method: {method}")
        report.append(f"    - Average Lexical Diversity: {avg_diversity:.3f}")
        report.append(f"    - Average Repetition Ratio: {avg_repetition:.3f}")
        report.append(f"    - Average Word Count: {avg_word_count:.1f}")

    # Recommendations
    report.append("\n  --- Recommendations ---")
    greedy_results = method_groups.get("greedy", [])
    nucleus_results = method_groups.get("nucleus", [])

    if greedy_results and nucleus_results:
        greedy_diversity = np.mean([r["lexical_diversity"] for r in greedy_results])
        nucleus_diversity = np.mean([r["lexical_diversity"] for r in nucleus_results])
        
        if nucleus_diversity > greedy_diversity * 1.1:
            report.append("  - ✅ Nucleus sampling is producing significantly more diverse text than greedy search.")
        elif greedy_diversity > nucleus_diversity:
            report.append("  - ⚠️ Greedy search is producing more diverse text than nucleus sampling, which is unusual. Check temperature and top_p parameters.")
        else:
            report.append("  - ℹ️ Both greedy and nucleus sampling are producing text with similar lexical diversity.")

    if any(np.mean([r["repetition_ratio"] for r in results]) > 0.3 for results in method_groups.values()):
        report.append("  - ⚠️ High repetition ratios detected. Consider increasing the repetition_penalty parameter in the generation config.")
    
    report.append("  - ℹ️ For more creative and varied output, try increasing the 'temperature' and experimenting with the 'top_p' parameter in nucleus sampling.")
    report.append("  - ℹ️ For more deterministic and factual output, prefer greedy search or a lower temperature for nucleus sampling.")

    return report

def analyze_generation_quality(text):
    """Analyze the quality and characteristics of generated text."""
    analysis = {
        "word_count": 0, "lexical_diversity": 0, "repetition_ratio": 0, "avg_word_length": 0,
    }
    words = text.split()
    if not words:
        return analysis

    word_count = len(words)
    unique_words = len(set(words))
    word_counts = Counter(words)
    repeated_words = {word: count for word, count in word_counts.items() if count > 1}

    analysis["word_count"] = word_count
    analysis["lexical_diversity"] = unique_words / word_count if word_count > 0 else 0
    analysis["repetition_ratio"] = len(repeated_words) / word_count if word_count > 0 else 0
    analysis["avg_word_length"] = np.mean([len(w) for w in words])

    return analysis

def analyze_overall_performance(all_results):
    """Analyze and summarize the results from all generation tests."""
    report = ["\n--- Standard Generation Performance Analysis ---"]
    total_tests = len(all_results)
    successful_tests = len([r for r in all_results if "error" not in r])

    if total_tests == 0:
        report.append("  No generation tests were executed.")
        return report

    report.append(f"  Total tests run: {total_tests}")
    report.append(f"  Successful tests: {successful_tests} ({successful_tests/total_tests:.1%})")

    if not successful_tests:
        return report

    # Analyze by generation method
    report.append("\n  Performance by Generation Method:")
    method_groups = {}
    for r in all_results:
        if "error" not in r:
            method = r["config"]["method"]
            if method not in method_groups:
                method_groups[method] = []
            method_groups[method].append(r)

    for method, results in method_groups.items():
        avg_time = np.mean([r["performance"]["generation_time"] for r in results])
        avg_diversity = np.mean([r["quality"]["lexical_diversity"] for r in results])
        avg_repetition = np.mean([r["quality"]["repetition_ratio"] for r in results])
        report.append(f"    Method '{method}' ({len(results)} tests):")
        report.append(f"      Avg. Time: {avg_time:.3f}s")
        report.append(f"      Avg. Lexical Diversity: {avg_diversity:.3f}")
        report.append(f"      Avg. Repetition Ratio: {avg_repetition:.3f}")

    # Analyze by temperature (for nucleus)
    report.append("\n  Impact of Temperature (Nucleus Sampling):")
    temp_groups = {}
    nucleus_results = method_groups.get("nucleus", [])
    for r in nucleus_results:
        temp = r["config"]["params"].get("temperature")
        if temp is not None:
            if temp not in temp_groups:
                temp_groups[temp] = []
            temp_groups[temp].append(r)

    for temp in sorted(temp_groups.keys()):
        results = temp_groups[temp]
        avg_diversity = np.mean([r["quality"]["lexical_diversity"] for r in results])
        avg_repetition = np.mean([r["quality"]["repetition_ratio"] for r in results])
        report.append(f"    Temp {temp}:  Diversity={avg_diversity:.3f}, Repetition={avg_repetition:.3f} ({len(results)} tests)")

    # Recommendations
    report.append("\n  Recommendations:")
    greedy_stats = method_groups.get("greedy", [])
    nucleus_stats = method_groups.get("nucleus", [])
    if greedy_stats and nucleus_stats:
        greedy_diversity = np.mean([r["quality"]["lexical_diversity"] for r in greedy_stats])
        nucleus_diversity = np.mean([r["quality"]["lexical_diversity"] for r in nucleus_stats])
        if nucleus_diversity > greedy_diversity * 1.1:
            report.append("    ✅ Nucleus sampling provides higher lexical diversity than greedy.")
        else:
            report.append("    ℹ️  Nucleus sampling and greedy methods show comparable diversity.")

    if temp_groups:
        low_temp = min(temp_groups.keys())
        high_temp = max(temp_groups.keys())
        low_temp_diversity = np.mean([r["quality"]["lexical_diversity"] for r in temp_groups[low_temp]])
        high_temp_diversity = np.mean([r["quality"]["lexical_diversity"] for r in temp_groups[high_temp]])
        if high_temp_diversity > low_temp_diversity:
            report.append("    ✅ Higher temperature tends to increase creativity (lexical diversity).")
        else:
            report.append("    ℹ️  Temperature shows a mixed impact on creativity in these tests.")

    return report

def run_standard_generation_tests(model, tokenizer):
    """Runs standard generation tests with comprehensive analysis and reporting."""
    report = ["\n--- Test 4: Standard Generation Configurations ---"]
    report.append(f"  Testing {len(TEST_CONFIGURATIONS)} different generation configurations.")

    all_results = []
    generated_texts_for_analysis = []
    all_results = []

    for i, config in enumerate(TEST_CONFIGURATIONS):
        report.append(f"\n  --- Config #{i+1} (Method: {config['method']}) ---")
        report.append(f"  Prompt: '{config['prompt']}'")
        
        start_time = time.time()
        generated_text = generate_with_model(
            model=model,
            tokenizer=tokenizer,
            prompt=config['prompt'],
            length=config['length'],
            method=config['method'],
            temperature=config['temperature'],
            top_k=config['top_k'],
            top_p=config['top_p']
        )
        end_time = time.time()
        
        analysis = analyze_generated_text(generated_text, config['prompt'], config['method'], config['length'])
        analysis["generation_time"] = end_time - start_time
        all_results.append(analysis)
        
        report.append(f"  Generated Text: '{generated_text[:150]}...'")
        report.append(f"  Generation Time: {analysis['generation_time']:.3f}s")
        report.append(f"  Word Count: {analysis['word_count']}, Lexical Diversity: {analysis['lexical_diversity']:.3f}, Repetition Ratio: {analysis['repetition_ratio']:.3f}")

        generated_texts_for_analysis.append({'config': config, 'text': generated_text})
    
    # Add performance analysis to the report
    report.extend(analyze_standard_generation_performance(all_results))
    
    return report, generated_texts_for_analysis
