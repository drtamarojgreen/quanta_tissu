import os
import sys
import argparse
import json
import numpy as np
import re
import time
import uuid
from datetime import datetime, timezone

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

# Updated imports to use the new generator and other required components
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config

# Import individual test modules
from quanta_tissu.tisslm.evaluation.full_fledged_workout01 import run_kv_cache_test, generate_with_model as generate_with_model_01
from quanta_tissu.tisslm.evaluation.full_fledged_workout02 import run_rag_and_self_update_test, generate_with_model as generate_with_model_02
from quanta_tissu.tisslm.evaluation.full_fledged_workout03 import run_experimental_sampling_tests, generate_with_model as generate_with_model_03
from quanta_tissu.tisslm.evaluation.full_fledged_workout04 import run_standard_generation_tests, generate_with_model as generate_with_model_04, TEST_CONFIGURATIONS
from quanta_tissu.tisslm.evaluation.full_fledged_workout05 import run_rule_enforcement_test, generate_with_model as generate_with_model_05
from quanta_tissu.tisslm.evaluation.full_fledged_workout06 import run_advanced_analysis_tests
from quanta_tissu.tisslm.evaluation.full_fledged_workout07 import run_advanced_analysis_tests as run_advanced_analysis_tests_07
from quanta_tissu.tisslm.evaluation.full_fledged_workout08 import run_tissdb_lite_tests
from quanta_tissu.tisslm.evaluation.full_fledged_workout09 import run_tisslang_parser_tests
from quanta_tissu.tisslm.evaluation.full_fledged_workout10 import run_advanced_sampling_tests
from quanta_tissu.tisslm.evaluation.full_fledged_workout11 import run_knowledge_retrieval_tests

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

# --- Expanded Evaluation Configuration ---
EXPANDED_GENERATION_PARAMETERS = [
    # Temperature variations
    {"temperature": 0.1, "top_p": 0.9, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 0.5, "top_p": 0.9, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 0.8, "top_p": 0.9, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 1.2, "top_p": 0.9, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 1.5, "top_p": 0.9, "repetition_penalty": 1.0, "method": "nucleus"},
    
    # Top-p variations
    {"temperature": 0.8, "top_p": 0.5, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 0.8, "top_p": 0.7, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 0.8, "top_p": 0.95, "repetition_penalty": 1.0, "method": "nucleus"},
    {"temperature": 0.8, "top_p": 0.99, "repetition_penalty": 1.0, "method": "nucleus"},
    
    # Repetition penalty variations
    {"temperature": 0.8, "top_p": 0.9, "repetition_penalty": 1.1, "method": "nucleus"},
    {"temperature": 0.8, "top_p": 0.9, "repetition_penalty": 1.3, "method": "nucleus"},
    {"temperature": 0.8, "top_p": 0.9, "repetition_penalty": 1.5, "method": "nucleus"},
    
    # Top-k variations
    {"temperature": 0.8, "top_k": 10, "repetition_penalty": 1.0, "method": "top_k"},
    {"temperature": 0.8, "top_k": 25, "repetition_penalty": 1.0, "method": "top_k"},
    {"temperature": 0.8, "top_k": 50, "repetition_penalty": 1.0, "method": "top_k"},
    {"temperature": 0.8, "top_k": 100, "repetition_penalty": 1.0, "method": "top_k"},
    
    # Greedy baseline
    {"temperature": 1.0, "repetition_penalty": 1.0, "method": "greedy"},
]

DIVERSE_PROMPT_SETS = {
    "short_completion": [
        "The weather today is",
        "My favorite color is",
        "The capital of France is",
        "In the morning, I usually",
        "The best way to learn is"
    ],
    "medium_narrative": [
        "Once upon a time in a distant galaxy, there lived a young explorer who discovered",
        "The old lighthouse keeper had been warning ships for decades, but tonight something was different",
        "As the scientist examined the mysterious artifact, she realized it contained",
        "The detective walked into the abandoned mansion and immediately noticed",
        "In the year 2150, humanity had finally achieved interstellar travel, but the first expedition revealed"
    ],
    "extended_analysis": [
        "The implications of artificial intelligence on modern society are complex and multifaceted. Consider the following aspects:",
        "Climate change represents one of the most significant challenges of our time. To understand its full impact, we must examine:",
        "The evolution of human language demonstrates fascinating patterns across cultures and time periods. Key observations include:",
        "Modern healthcare systems face unprecedented challenges that require innovative solutions. The primary issues to address are:",
        "The relationship between technology and human creativity has evolved dramatically over the past century. This transformation can be understood through:"
    ],
    "technical_explanation": [
        "To implement a neural network from scratch, one must understand the fundamental components:",
        "The process of photosynthesis involves several complex biochemical reactions that can be broken down into:",
        "Quantum computing differs from classical computing in several key ways, particularly in how it handles:",
        "The architecture of modern web applications typically consists of multiple layers, each serving specific functions:",
        "Machine learning algorithms can be categorized into different types based on their learning approach:"
    ],
    "creative_writing": [
        "The dragon's eyes gleamed in the moonlight as it surveyed the sleeping village below. For centuries, it had watched over these people, but tonight",
        "She opened the letter with trembling hands, knowing that whatever it contained would change her life forever. The words on the page revealed",
        "The time traveler stepped out of the machine and into a world that was both familiar and utterly alien. The first thing that struck him was",
        "In the depths of the ocean, where sunlight never reaches, an ancient civilization thrived in ways that surface dwellers could never imagine",
        "The last human on Earth sat in the empty library, surrounded by the accumulated knowledge of millennia, and began to write"
    ]
}

