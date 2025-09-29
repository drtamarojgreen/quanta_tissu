
import os
import sys
import numpy as np

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.config import model_config

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_1000.npz")

# --- Predefined Generation Configurations ---
# A list of different prompts and settings to test.
TEST_CONFIGURATIONS = [
    {
        "prompt": "The future of AI is",
        "method": "greedy",
        "temperature": 1.0, # Note: temp has no effect on greedy
        "top_k": 1,
        "top_p": 1.0,
        "length": 40
    },
    {
        "prompt": "The future of AI is",
        "method": "nucleus",
        "temperature": 0.7,
        "top_k": 50,
        "top_p": 0.85,
        "length": 40
    },
    {
        "prompt": "The future of AI is",
        "method": "top_k",
        "temperature": 0.7,
        "top_k": 40,
        "top_p": 1.0,
        "length": 40
    },
    {
        "prompt": "To be or not to be, that is the question",
        "method": "nucleus",
        "temperature": 0.9,
        "top_k": 100,
        "top_p": 0.95,
        "length": 60
    },
    {
        "prompt": "The best way to learn a new skill is",
        "method": "nucleus",
        "temperature": 0.75,
        "top_k": 30,
        "top_p": 0.9,
        "length": 50
    },
    {
        "prompt": "Once upon a time, in a land far away",
        "method": "random",
        "temperature": 0.8, 
        "top_k": 200, # A larger top_k for more random sampling
        "top_p": 1.0,
        "length": 50
    }
]

def main():
    """Main function to run a series of generation tests."""
    # --- Load Tokenizer and Model (once) ---
    print("--- Loading tokenizer and model ---")
    if not os.path.exists(FINAL_CHECKPOINT_PATH):
        raise FileNotFoundError(f"Final checkpoint not found at {FINAL_CHECKPOINT_PATH}.")

    try:
        tokenizer = Tokenizer(tokenizer_prefix=TOKENIZER_SAVE_PREFIX)
    except FileNotFoundError:
        raise FileNotFoundError(f"Tokenizer not found at {TOKENIZER_SAVE_PREFIX}.")

    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(FINAL_CHECKPOINT_PATH)
    print("Tokenizer and model loaded successfully.")

    # --- Run All Test Configurations ---
    for i, config in enumerate(TEST_CONFIGURATIONS):
        print(f"\n==================================================================")
        print(f"Running Test Configuration #{i+1}")
        print(f"==================================================================")
        print(f"  Prompt: '{config['prompt']}'")
        print(f"  Method: {config['method']}, Temp: {config['temperature']}, Top-K: {config['top_k']}, Top-P: {config['top_p']}")
        print("------------------------------------------------------------------")

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
        
        print(generated_text)
    
    print("\n\nAll test configurations complete.")

if __name__ == "__main__":
    main()
