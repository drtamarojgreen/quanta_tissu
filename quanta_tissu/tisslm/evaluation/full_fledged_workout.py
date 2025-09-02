import os
import sys
import argparse
import json
import numpy as np
import re
import time

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

# Updated imports to use the new generator and other required components
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.rules.enforcer import RuleEnforcer
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer
from quanta_tissu.tisslm.core.grammar_parser import GrammarParser
from quanta_tissu.tisslm.core.lexicon_analyzer import LexiconAnalyzer
from quanta_tissu.tisslm.core.context_reviewer import ContextReviewer
# Imports for new tests
from quanta_tissu.tisslm.core.knowledge_base import KnowledgeBase
from quanta_tissu.tisslm.core.embedding.embedder import Embedder
from quanta_tissu.tisslm.core.db.client import TissDBClient
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

# --- Test Configurations ---
TEST_CONFIGURATIONS = [
    {"prompt": "The meaning of life is", "method": "greedy", "temperature": 1.0, "top_k": 1, "top_p": 1.0, "length": 50},
    {"prompt": "The future of humanity is", "method": "nucleus", "temperature": 0.7, "top_k": 50, "top_p": 0.85, "length": 50},
]

# --- Helper function for text generation ---
def generate_with_model(model, tokenizer, prompt, length, method, **kwargs):
    """Helper to generate text using model.sample, which uses AlgorithmicGenerator."""
    prompt_tokens = tokenizer.tokenize(prompt).tolist()
    generated_tokens = model.sample(
        prompt_tokens=prompt_tokens,
        n_new_tokens=length,
        method=method,
        tokenizer=tokenizer,  # Pass tokenizer for methods that need it
        **kwargs
    )
    return tokenizer.detokenize(np.array(generated_tokens))

# --- Test Functions ---

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

def run_rag_and_self_update_test(model, tokenizer):
    """Evaluates the Retrieve-Evaluate-Generate workflow and self-updating KB."""
    report = ["\n--- Test 2: RAG and Self-Updating Knowledge Base ---"]

    try:
        # --- 1. Setup RAG components in disconnected mode ---
        report.append("\n  --- Test 2a: Retrieve-Evaluate-Generate (RAG) ---")
        report.append("  Setting up RAG components in disconnected (local cache) mode...")

        # The embedder needs the model's embedding matrix and tokenizer
        embedder = Embedder(model.embeddings.value, tokenizer)
        db_client = TissDBClient(is_mock=True) # Mock client, won't connect to a real DB
        kb = KnowledgeBase(embedder=embedder, db_client=db_client)

        # --- 2. Populate the Knowledge Base ---
        report.append("  Populating in-memory Knowledge Base with sample documents...")
        sample_docs = [
            "The first manned mission to Mars, named 'Ares 1', is scheduled for 2035.",
            "The capital of the fictional country of Eldoria is Silverhaven.",
            "Quantum computing relies on the principles of superposition and entanglement."
        ]
        for doc in sample_docs:
            kb.add_document(doc)
        report.append(f"  Added {len(sample_docs)} documents to the local KB cache.")

        # --- 3. Run the RAG Workflow ---
        user_query = "What is the name of the first Mars mission and when is it scheduled?"
        report.append(f"\n  User Query: '{user_query}'")

        # Step 3a: Retrieve
        retrieved_docs = kb.retrieve(user_query, k=1)
        if not retrieved_docs:
            report.append("  [ERROR] Retrieval failed to return any documents.")
            return report
        retrieved_context = "\n".join(retrieved_docs)
        report.append(f"  Retrieved Context: '{retrieved_context}'")

        # Step 3b: Evaluate
        evaluator_prompt = f"""You are a fact-checking AI. Analyze the retrieved documents in the context of the user's query. Extract only the information that is factually accurate and directly relevant.

User Query: "{user_query}"

Retrieved Documents:
---
{retrieved_context}
---

Output only the extracted, verified facts."""

        report.append("  Sending context to LLM for evaluation and sanitization...")
        sanitized_context = generate_with_model(
            model, tokenizer,
            prompt=evaluator_prompt,
            length=60,
            method="nucleus"
        )
        report.append(f"  Sanitized Context: '{sanitized_context}'")

        # Step 3c: Generate
        final_prompt = f"""Based on the following verified information, answer the user's question.

Verified Information: "{sanitized_context}"

User Question: "{user_query}"

Answer:"""
        report.append("  Generating final answer using sanitized context...")
        final_answer = generate_with_model(
            model, tokenizer,
            prompt=final_prompt,
            length=50,
            method="nucleus"
        )
        report.append(f"  Final Generated Answer: '{final_answer}'")

        # --- 4. Test Self-Updating Knowledge Base ---
        report.append("\n  --- Test 2b: Self-Updating Knowledge Base ---")
        report.append("  Testing the ability of the KB to learn from interactions...")

        # Use the query and answer from the previous RAG step to update the KB
        kb.self_update_from_interaction(query=user_query, generated_response=final_answer)
        report.append("  KB updated with the previous Q&A pair.")

        # Now, try to retrieve the information with a new, related query
        new_query = "Tell me about the Ares 1 mission."
        report.append(f"  New Query: '{new_query}'")

        retrieved_after_update = kb.retrieve(new_query, k=1)

        if not retrieved_after_update:
            report.append("  [ERROR] Retrieval after self-update failed.")
        else:
            report.append(f"  Retrieved after update: '{retrieved_after_update[0][:100]}...'")
            # Check if the retrieved text contains content from the last interaction
            if user_query in retrieved_after_update[0] and final_answer in retrieved_after_update[0]:
                report.append("  [SUCCESS] Self-updated knowledge was successfully retrieved.")
            else:
                report.append("  [WARNING] Retrieved document does not seem to contain the self-updated knowledge.")

    except Exception as e:
        report.append(f"\n  [ERROR] RAG test failed unexpectedly: {e}")

    return report