TOKEN_LENGTH_TESTS = [25, 50, 100, 200, 400, 800, 1200]  # Varying max token lengths

def run_expanded_parameter_evaluation(model, tokenizer):
    """Run comprehensive evaluation across different generation parameters."""
    report = ["\n--- EXPANDED PARAMETER EVALUATION ---"]
    report.append(f"Testing {len(EXPANDED_GENERATION_PARAMETERS)} parameter configurations")
    report.append(f"Across {sum(len(prompts) for prompts in DIVERSE_PROMPT_SETS.values())} diverse prompts")
    report.append(f"With {len(TOKEN_LENGTH_TESTS)} different token lengths")
    
    evaluation_results = []
    total_tests = 0
    successful_tests = 0
    
    for param_idx, params in enumerate(EXPANDED_GENERATION_PARAMETERS):
        report.append(f"\n  Parameter Set #{param_idx + 1}: {params}")
        
        for prompt_category, prompts in DIVERSE_PROMPT_SETS.items():
            for prompt_idx, prompt in enumerate(prompts):
                for length in TOKEN_LENGTH_TESTS:
                    total_tests += 1
                    
                    try:
                        # Use the appropriate generation method based on available callables
                        if hasattr(model, 'generator') and hasattr(model.generator, 'sample'):
                            # Use AlgorithmicGenerator
                            prompt_tokens = tokenizer.tokenize(prompt).tolist()
                            generated_tokens = model.generator.sample(
                                prompt_tokens=prompt_tokens,
                                n_new_tokens=length,
                                tokenizer=tokenizer,
                                **params
                            )
                            generated_text = tokenizer.detokenize(np.array(generated_tokens))
                        elif hasattr(model, 'sample'):
                            # Use model's sample method
                            prompt_tokens = tokenizer.tokenize(prompt).tolist()
                            generated_tokens = model.sample(
                                prompt_tokens,
                                n_new_tokens=length,
                                **params
                            )
                            generated_text = tokenizer.detokenize(np.array(generated_tokens))
                        else:
                            # Fallback to basic generation
                            from quanta_tissu.tisslm.core.generate_text import generate_text
                            generated_text = generate_text(
                                model, tokenizer, prompt, length,
                                params.get('method', 'nucleus'),
                                params.get('temperature', 0.8),
                                params.get('top_k', 50),
                                params.get('top_p', 0.9),
                                params.get('repetition_penalty', 1.0)
                            )
                        
                        # Analyze the generated text
                        analysis = analyze_generation_quality(prompt, generated_text, length, params)
                        evaluation_results.append({
                            'param_set': param_idx,
                            'prompt_category': prompt_category,
                            'prompt_idx': prompt_idx,
                            'target_length': length,
                            'actual_length': len(generated_text.split()),
                            'parameters': params,
                            'analysis': analysis,
                            'generated_text': generated_text[:200] + "..." if len(generated_text) > 200 else generated_text
                        })
                        
                        successful_tests += 1
                        
                        # Log progress for longer tests
                        if total_tests % 50 == 0:
                            report.append(f"    Progress: {total_tests} tests completed")
                            
                    except Exception as e:
                        report.append(f"    ERROR in test {total_tests}: {str(e)}")
                        evaluation_results.append({
                            'param_set': param_idx,
                            'prompt_category': prompt_category,
                            'prompt_idx': prompt_idx,
                            'target_length': length,
                            'error': str(e),
                            'parameters': params
                        })
    
    # Generate summary statistics
    report.extend(generate_evaluation_summary(evaluation_results, total_tests, successful_tests))
    
    return report, evaluation_results

