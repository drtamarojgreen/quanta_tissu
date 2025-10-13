import os
import sys
import numpy as np
import re
import time
from collections import Counter

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.rules.enforcer import RuleEnforcer

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")

# --- Expanded Test Configurations ---
RULE_ENFORCEMENT_TEST_CASES = [
    {
        "name": "Basic repetition and punctuation",
        "text": "this is a test. it has some repeated words. words words. and missing punctuation",
        "expected_issues": ["repetition", "punctuation"],
        "description": "Simple case with word repetition and punctuation issues"
    },
    {
        "name": "Multiple repetitions",
        "text": "the the the cat sat on the mat mat mat. the dog dog ran quickly quickly quickly",
        "expected_issues": ["repetition"],
        "description": "Multiple instances of word repetition"
    },
    {
        "name": "Capitalization issues",
        "text": "hello world. this is a sentence without proper capitalization. another sentence here",
        "expected_issues": ["capitalization"],
        "description": "Missing capitalization at sentence beginnings"
    },
    {
        "name": "Mixed issues",
        "text": "the quick brown fox fox jumps over the lazy dog dog dog. it was a sunny sunny day",
        "expected_issues": ["repetition", "capitalization"],
        "description": "Combination of repetition and capitalization issues"
    },
    {
        "name": "Long text with various issues",
        "text": "artificial intelligence is is transforming the world. machine learning algorithms algorithms learn from data data data. neural networks networks process information quickly quickly. the future future of technology looks bright bright bright",
        "expected_issues": ["repetition", "capitalization"],
        "description": "Extended text with multiple types of issues"
    },
    {
        "name": "Clean text",
        "text": "This is a well-formatted sentence with proper punctuation and capitalization. It should require minimal cleaning.",
        "expected_issues": [],
        "description": "Text that should require minimal rule enforcement"
    }
]

STRICTNESS_LEVELS = [0.3, 0.5, 0.7, 1.0]

GENERATION_PROMPTS = [
    {"prompt": "The weather today is", "length": 40, "method": "nucleus", "temp": 0.8},
    {"prompt": "Artificial intelligence will", "length": 60, "method": "nucleus", "temp": 1.0},
    {"prompt": "In the future, technology", "length": 80, "method": "greedy", "temp": 1.0},
    {"prompt": "The importance of education", "length": 50, "method": "nucleus", "temp": 0.6},
]

# --- Helper function for text generation ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    if method == "nucleus" and "top_p" not in kwargs:
        kwargs["top_p"] = 0.9
    prompt_tokens = tokenizer.tokenize(prompt).tolist()
    generated_tokens = model.generator.sample(
        prompt_tokens=prompt_tokens,
        n_new_tokens=length,
        method=method,
        tokenizer=tokenizer,  # Pass tokenizer for methods that need it
        **kwargs
    )
    return tokenizer.detokenize(np.array(generated_tokens))

def analyze_text_quality(text):
    """Analyze text quality metrics before and after rule enforcement."""
    analysis = {
        "word_count": len(text.split()),
        "char_count": len(text),
        "sentence_count": len(re.split(r'[.!?]+', text.strip())),
        "unique_words": len(set(text.lower().split())),
    }
    
    words = text.split()
    if words:
        analysis["lexical_diversity"] = len(set(words)) / len(words)
        analysis["avg_word_length"] = np.mean([len(word) for word in words])
        
        # Repetition analysis
        word_counts = Counter(words)
        repeated_words = {word: count for word, count in word_counts.items() if count > 1}
        analysis["repetition_ratio"] = len(repeated_words) / len(words)
        analysis["max_repetition"] = max(word_counts.values()) if word_counts else 0
        analysis["repeated_word_instances"] = sum(count - 1 for count in repeated_words.values())
        
        # Capitalization analysis
        sentences = re.split(r'[.!?]+', text.strip())
        properly_capitalized = 0
        for sentence in sentences:
            sentence = sentence.strip()
            if sentence and sentence[0].isupper():
                properly_capitalized += 1
        analysis["capitalization_ratio"] = properly_capitalized / len(sentences) if sentences else 0
        
        # Punctuation analysis
        analysis["ends_with_punctuation"] = text.strip().endswith(('.', '!', '?'))
        analysis["punctuation_count"] = len(re.findall(r'[.!?;,:]', text))
        
    else:
        analysis.update({
            "lexical_diversity": 0,
            "avg_word_length": 0,
            "repetition_ratio": 0,
            "max_repetition": 0,
            "repeated_word_instances": 0,
            "capitalization_ratio": 0,
            "ends_with_punctuation": False,
            "punctuation_count": 0
        })
    
    return analysis

