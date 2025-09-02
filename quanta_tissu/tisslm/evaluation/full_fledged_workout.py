
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
from quanta_tissu.tisslm.evaluation.generate_similar import analyze_similarity, levenshtein_distance # Reusing analysis
from quanta_tissu.tisslm.core.rules.enforcer import RuleEnforcer
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer
from quanta_tissu.tisslm.core.grammar_parser import GrammarParser
from quanta_tissu.tisslm.core.lexicon_analyzer import LexiconAnalyzer
from quanta_tissu.tisslm.core.context_reviewer import ContextReviewer

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz") # Assuming the 50k checkpoint
DICT_PATH = os.path.join(project_root, "data", "dict.json")
WORDLIST_PATH = os.path.join(project_root, "data", "wordlist.txt")
TEMP_LIST_PATH = os.path.join(project_root, "data", "temp_list.txt")
CORPUS_SENTIMENTS_PATH = os.path.join(project_root, "data", "corpus_sentiments.cat")

# --- Test Configurations (subset of test_tune_prompt) ---
TEST_CONFIGURATIONS = [
    {"prompt": "The meaning of life is", "method": "greedy", "temperature": 1.0, "top_k": 1, "top_p": 1.0, "length": 50},
    {"prompt": "The future of humanity is", "method": "nucleus", "temperature": 0.7, "top_k": 50, "top_p": 0.85, "length": 50},
    {"prompt": "Once upon a time, in a land far away", "method": "nucleus", "temperature": 0.9, "top_k": 100, "top_p": 0.95, "length": 50},
]

