import sys
import os
import numpy as np

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from quanta_tissu.quanta_tissu.model import QuantaTissu
from quanta_tissu.quanta_tissu.tokenizer import Tokenizer
from quanta_tissu.quanta_tissu.config import model_config

def main():
    """
    A simple demonstration of the QuantaTissu model's core functionality.
    This script follows the steps outlined in Phase 5 of the project plan.
    """
    print("--- QuantaTissu Core Functionality Demo ---")

    # 1. Instantiate the Tokenizer and Model
    print("1. Initializing tokenizer and model...")
    tokenizer = Tokenizer()
    model = QuantaTissu(model_config)
    np.random.seed(42) # for reproducibility

    # 2. Tokenize a sample prompt
    prompt = "hello world"
    print(f"\n2. Tokenizing prompt: '{prompt}'")
    token_ids = tokenizer.tokenize(prompt)
    print(f"   - Token IDs: {token_ids}")

    # 3. Add a batch dimension
    print("\n3. Adding batch dimension...")
    batched_token_ids = np.array([token_ids])
    print(f"   - Batched Token IDs shape: {batched_token_ids.shape}")

    # 4. Generate the next token
    print("\n4. Predicting next token...")
    next_token_id = model.predict(batched_token_ids, method="greedy")
    print(f"   - Predicted Token ID: {next_token_id}")

    # 5. Detokenize and print the result
    print("\n5. Detokenizing result...")
    next_token = tokenizer.detokenize([next_token_id])
    print(f"   - Predicted Token: '{next_token}'")

    print("\n--- Demo Complete ---")
    print(f"\nFinal output: {prompt} {next_token}")

if __name__ == "__main__":
    main()