def run_rule_enforcement_on_test_cases(strictness_levels):
    """Test rule enforcement on predefined test cases."""
    report = ["\n  --- Test 5a: Rule Enforcement on Test Cases ---"]
    report.append(f"  Testing {len(RULE_ENFORCEMENT_TEST_CASES)} test cases with {len(strictness_levels)} strictness levels")
    
    all_results = []
    
    for case_idx, test_case in enumerate(RULE_ENFORCEMENT_TEST_CASES):
        report.append(f"\n    Test Case {case_idx + 1}: {test_case['name']}")
        report.append(f"    Description: {test_case['description']}")
        report.append(f"    Original: '{test_case['text'][:80]}...'")
        
        case_results = {
            "name": test_case['name'],
            "original_text": test_case['text'],
            "expected_issues": test_case['expected_issues'],
            "strictness_results": {}
        }
        
        # Analyze original text
        original_analysis = analyze_text_quality(test_case['text'])
        case_results["original_analysis"] = original_analysis
        
        for strictness in strictness_levels:
            try:
                start_time = time.time()
                enforcer = RuleEnforcer(strictness=strictness)
                cleaned_text = enforcer.apply_rules(test_case['text'])
                end_time = time.time()
                
                # Analyze cleaned text
                cleaned_analysis = analyze_text_quality(cleaned_text)
                
                # Calculate improvements
                improvements = {
                    "repetition_reduction": original_analysis["repetition_ratio"] - cleaned_analysis["repetition_ratio"],
                    "capitalization_improvement": cleaned_analysis["capitalization_ratio"] - original_analysis["capitalization_ratio"],
                    "lexical_diversity_change": cleaned_analysis["lexical_diversity"] - original_analysis["lexical_diversity"],
                    "processing_time": end_time - start_time
                }
                
                case_results["strictness_results"][strictness] = {
                    "cleaned_text": cleaned_text,
                    "cleaned_analysis": cleaned_analysis,
                    "improvements": improvements,
                    "processing_time": end_time - start_time
                }
                
                report.append(f"      Strictness {strictness}: '{cleaned_text[:60]}...'")
                report.append(f"        Repetition reduction: {improvements['repetition_reduction']:.3f}")
                report.append(f"        Capitalization improvement: {improvements['capitalization_improvement']:.3f}")
                report.append(f"        Processing time: {improvements['processing_time']:.4f}s")
                
            except Exception as e:
                report.append(f"      [ERROR] Strictness {strictness} failed: {e}")
                case_results["strictness_results"][strictness] = {"error": str(e)}
        
        all_results.append(case_results)
    
    return report, all_results