def analyze_generation_quality(prompt, generated_text, target_length, params):
    """Analyze the quality and characteristics of generated text."""
    analysis = {}
    
    # Basic metrics
    words = generated_text.split()
    analysis['word_count'] = len(words)
    analysis['char_count'] = len(generated_text)
    analysis['avg_word_length'] = np.mean([len(word) for word in words]) if words else 0
    
    # Repetition analysis
    word_counts = {}
    for word in words:
        word_counts[word] = word_counts.get(word, 0) + 1
    
    repeated_words = {word: count for word, count in word_counts.items() if count > 1}
    analysis['repetition_ratio'] = len(repeated_words) / len(words) if words else 0
    analysis['max_word_repetition'] = max(word_counts.values()) if word_counts else 0
    
    # Diversity metrics
    analysis['unique_words'] = len(set(words))
    analysis['lexical_diversity'] = len(set(words)) / len(words) if words else 0
    
    # Coherence indicators
    sentences = re.split(r'[.!?]+', generated_text)
    analysis['sentence_count'] = len([s for s in sentences if s.strip()])
    analysis['avg_sentence_length'] = analysis['word_count'] / analysis['sentence_count'] if analysis['sentence_count'] > 0 else 0
    
    # Parameter effectiveness
    analysis['length_accuracy'] = min(analysis['word_count'] / target_length, 1.0) if target_length > 0 else 0
    
    return analysis

def generate_evaluation_summary(results, total_tests, successful_tests):
    """Generate comprehensive summary of evaluation results."""
    summary = [f"\n--- EVALUATION SUMMARY ---"]
    summary.append(f"Total tests: {total_tests}")
    summary.append(f"Successful tests: {successful_tests}")
    summary.append(f"Success rate: {successful_tests/total_tests*100:.1f}%")
    
    if not results:
        summary.append("No results to analyze.")
        return summary
    
    # Filter successful results
    successful_results = [r for r in results if 'analysis' in r]
    
    if not successful_results:
        summary.append("No successful generations to analyze.")
        return summary
    
    # Parameter performance analysis
    param_performance = {}
    for result in successful_results:
        param_key = str(result['parameters'])
        if param_key not in param_performance:
            param_performance[param_key] = []
        param_performance[param_key].append(result['analysis'])
    
    summary.append(f"\n  Parameter Performance Analysis:")
    for param_key, analyses in param_performance.items():
        avg_diversity = np.mean([a['lexical_diversity'] for a in analyses])
        avg_repetition = np.mean([a['repetition_ratio'] for a in analyses])
        avg_length_accuracy = np.mean([a['length_accuracy'] for a in analyses])
        
        summary.append(f"    {param_key[:80]}...")
        summary.append(f"      Avg Lexical Diversity: {avg_diversity:.3f}")
        summary.append(f"      Avg Repetition Ratio: {avg_repetition:.3f}")
        summary.append(f"      Avg Length Accuracy: {avg_length_accuracy:.3f}")
    
    # Prompt category analysis
    category_performance = {}
    for result in successful_results:
        category = result['prompt_category']
        if category not in category_performance:
            category_performance[category] = []
        category_performance[category].append(result['analysis'])
    
    summary.append(f"\n  Prompt Category Performance:")
    for category, analyses in category_performance.items():
        avg_diversity = np.mean([a['lexical_diversity'] for a in analyses])
        avg_word_count = np.mean([a['word_count'] for a in analyses])
        
        summary.append(f"    {category}: Avg Diversity={avg_diversity:.3f}, Avg Words={avg_word_count:.1f}")
    
    # Length scaling analysis
    length_performance = {}
    for result in successful_results:
        target_length = result['target_length']
        if target_length not in length_performance:
            length_performance[target_length] = []
        length_performance[target_length].append(result['analysis'])
    
    summary.append(f"\n  Length Scaling Performance:")
    for target_length in sorted(length_performance.keys()):
        analyses = length_performance[target_length]
        avg_accuracy = np.mean([a['length_accuracy'] for a in analyses])
        avg_actual_words = np.mean([a['word_count'] for a in analyses])
        
        summary.append(f"    Target {target_length} tokens: Accuracy={avg_accuracy:.3f}, Avg Actual={avg_actual_words:.1f} words")
    
    return summary

