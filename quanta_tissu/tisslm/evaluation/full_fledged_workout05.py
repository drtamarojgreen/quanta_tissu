import os
import sys
import numpy as np
import re

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
