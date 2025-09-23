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

def main():
    """Main function to orchestrate the evaluation workout."""
    parser = argparse.ArgumentParser(description="Run a comprehensive evaluation workout on the TissLM model.")
    parser.add_argument('--test', type=str, help='Specify which test to run (e.g., "1", "2", "all").')
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

    # --- Final Report ---
    print("\n" + "="*80)
    print("COMPREHENSIVE EXPERIMENTAL FEATURES WORKOUT REPORT")
    print("="*80)
    print("\n".join(full_report))
    print("\n" + "="*80)

if __name__ == "__main__":
    main()