def run_generative_callable_discovery(model, tokenizer):
    """Discover and test all available generative callables in the model."""
    report = ["\n--- GENERATIVE CALLABLE DISCOVERY ---"]
    
    discovered_methods = []
    
    # Check model methods
    if hasattr(model, 'sample'):
        discovered_methods.append(('model.sample', model.sample))
        report.append("  ✓ Found model.sample method")
    
    if hasattr(model, 'generate'):
        discovered_methods.append(('model.generate', model.generate))
        report.append("  ✓ Found model.generate method")
    
    # Check generator methods
    if hasattr(model, 'generator'):
        if hasattr(model.generator, 'sample'):
            discovered_methods.append(('model.generator.sample', model.generator.sample))
            report.append("  ✓ Found model.generator.sample method")
        
        if hasattr(model.generator, 'iterative_sampling'):
            discovered_methods.append(('model.generator.iterative_sampling', model.generator.iterative_sampling))
            report.append("  ✓ Found model.generator.iterative_sampling method")
        
        if hasattr(model.generator, 'dynamic_token_revision_sampling'):
            discovered_methods.append(('model.generator.dynamic_token_revision_sampling', model.generator.dynamic_token_revision_sampling))
            report.append("  ✓ Found model.generator.dynamic_token_revision_sampling method")
        
        if hasattr(model.generator, 'bayesian_word_expansion_sampling'):
            discovered_methods.append(('model.generator.bayesian_word_expansion_sampling', model.generator.bayesian_word_expansion_sampling))
            report.append("  ✓ Found model.generator.bayesian_word_expansion_sampling method")
        
        if hasattr(model.generator, 'adaptive_sentiment_sampling'):
            discovered_methods.append(('model.generator.adaptive_sentiment_sampling', model.generator.adaptive_sentiment_sampling))
            report.append("  ✓ Found model.generator.adaptive_sentiment_sampling method")
    
    report.append(f"\n  Total discovered methods: {len(discovered_methods)}")
    
    # Test each discovered method with a simple prompt
    test_prompt = "The future of artificial intelligence"
    test_length = 50
    
    for method_name, method_func in discovered_methods:
        try:
            report.append(f"\n  Testing {method_name}:")
            
            if 'model.generator.' in method_name:
                # Generator methods need specific parameters
                prompt_tokens = tokenizer.tokenize(test_prompt).tolist()
                
                if 'dynamic_token_revision' in method_name:
                    result = method_func(prompt_tokens, test_length, underlying_method='nucleus', temperature=0.8, top_p=0.9)
                elif 'bayesian_word_expansion' in method_name:
                    result = method_func(prompt_tokens, test_length, tokenizer=tokenizer, temperature=0.8, top_p=0.9)
                elif 'adaptive_sentiment' in method_name:
                    # Skip this one as it requires sentiment analyzer
                    report.append("    [SKIPPED] Requires sentiment analyzer")
                    continue
                else:
                    result = method_func(prompt_tokens, test_length, method='nucleus', temperature=0.8, top_p=0.9, tokenizer=tokenizer)
                
                if isinstance(result, list):
                    generated_text = tokenizer.detokenize(np.array(result))
                else:
                    generated_text = str(result)
            
            elif method_name == 'model.sample':
                prompt_tokens = tokenizer.tokenize(test_prompt).tolist()
                result = method_func(prompt_tokens, n_new_tokens=test_length, method='nucleus', temperature=0.8, top_p=0.9)
                generated_text = tokenizer.detokenize(np.array(result))
            
            else:
                # Generic method call
                result = method_func(test_prompt, test_length)
                generated_text = str(result)
            
            report.append(f"    ✓ Success: '{generated_text[:100]}...'")
            
        except Exception as e:
            report.append(f"    ✗ Error: {str(e)}")
    
    return report

def save_evaluation_results(results, output_dir="evaluation_logs"):
    """Save detailed evaluation results to files."""
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    # Save detailed results as JSON
    results_file = os.path.join(output_dir, f"expanded_evaluation_{timestamp}.json")
    with open(results_file, 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2, ensure_ascii=False)
    
    # Save summary statistics
    summary_file = os.path.join(output_dir, f"evaluation_summary_{timestamp}.txt")
    with open(summary_file, 'w', encoding='utf-8') as f:
        f.write("EXPANDED EVALUATION SUMMARY\n")
        f.write("=" * 50 + "\n\n")
        
        successful_results = [r for r in results if 'analysis' in r]
        if successful_results:
            f.write(f"Total successful tests: {len(successful_results)}\n")
            
            # Overall statistics
            avg_diversity = np.mean([r['analysis']['lexical_diversity'] for r in successful_results])
            avg_repetition = np.mean([r['analysis']['repetition_ratio'] for r in successful_results])
            avg_length_accuracy = np.mean([r['analysis']['length_accuracy'] for r in successful_results])
            
            f.write(f"Average lexical diversity: {avg_diversity:.3f}\n")
            f.write(f"Average repetition ratio: {avg_repetition:.3f}\n")
            f.write(f"Average length accuracy: {avg_length_accuracy:.3f}\n")
    
    return results_file, summary_file