def run_rule_enforcement_on_generated_text(model, tokenizer):
    """Test rule enforcement on model-generated text."""
    report = ["\n  --- Test 5b: Rule Enforcement on Generated Text ---"]
    report.append(f"  Testing rule enforcement on {len(GENERATION_PROMPTS)} generated texts")
    
    all_results = []
    
    for prompt_idx, prompt_config in enumerate(GENERATION_PROMPTS):
        report.append(f"\n    Generation {prompt_idx + 1}: {prompt_config['method']} method")
        report.append(f"    Prompt: '{prompt_config['prompt']}'")
        report.append(f"    Length: {prompt_config['length']}, Temperature: {prompt_config['temp']}")
        
        try:
            # Generate text
            start_gen_time = time.time()
            generated_text = generate_with_model(
                model, tokenizer,
                prompt=prompt_config['prompt'],
                length=prompt_config['length'],
                method=prompt_config['method'],
                temperature=prompt_config['temp'],
                top_p=0.9
            )
            end_gen_time = time.time()
            
            generation_result = {
                "prompt_config": prompt_config,
                "generated_text": generated_text,
                "generation_time": end_gen_time - start_gen_time,
                "strictness_results": {}
            }
            
            # Analyze original generated text
            original_analysis = analyze_text_quality(generated_text)
            generation_result["original_analysis"] = original_analysis
            
            report.append(f"    Generated: '{generated_text[:80]}...'")
            report.append(f"    Generation time: {generation_result['generation_time']:.3f}s")
            report.append(f"    Original quality - Repetition: {original_analysis['repetition_ratio']:.3f}, "
                         f"Lexical diversity: {original_analysis['lexical_diversity']:.3f}")
            
            # Apply rule enforcement at different strictness levels
            for strictness in STRICTNESS_LEVELS:
                try:
                    start_time = time.time()
                    enforcer = RuleEnforcer(strictness=strictness)
                    cleaned_text = enforcer.apply_rules(generated_text)
                    end_time = time.time()
                    
                    # Analyze cleaned text
                    cleaned_analysis = analyze_text_quality(cleaned_text)
                    
                    # Calculate improvements
                    improvements = {
                        "repetition_reduction": original_analysis["repetition_ratio"] - cleaned_analysis["repetition_ratio"],
                        "capitalization_improvement": cleaned_analysis["capitalization_ratio"] - original_analysis["capitalization_ratio"],
                        "lexical_diversity_change": cleaned_analysis["lexical_diversity"] - original_analysis["lexical_diversity"],
                        "processing_time": end_time - start_time,
                        "length_change": len(cleaned_text) - len(generated_text)
                    }
                    
                    generation_result["strictness_results"][strictness] = {
                        "cleaned_text": cleaned_text,
                        "cleaned_analysis": cleaned_analysis,
                        "improvements": improvements,
                        "processing_time": end_time - start_time
                    }
                    
                    report.append(f"      Strictness {strictness}: Repetition reduction: {improvements['repetition_reduction']:.3f}, "
                                 f"Length change: {improvements['length_change']}")
                    
                except Exception as e:
                    report.append(f"      [ERROR] Strictness {strictness} failed: {e}")
                    generation_result["strictness_results"][strictness] = {"error": str(e)}
            
            all_results.append(generation_result)
            
        except Exception as e:
            report.append(f"    [ERROR] Generation {prompt_idx + 1} failed: {e}")
            all_results.append({
                "prompt_config": prompt_config,
                "error": str(e)
            })
    
    return report, all_results

