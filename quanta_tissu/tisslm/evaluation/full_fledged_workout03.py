import os
import sys
import numpy as np
import re
import json
import time
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

# --- Expanded Test Configurations ---
DYNAMIC_TOKEN_REVISION_TESTS = [
    {"prompt": "Testing dynamic token revision with", "length": 30, "save_interval": 10, "underlying_method": "greedy"},
    {"prompt": "The future of technology will", "length": 50, "save_interval": 15, "underlying_method": "nucleus"},
    {"prompt": "In a world where artificial intelligence", "length": 75, "save_interval": 20, "underlying_method": "nucleus"},
]

BAYESIAN_WORD_EXPANSION_TESTS = [
    {"prompt": "Testing word expansion capabilities", "length": 40, "expansion_threshold": 0.5},
    {"prompt": "The mysterious phenomenon occurred", "length": 60, "expansion_threshold": 0.7},
    {"prompt": "Scientific research demonstrates", "length": 80, "expansion_threshold": 0.6},
]

ADAPTIVE_SENTIMENT_TESTS = [
    {"sentiment": "positive", "strength": 0.8, "prompt": "I feel very", "length": 30},
    {"sentiment": "negative", "strength": 0.8, "prompt": "This is a very", "length": 30},
    {"sentiment": "neutral", "strength": 0.5, "prompt": "The situation is", "length": 30},
    {"sentiment": "positive", "strength": 0.6, "prompt": "The weather today brings", "length": 50},
    {"sentiment": "negative", "strength": 0.9, "prompt": "The economic crisis has", "length": 50},
]