# Example Multi-Word Expressions for LexiconAnalyzer
EXAMPLE_MWES = [
    "once upon a time", "as a matter of fact", "in order to", "by the way",
    "for example", "such as", "in addition", "on the other hand"
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

    # --- Initialize Analyzers ---
    grammar_parser = GrammarParser()
    lexicon_analyzer = LexiconAnalyzer()
    context_reviewer = ContextReviewer()
    report.append("GrammarParser, LexiconAnalyzer, ContextReviewer initialized.")

    # --- Test 1: Standard Generation Configurations ---
    report.append("\n--- Test 1: Standard Generation Configurations ---")
    generated_texts_for_analysis = []
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
        generated_texts_for_analysis.append({'config': config, 'text': generated_text})

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
    report.append(f"Sample text for analysis: '{sample_text_for_analysis[:100]}...'")
    if analysis_results:
        report.append("Top 5 similar words:")
        for i, item in enumerate(analysis_results[:5]): # Show only top 5 generated words for brevity
            if item['top_matches']:
                first_match = item['top_matches'][0]
                report.append(f"  - '{item['generated_word']}' -> '{first_match['word']}' (Dist: {first_match['distance']})")
            else:
                report.append(f"  - '{item['generated_word']}' -> (No close match found)")
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
    report.append(f"Raw text for rules: '{raw_text_for_rules[:100]}...'")

    # Overall Enforcement
    enforcer_default = RuleEnforcer(strictness=0.5)
    cleaned_text_default = enforcer_default.apply_rules(raw_text_for_rules)
    report.append(f"Cleaned (Default Strictness): '{cleaned_text_default[:100]}...'")

    enforcer_strict = RuleEnforcer(strictness=1.0)
    cleaned_text_strict = enforcer_strict.apply_rules(raw_text_for_rules)
    report.append(f"Cleaned (Strict Strictness): '{cleaned_text_strict[:100]}...'")

    # Rule-by-Rule Breakdown
    report.append("\n  Rule-by-Rule Breakdown (Strictness: 1.0):")
    text_for_breakdown = raw_text_for_rules
    for rule in enforcer_strict.rules:
        transformed_text = rule.apply(text_for_breakdown)
        if transformed_text != text_for_breakdown:
            report.append(f"    - Rule: {rule.__class__.__name__}")
            report.append(f"      - BEFORE: '{text_for_breakdown[:80]}...'")
            report.append(f"      - AFTER:  '{transformed_text[:80]}...'")
            text_for_breakdown = transformed_text # Use the transformed text for the next rule

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

    # Bayesian Word Expansion (commented out due to persistent error)
    # try:
    #     # Mock query_embedding and hessian_matrix for demonstration purposes
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

    # Adaptive Sentiment Sampling
    try:
        # Ensure corpus_sentiments.cat exists and is populated for this test
        if not os.path.exists(CORPUS_SENTIMENTS_PATH) or not sentiment_analyzer.sentiment_scores:
            report.append(f"Skipping Adaptive Sentiment Sampling: {CORPUS_SENTIMENTS_PATH} not found or empty.")
        else:
            generated_ass = model.sample(
                tokenizer.tokenize("I feel very").tolist(),
                n_new_tokens=30,
                method="adaptive_sentiment",
                tokenizer=tokenizer,
                sentiment_analyzer=sentiment_analyzer,
                target_sentiment="positive",
                target_strength=0.8,
                evaluation_interval=5
            )
            report.append(f"Adaptive Sentiment Sampling (Positive): '{tokenizer.detokenize(np.array(generated_ass))[:50]}...'")

            generated_ass_neg = model.sample(
                tokenizer.tokenize("This is a very").tolist(),
                n_new_tokens=30,
                method="adaptive_sentiment",
                tokenizer=tokenizer,
                sentiment_analyzer=sentiment_analyzer,
                target_sentiment="negative",
                target_strength=0.8,
                evaluation_interval=5
            )
            report.append(f"Adaptive Sentiment Sampling (Negative): '{tokenizer.detokenize(np.array(generated_ass_neg))[:50]}...'")

    except Exception as e:
        report.append(f"Error in Adaptive Sentiment Sampling: {e}")

    # --- Test 5: Advanced Text Analysis (Grammar, Lexicon, Context) ---
    report.append("\n--- Test 5: Advanced Text Analysis ---")
    for item in generated_texts_for_analysis:
        config = item['config']
        analysis_text = item['text']
        report.append(f"\n  --- Analysis for Config (Method: {config['method']}) ---")
        report.append(f"  Text Analyzed: '{analysis_text[:100]}...'")

        # Grammar Parser Analysis
        report.append("\n  Grammar Parser Analysis:")
        sentences = re.split(r'(?<=[.?!])\s+', analysis_text) # Split into sentences
        for sentence in sentences:
            if sentence.strip():
                grammar_analysis = grammar_parser.analyze_sentence(sentence)
                report.append(f"    Sentence: '{sentence.strip()[:50]}...'")
                report.append(f"      Noun Phrases: {grammar_analysis['noun_phrases']}")
                report.append(f"      Verb Phrases: {grammar_analysis['verb_phrases']}")
                report.append(f"      Prepositional Phrases: {grammar_analysis['prepositional_phrases']}")

        # Lexicon Analyzer Analysis
        report.append("\n  Lexicon Analyzer Analysis:")
        lexicon_analysis = lexicon_analyzer.analyze_text(analysis_text, mwe_list=EXAMPLE_MWES, n_gram=2, top_n_collocations=5)
        report.append(f"    Found MWEs: {lexicon_analysis['found_mwe']}")
        report.append(f"    Top 5 Bigrams: {lexicon_analysis['collocations']}")

        # Context Reviewer Analysis
        report.append("\n  Context Reviewer Analysis:")
        context_analysis = context_reviewer.analyze_text(analysis_text)
        if context_analysis['sentence_transitions']:
            report.append(f"    Sentence Transition Issues: {len(context_analysis['sentence_transitions'])} issues found.")
        else:
            report.append("    Sentence Transitions: OK.")
        if context_analysis['paragraph_unity']:
            report.append(f"    Paragraph Unity Issues: {len(context_analysis['paragraph_unity'])} issues found.")
        else:
            report.append("    Paragraph Unity: OK.")

    # --- Final Report ---
    print("\n" + "="*80)
    print("FULL FLEDGED WORKOUT REPORT")
    print("="*80)
    print("\n".join(report))
    print("\n" + "="*80)

if __name__ == "__main__":
    main()