def run_experimental_sampling_tests(model, tokenizer):
    """Evaluates experimental sampling methods like Bayesian Expansion."""
    report = ["\n--- Test 3: Experimental Sampling Methods ---"]

    # --- Test 3a: Dynamic Token Revision ---
    report.append("\n  --- Test 3a: Dynamic Token Revision ---")
    try:
        generated_dtr = generate_with_model(
            model, tokenizer,
            prompt="Testing dynamic token revision",
            length=30,
            method="dynamic_token_revision",
            underlying_method="greedy",
            save_interval=10
        )
        report.append(f"  Generated: '{generated_dtr[:80]}...'")
        report.append(f"  Verification: Check {TEMP_LIST_PATH} for token counts.")
    except Exception as e:
        report.append(f"  [ERROR] Dynamic Token Revision failed: {e}")

    # --- Test 3b: Bayesian Word Expansion (FIXED) ---
    report.append("\n  --- Test 3b: Bayesian Word Expansion ---")
    try:
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
        report.append(f"  Verification: Check {WORDLIST_PATH} for new words.")
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
                # I have to check the script content before running it
                # read_file('quanta_tissu/tisslm/evaluation/create_corpus_sentiments.py') -> contains no compilation
                create_corpus_sentiments_main()
                report.append("  Sentiments file generated successfully.")
            except Exception as e:
                report.append(f"  [ERROR] Failed to generate sentiments file: {e}")
                report.append("  Skipping Adaptive Sentiment Sampling test.")
                return report # Exit this test function early

        sentiment_analyzer = SentimentAnalyzer()
        sentiment_analyzer.load_sentiments(CORPUS_SENTIMENTS_PATH)

        if not sentiment_analyzer.sentiment_scores:
            report.append(f"  [SKIPPED] Adaptive Sentiment Sampling: Sentiments file is empty.")
        else:
            generated_ass_pos = generate_with_model(
                model, tokenizer,
                prompt="I feel very",
                length=30,
                method="adaptive_sentiment",
                sentiment_analyzer=sentiment_analyzer,
                target_sentiment="positive",
                target_strength=0.8
            )
            report.append(f"  Generated (Positive Target): '{generated_ass_pos[:80]}...'")

            generated_ass_neg = generate_with_model(
                model, tokenizer,
                prompt="This is a very",
                length=30,
                method="adaptive_sentiment",
                sentiment_analyzer=sentiment_analyzer,
                target_sentiment="negative",
                target_strength=0.8
            )
            report.append(f"  Generated (Negative Target): '{generated_ass_neg[:80]}...'")

    except Exception as e:
        report.append(f"  [ERROR] Adaptive Sentiment Sampling failed: {e}")

    return report

