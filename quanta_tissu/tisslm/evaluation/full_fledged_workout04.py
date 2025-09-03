import os
import sys
import numpy as np

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")

TEST_CONFIGURATIONS = [
    {"prompt": "The meaning of life is", "method": "greedy", "temperature": 1.0, "top_k": 1, "top_p": 1.0, "length": 50},
    {"prompt": "The future of humanity is", "method": "nucleus", "temperature": 0.7, "top_k": 50, "top_p": 0.85, "length": 50},
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