def main():
    """Main function to orchestrate the evaluation workout."""
    parser = argparse.ArgumentParser(description="Run a comprehensive evaluation workout on the TissLM model.")
    parser.add_argument('--test', type=str, help='Specify which test to run (e.g., "1" through "11", "all", "expanded").')
    parser.add_argument('--save-results', action='store_true', help='Save detailed evaluation results to files.')
    args = parser.parse_args()

    full_report = []

    # --- Load Tokenizer and Model ---
    try:
        tokenizer = Tokenizer(tokenizer_prefix=TOKENIZER_SAVE_PREFIX)
        model_config["vocab_size"] = tokenizer.get_vocab_size()
        # Pass project root to config for file path resolution in AlgorithmicGenerator
        model_config["_project_root"] = project_root
        model = QuantaTissu(model_config)
        model.load_weights(FINAL_CHECKPOINT_PATH)
        full_report.append("Tokenizer and model loaded successfully.")
    except Exception as e:
        full_report.append(f"FATAL: Error loading tokenizer or model: {e}")
        print("\n".join(full_report))
        sys.exit(1)

    # --- Run Test Suites ---
    evaluation_results = []
    
    if args.test == "expanded" or args.test == "all":
        # Run expanded parameter evaluation
        expanded_report, expanded_results = run_expanded_parameter_evaluation(model, tokenizer)
        full_report.extend(expanded_report)
        evaluation_results.extend(expanded_results)
        
        # Run generative callable discovery
        discovery_report = run_generative_callable_discovery(model, tokenizer)
        full_report.extend(discovery_report)

    if args.test == "1" or args.test == "all":
        full_report.extend(run_kv_cache_test(model, tokenizer))

    if args.test == "2" or args.test == "all":
        full_report.extend(run_rag_and_self_update_test(model, tokenizer))

    if args.test == "3" or args.test == "all":
        full_report.extend(run_experimental_sampling_tests(model, tokenizer))

    if args.test == "4" or args.test == "all":
        standard_gen_report, generated_texts = run_standard_generation_tests(model, tokenizer)
        full_report.extend(standard_gen_report)
    else:
        generated_texts = [] # Initialize if test 4 is not run

    if args.test == "5" or args.test == "all":
        full_report.extend(run_rule_enforcement_test(model, tokenizer))

    if args.test == "6" or args.test == "all":
        if generated_texts: # Only run if generated_texts is available from test 4
            full_report.extend(run_advanced_analysis_tests(generated_texts))
        else:
            full_report.append("\n--- Test 6: Advanced Text Analysis (Grammar, Lexicon, Context) ---")
            full_report.append("  [SKIPPED] Test 6 skipped because Test 4 (Standard Generation) was not run.")

    if args.test == "7" or args.test == "all":
        if generated_texts: # Only run if generated_texts is available from test 4
            full_report.extend(run_advanced_analysis_tests_07(generated_texts))
        else:
            full_report.append("\n--- Test 7: Advanced Text Analysis (Grammar, Lexicon, Context) ---")
            full_report.append("  [SKIPPED] Test 7 skipped because Test 4 (Standard Generation) was not run.")

    if args.test == "8" or args.test == "all":
        full_report.extend(run_tissdb_lite_tests())

    if args.test == "9" or args.test == "all":
        full_report.extend(run_tisslang_parser_tests())

    if args.test == "10" or args.test == "all":
        # Assuming a draft model would be loaded here if available/needed
        full_report.extend(run_advanced_sampling_tests(model, tokenizer))

    if args.test == "11" or args.test == "all":
        full_report.extend(run_knowledge_retrieval_tests(model, tokenizer))

    # Save results if requested
    if args.save_results and evaluation_results:
        try:
            results_file, summary_file = save_evaluation_results(evaluation_results)
            full_report.append(f"\n--- Results saved to: {results_file} ---")
            full_report.append(f"--- Summary saved to: {summary_file} ---")
        except Exception as e:
            full_report.append(f"Error saving results: {e}")

    # --- Final Report ---
    print("\n" + "="*80)
    print("COMPREHENSIVE EXPERIMENTAL FEATURES WORKOUT REPORT")
    print("="*80)
    print("\n".join(full_report))
    print("\n" + "="*80)

if __name__ == "__main__":
    main()