# --- Helper function for text generation ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    if method == "nucleus" and "top_p" not in kwargs:
        kwargs["top_p"] = 0.9
    prompt_tokens = tokenizer.tokenize(prompt).tolist()

    if method in ["dynamic_token_revision", "bayesian_word_expansion", "adaptive_sentiment"]:
        generated_tokens = model.generator.sample(
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
        
        # Repetition analysis
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

def run_dynamic_token_revision_tests(model, tokenizer):
    """Enhanced testing of dynamic token revision sampling."""
    report = ["\n  --- Test 3a: Enhanced Dynamic Token Revision ---"]
    report.append(f"  Running {len(DYNAMIC_TOKEN_REVISION_TESTS)} test scenarios")
    
    all_results = []
    
    for test_idx, test_config in enumerate(DYNAMIC_TOKEN_REVISION_TESTS):
        report.append(f"\n    Test {test_idx + 1}: {test_config['underlying_method']} method")
        report.append(f"    Prompt: '{test_config['prompt']}'")
        report.append(f"    Length: {test_config['length']}, Save interval: {test_config['save_interval']}")
        
        try:
            # Clear the temp list file before the test
            with open(TEMP_LIST_PATH, "w") as f:
                f.write("")
            
            start_time = time.time()
            generated_text = generate_with_model(
                model, tokenizer,
                prompt=test_config['prompt'],
                length=test_config['length'],
                method="dynamic_token_revision",
                underlying_method=test_config['underlying_method'],
                save_interval=test_config['save_interval'],
                temperature=0.8,
                top_p=0.9
            )
            end_time = time.time()
            
            # Analyze the generated text
            analysis = analyze_generated_text(
                generated_text, 
                test_config['prompt'], 
                "dynamic_token_revision", 
                test_config['length']
            )
            analysis["generation_time"] = end_time - start_time
            analysis["save_interval"] = test_config['save_interval']
            analysis["underlying_method"] = test_config['underlying_method']
            
            all_results.append(analysis)
            
            report.append(f"    Generated: '{generated_text[:80]}...'")
            report.append(f"    Generation time: {analysis['generation_time']:.3f}s")
            report.append(f"    Word count: {analysis['word_count']}, Lexical diversity: {analysis['lexical_diversity']:.3f}")
            
            # Verification - check temp file was updated
            with open(TEMP_LIST_PATH, "r") as f:
                temp_list_content = f.read()
                if len(temp_list_content) > 0:
                    report.append(f"    ✓ Temp list file updated ({len(temp_list_content)} chars)")
                    analysis["temp_file_updated"] = True
                else:
                    report.append(f"    ✗ Temp list file is empty")
                    analysis["temp_file_updated"] = False

        except Exception as e:
            report.append(f"    [ERROR] Dynamic Token Revision test {test_idx + 1} failed: {e}")
            all_results.append({
                "method": "dynamic_token_revision",
                "error": str(e),
                "test_config": test_config
            })
    
    # Summary analysis
    successful_tests = [r for r in all_results if "error" not in r]
    if successful_tests:
        avg_diversity = np.mean([r["lexical_diversity"] for r in successful_tests])
        avg_time = np.mean([r["generation_time"] for r in successful_tests])
        temp_file_success_rate = np.mean([r.get("temp_file_updated", False) for r in successful_tests])
        
        report.append(f"\n    Summary - Dynamic Token Revision:")
        report.append(f"    Successful tests: {len(successful_tests)}/{len(DYNAMIC_TOKEN_REVISION_TESTS)}")
        report.append(f"    Average lexical diversity: {avg_diversity:.3f}")
        report.append(f"    Average generation time: {avg_time:.3f}s")
        report.append(f"    Temp file update success rate: {temp_file_success_rate:.1%}")
    
    return report, all_results

def run_bayesian_word_expansion_tests(model, tokenizer):
    """Enhanced testing of Bayesian word expansion sampling."""
    report = ["\n  --- Test 3b: Enhanced Bayesian Word Expansion ---"]
    report.append(f"  Running {len(BAYESIAN_WORD_EXPANSION_TESTS)} test scenarios")
    
    all_results = []
    
    # Backup original wordlist
    original_wordlist = ""
    if os.path.exists(WORDLIST_PATH):
        with open(WORDLIST_PATH, "r") as f:
            original_wordlist = f.read()
    
    for test_idx, test_config in enumerate(BAYESIAN_WORD_EXPANSION_TESTS):
        report.append(f"\n    Test {test_idx + 1}: Expansion threshold {test_config['expansion_threshold']}")
        report.append(f"    Prompt: '{test_config['prompt']}'")
        report.append(f"    Length: {test_config['length']}")
        
        try:
            # Reset wordlist for each test
            with open(WORDLIST_PATH, "w") as f:
                f.write("hello world test example")
            
            # Create mock data required by the method
            n_embd = model.config.get('n_embd', 128)
            mock_query_embedding = np.random.rand(n_embd).astype(np.float32)
            mock_hessian_matrix = np.random.rand(n_embd, n_embd).astype(np.float32)
            
            start_time = time.time()
            generated_text = generate_with_model(
                model, tokenizer,
                prompt=test_config['prompt'],
                length=test_config['length'],
                method="bayesian_word_expansion",
                query_embedding=mock_query_embedding,
                hessian_matrix=mock_hessian_matrix,
                expansion_threshold=test_config['expansion_threshold'],
                temperature=0.8,
                top_p=0.9
            )
            end_time = time.time()
            
            # Analyze the generated text
            analysis = analyze_generated_text(
                generated_text, 
                test_config['prompt'], 
                "bayesian_word_expansion", 
                test_config['length']
            )
            analysis["generation_time"] = end_time - start_time
            analysis["expansion_threshold"] = test_config['expansion_threshold']
            
            # Check wordlist expansion
            with open(WORDLIST_PATH, "r") as f:
                new_wordlist_content = f.read()
                words_added = len(new_wordlist_content.split()) - 4  # Original had 4 words
                analysis["words_added"] = max(0, words_added)
                analysis["wordlist_expanded"] = words_added > 0
            
            all_results.append(analysis)
            
            report.append(f"    Generated: '{generated_text[:80]}...'")
            report.append(f"    Generation time: {analysis['generation_time']:.3f}s")
            report.append(f"    Word count: {analysis['word_count']}, Lexical diversity: {analysis['lexical_diversity']:.3f}")
            report.append(f"    Words added to wordlist: {analysis['words_added']}")

        except Exception as e:
            report.append(f"    [ERROR] Bayesian Word Expansion test {test_idx + 1} failed: {e}")
            all_results.append({
                "method": "bayesian_word_expansion",
                "error": str(e),
                "test_config": test_config
            })
    
    # Restore original wordlist
    with open(WORDLIST_PATH, "w") as f:
        f.write(original_wordlist)
    
    # Summary analysis
    successful_tests = [r for r in all_results if "error" not in r]
    if successful_tests:
        avg_diversity = np.mean([r["lexical_diversity"] for r in successful_tests])
        avg_time = np.mean([r["generation_time"] for r in successful_tests])
        expansion_success_rate = np.mean([r.get("wordlist_expanded", False) for r in successful_tests])
        total_words_added = sum([r.get("words_added", 0) for r in successful_tests])
        
        report.append(f"\n    Summary - Bayesian Word Expansion:")
        report.append(f"    Successful tests: {len(successful_tests)}/{len(BAYESIAN_WORD_EXPANSION_TESTS)}")
        report.append(f"    Average lexical diversity: {avg_diversity:.3f}")
        report.append(f"    Average generation time: {avg_time:.3f}s")
        report.append(f"    Wordlist expansion success rate: {expansion_success_rate:.1%}")
        report.append(f"    Total words added across tests: {total_words_added}")
    
    return report, all_results

def run_adaptive_sentiment_tests(model, tokenizer):
    """Enhanced testing of adaptive sentiment sampling."""
    report = ["\n  --- Test 3c: Enhanced Adaptive Sentiment Sampling ---"]
    
    # Check for corpus sentiments file and generate if missing
    if not os.path.exists(CORPUS_SENTIMENTS_PATH):
        report.append(f"  [INFO] Sentiments file not found at {CORPUS_SENTIMENTS_PATH}.")
        report.append("  Attempting to generate it automatically...")
        try:
            create_corpus_sentiments_main()
            report.append("  Sentiments file generated successfully.")
        except Exception as e:
            report.append(f"  [ERROR] Failed to generate sentiments file: {e}")
            report.append("  Skipping Adaptive Sentiment Sampling test.")
            return report, []
    
    try:
        sentiment_analyzer = SentimentAnalyzer(tokenizer=tokenizer, sentiment_lexicon_path=CORPUS_SENTIMENTS_PATH)
        
        if not sentiment_analyzer.sentiment_scores:
            report.append(f"  [SKIPPED] Adaptive Sentiment Sampling: Sentiments file is empty.")
            return report, []
        
        report.append(f"  Loaded sentiment analyzer with {len(sentiment_analyzer.sentiment_scores)} sentiment words")
        report.append(f"  Running {len(ADAPTIVE_SENTIMENT_TESTS)} test scenarios")
        
    except Exception as e:
        report.append(f"  [ERROR] Failed to initialize sentiment analyzer: {e}")
        return report, []
    
    all_results = []
    
    for test_idx, test_config in enumerate(ADAPTIVE_SENTIMENT_TESTS):
        report.append(f"\n    Test {test_idx + 1}: {test_config['sentiment']} sentiment (strength: {test_config['strength']})")
        report.append(f"    Prompt: '{test_config['prompt']}'")
        report.append(f"    Length: {test_config['length']}")
        
        try:
            start_time = time.time()
            generated_text = generate_with_model(
                model, tokenizer,
                prompt=test_config['prompt'],
                length=test_config['length'],
                method="adaptive_sentiment",
                sentiment_analyzer=sentiment_analyzer,
                target_sentiment=test_config['sentiment'],
                target_strength=test_config['strength'],
                evaluation_interval=10,
                underlying_method="nucleus",
                temperature=0.8,
                top_p=0.9
            )
            end_time = time.time()
            
            # Analyze the generated text
            analysis = analyze_generated_text(
                generated_text, 
                test_config['prompt'], 
                "adaptive_sentiment", 
                test_config['length']
            )
            analysis["generation_time"] = end_time - start_time
            analysis["target_sentiment"] = test_config['sentiment']
            analysis["target_strength"] = test_config['strength']
            
            # Analyze sentiment of generated text
            words_in_text = re.findall(r'\b\w+\b', generated_text.lower())
            sentiment_sum = 0.0
            sentiment_word_count = 0
            
            for word in words_in_text:
                if word in sentiment_analyzer.sentiment_scores:
                    sentiment_sum += sentiment_analyzer.sentiment_scores[word]
                    sentiment_word_count += 1
            
            if sentiment_word_count > 0:
                analysis["actual_sentiment_score"] = sentiment_sum / sentiment_word_count
                analysis["sentiment_words_found"] = sentiment_word_count
            else:
                analysis["actual_sentiment_score"] = 0.0
                analysis["sentiment_words_found"] = 0
            
            # Determine if sentiment target was achieved
            target_score = 0.0
            if test_config['sentiment'] == 'positive':
                target_score = test_config['strength']
            elif test_config['sentiment'] == 'negative':
                target_score = -test_config['strength']
            
            analysis["target_sentiment_score"] = target_score
            analysis["sentiment_accuracy"] = 1.0 - abs(analysis["actual_sentiment_score"] - target_score)
            
            all_results.append(analysis)
            
            report.append(f"    Generated: '{generated_text[:80]}...'")
            report.append(f"    Generation time: {analysis['generation_time']:.3f}s")
            report.append(f"    Target sentiment score: {target_score:.2f}, Actual: {analysis['actual_sentiment_score']:.2f}")
            report.append(f"    Sentiment words found: {analysis['sentiment_words_found']}")
            report.append(f"    Sentiment accuracy: {analysis['sentiment_accuracy']:.3f}")

        except Exception as e:
            report.append(f"    [ERROR] Adaptive Sentiment test {test_idx + 1} failed: {e}")
            all_results.append({
                "method": "adaptive_sentiment",
                "error": str(e),
                "test_config": test_config
            })
    
    # Summary analysis
    successful_tests = [r for r in all_results if "error" not in r]
    if successful_tests:
        avg_diversity = np.mean([r["lexical_diversity"] for r in successful_tests])
        avg_time = np.mean([r["generation_time"] for r in successful_tests])
        avg_sentiment_accuracy = np.mean([r.get("sentiment_accuracy", 0) for r in successful_tests])
        avg_sentiment_words = np.mean([r.get("sentiment_words_found", 0) for r in successful_tests])
        
        # Analyze by sentiment type
        sentiment_performance = {}
        for sentiment in ["positive", "negative", "neutral"]:
            sentiment_tests = [r for r in successful_tests if r.get("target_sentiment") == sentiment]
            if sentiment_tests:
                sentiment_performance[sentiment] = {
                    "count": len(sentiment_tests),
                    "avg_accuracy": np.mean([r["sentiment_accuracy"] for r in sentiment_tests]),
                    "avg_words_found": np.mean([r["sentiment_words_found"] for r in sentiment_tests])
                }
        
        report.append(f"\n    Summary - Adaptive Sentiment Sampling:")
        report.append(f"    Successful tests: {len(successful_tests)}/{len(ADAPTIVE_SENTIMENT_TESTS)}")
        report.append(f"    Average lexical diversity: {avg_diversity:.3f}")
        report.append(f"    Average generation time: {avg_time:.3f}s")
        report.append(f"    Average sentiment accuracy: {avg_sentiment_accuracy:.3f}")
        report.append(f"    Average sentiment words per text: {avg_sentiment_words:.1f}")
        
        for sentiment, perf in sentiment_performance.items():
            report.append(f"    {sentiment.capitalize()} sentiment: {perf['avg_accuracy']:.3f} accuracy ({perf['count']} tests)")
    
    return report, all_results

def run_experimental_sampling_tests(model, tokenizer):
    """Evaluates experimental sampling methods with comprehensive analysis."""
    report = ["\n--- Test 3: Enhanced Experimental Sampling Methods ---"]
    
    all_results = {
        "dynamic_token_revision": [],
        "bayesian_word_expansion": [],
        "adaptive_sentiment": []
    }
    
    # Run Dynamic Token Revision tests
    dtr_report, dtr_results = run_dynamic_token_revision_tests(model, tokenizer)
    report.extend(dtr_report)
    all_results["dynamic_token_revision"] = dtr_results
    
    # Run Bayesian Word Expansion tests
    bwe_report, bwe_results = run_bayesian_word_expansion_tests(model, tokenizer)
    report.extend(bwe_report)
    all_results["bayesian_word_expansion"] = bwe_results
    
    # Run Adaptive Sentiment tests
    as_report, as_results = run_adaptive_sentiment_tests(model, tokenizer)
    report.extend(as_report)
    all_results["adaptive_sentiment"] = as_results
    
    # Overall summary
    report.append(f"\n--- Experimental Sampling Methods Summary ---")
    
    total_tests = sum(len(results) for results in all_results.values())
    successful_tests = sum(len([r for r in results if "error" not in r]) for results in all_results.values())
    
    report.append(f"  Total experimental tests: {total_tests}")
    report.append(f"  Successful tests: {successful_tests}")
    report.append(f"  Overall success rate: {successful_tests/total_tests:.1%}" if total_tests > 0 else "  No tests completed")
    
    # Method-specific summaries
    for method, results in all_results.items():
        successful = [r for r in results if "error" not in r]
        if successful:
            avg_diversity = np.mean([r.get("lexical_diversity", 0) for r in successful])
            avg_time = np.mean([r.get("generation_time", 0) for r in successful])
            report.append(f"  {method.replace('_', ' ').title()}: {len(successful)}/{len(results)} success, "
                         f"avg diversity: {avg_diversity:.3f}, avg time: {avg_time:.3f}s")
    
    return report