def run_standard_generation_tests(model, tokenizer):
    """Runs standard generation tests and returns generated texts for other analyses."""
    report = ["\n--- Test 4: Standard Generation Configurations ---"]
    generated_texts_for_analysis = []
    for i, config in enumerate(TEST_CONFIGURATIONS):
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
        report.append(f"Config #{i+1} (Method: {config['method']}): '{generated_text[:100]}...'")
        generated_texts_for_analysis.append({'config': config, 'text': generated_text})
    return report, generated_texts_for_analysis

def run_rule_enforcement_test(model, tokenizer):
    """Evaluates the RuleEnforcer capabilities."""
    report = ["\n--- Test 5: Rule Enforcement ---"]
    raw_text_for_rules = generate_with_model(
        model, tokenizer,
        prompt="this is a test. it has some repeated words. words words. and missing punctuation",
        length=50, method="nucleus", temperature=0.8, top_k=40, top_p=0.9
    )
    report.append(f"Raw text for rules: '{raw_text_for_rules[:100]}...'")
    enforcer_strict = RuleEnforcer(strictness=1.0)
    cleaned_text_strict = enforcer_strict.apply_rules(raw_text_for_rules)
    report.append(f"Cleaned (Strict): '{cleaned_text_strict[:100]}...'")
    return report

def run_advanced_analysis_tests(generated_texts):
    """Evaluates advanced text analysis tools on generated text."""
    report = ["\n--- Test 6: Advanced Text Analysis (Grammar, Lexicon, Context) ---"]
    grammar_parser = GrammarParser()
    lexicon_analyzer = LexiconAnalyzer()
    context_reviewer = ContextReviewer()

    for item in generated_texts:
        analysis_text = item['text']
        report.append(f"\n  --- Analysis for: '{analysis_text[:50]}...' ---")
        # Grammar
        sentences = re.split(r'(?<=[.?!])\s+', analysis_text)
        grammar_analysis = grammar_parser.analyze_sentence(sentences[0] if sentences else "")
        report.append(f"    Grammar (Noun Phrases): {grammar_analysis['noun_phrases']}")
        # Lexicon
        lexicon_analysis = lexicon_analyzer.analyze_text(analysis_text, top_n_collocations=3)
        report.append(f"    Lexicon (Top Bigrams): {lexicon_analysis['collocations']}")
        # Context
        context_analysis = context_reviewer.analyze_text(analysis_text)
        report.append(f"    Context Unity OK: {not context_analysis['paragraph_unity']}")
    return report

def main():
    """Main function to orchestrate the evaluation workout."""
    parser = argparse.ArgumentParser(description="Run a comprehensive evaluation workout on the TissLM model.")
    # Args can be added here as needed
    args = parser.parse_args()

    full_report = []

    # --- Load Tokenizer and Model ---
    try:
        tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
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
    full_report.extend(run_kv_cache_test(model, tokenizer))
    full_report.extend(run_rag_and_self_update_test(model, tokenizer))
    full_report.extend(run_experimental_sampling_tests(model, tokenizer))

    standard_gen_report, generated_texts = run_standard_generation_tests(model, tokenizer)
    full_report.extend(standard_gen_report)

    full_report.extend(run_rule_enforcement_test(model, tokenizer))

    if generated_texts:
        full_report.extend(run_advanced_analysis_tests(generated_texts))

    # --- Final Report ---
    print("\n" + "="*80)
    print("COMPREHENSIVE EXPERIMENTAL FEATURES WORKOUT REPORT")
    print("="*80)
    print("\n".join(full_report))
    print("\n" + "="*80)

if __name__ == "__main__":
    main()