def analyze_rule_enforcement_performance(test_case_results, generation_results):
    """Analyze overall rule enforcement performance."""
    analysis = {
        "test_cases": {
            "total": len(test_case_results),
            "successful": len([r for r in test_case_results if any("error" not in sr for sr in r.get("strictness_results", {}).values())]),
        },
        "generated_text": {
            "total": len(generation_results),
            "successful": len([r for r in generation_results if "error" not in r and any("error" not in sr for sr in r.get("strictness_results", {}).values())]),
        },
        "strictness_performance": {},
        "overall_improvements": {}
    }
    
    # Analyze performance by strictness level
    for strictness in STRICTNESS_LEVELS:
        strictness_data = {
            "successful_applications": 0,
            "total_applications": 0,
            "avg_repetition_reduction": 0,
            "avg_capitalization_improvement": 0,
            "avg_processing_time": 0,
            "improvements": []
        }
        
        # Collect data from test cases
        for result in test_case_results:
            strictness_result = result.get("strictness_results", {}).get(strictness, {})
            if "error" not in strictness_result:
                strictness_data["successful_applications"] += 1
                if "improvements" in strictness_result:
                    strictness_data["improvements"].append(strictness_result["improvements"])
            strictness_data["total_applications"] += 1
        
        # Collect data from generated text
        for result in generation_results:
            if "error" not in result:
                strictness_result = result.get("strictness_results", {}).get(strictness, {})
                if "error" not in strictness_result:
                    strictness_data["successful_applications"] += 1
                    if "improvements" in strictness_result:
                        strictness_data["improvements"].append(strictness_result["improvements"])
                strictness_data["total_applications"] += 1
        
        # Calculate averages
        if strictness_data["improvements"]:
            strictness_data["avg_repetition_reduction"] = np.mean([imp["repetition_reduction"] for imp in strictness_data["improvements"]])
            strictness_data["avg_capitalization_improvement"] = np.mean([imp["capitalization_improvement"] for imp in strictness_data["improvements"]])
            strictness_data["avg_processing_time"] = np.mean([imp["processing_time"] for imp in strictness_data["improvements"]])
        
        strictness_data["success_rate"] = strictness_data["successful_applications"] / strictness_data["total_applications"] if strictness_data["total_applications"] > 0 else 0
        
        analysis["strictness_performance"][strictness] = strictness_data
    
    return analysis

def run_rule_enforcement_test(model, tokenizer):
    """Enhanced rule enforcement evaluation with comprehensive testing."""
    report = ["\n--- Test 5: Enhanced Rule Enforcement ---"]
    report.append(f"Testing rule enforcement with {len(STRICTNESS_LEVELS)} strictness levels")
    report.append(f"Strictness levels: {STRICTNESS_LEVELS}")
    
    # Run test cases
    test_case_report, test_case_results = run_rule_enforcement_on_test_cases(STRICTNESS_LEVELS)
    report.extend(test_case_report)
    
    # Run on generated text
    generation_report, generation_results = run_rule_enforcement_on_generated_text(model, tokenizer)
    report.extend(generation_report)
    
    # Overall analysis
    analysis = analyze_rule_enforcement_performance(test_case_results, generation_results)
    
    report.append(f"\n--- Rule Enforcement Performance Analysis ---")
    report.append(f"  Test Cases: {analysis['test_cases']['successful']}/{analysis['test_cases']['total']} successful")
    report.append(f"  Generated Text: {analysis['generated_text']['successful']}/{analysis['generated_text']['total']} successful")
    
    report.append(f"\n  Performance by Strictness Level:")
    for strictness, perf in analysis["strictness_performance"].items():
        report.append(f"    Strictness {strictness}:")
        report.append(f"      Success rate: {perf['success_rate']:.1%}")
        report.append(f"      Avg repetition reduction: {perf['avg_repetition_reduction']:.3f}")
        report.append(f"      Avg capitalization improvement: {perf['avg_capitalization_improvement']:.3f}")
        report.append(f"      Avg processing time: {perf['avg_processing_time']:.4f}s")
    
    # Recommendations
    report.append(f"\n  Recommendations:")
    best_strictness = max(analysis["strictness_performance"].items(), 
                         key=lambda x: x[1]["success_rate"] * (x[1]["avg_repetition_reduction"] + x[1]["avg_capitalization_improvement"]))
    report.append(f"    Best performing strictness level: {best_strictness[0]} (success rate: {best_strictness[1]['success_rate']:.1%})")
    
    avg_processing_time = np.mean([perf["avg_processing_time"] for perf in analysis["strictness_performance"].values() if perf["avg_processing_time"] > 0])
    if avg_processing_time < 0.01:
        report.append(f"    ✅ Rule enforcement is very fast (avg: {avg_processing_time:.4f}s)")
    elif avg_processing_time < 0.1:
        report.append(f"    ✅ Rule enforcement is reasonably fast (avg: {avg_processing_time:.4f}s)")
    else:
        report.append(f"    ⚠️  Rule enforcement may be slow for real-time use (avg: {avg_processing_time:.4f}s)")
    
    return report
