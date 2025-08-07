import sys
import os
import numpy as np

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from quanta_tissu.model import QuantaTissu
from quanta_tissu.tokenizer import detokenize
from quanta_tissu.config import model_config

def main():
    # --- Setup ---
    # For reproducibility, let's seed numpy's random generator
    np.random.seed(42)
    model = QuantaTissu(model_config)

    # --- Populate Knowledge Base ---
    print("--- Populating Knowledge Base ---")
    # Add documents using words from the limited vocabulary
    model.knowledge_base.add_document("this is a test")
    model.knowledge_base.add_document("hello world .")
    print("-" * 20)

    # --- Run Inference with Knowledge Base ---
    # This prompt is designed to be similar to "hello world ."
    prompt = "world"
    print(f"\n--- Running Generation with KB ---")
    print(f"Prompt: '{prompt}'")

    # Use the new method to generate a prediction. The model should retrieve context.
    kb_token = model.generate_with_kb(prompt, generation_method="greedy")

    if kb_token is not None:
        print(f"\nGreedy prediction with KB: '{detokenize([kb_token])}'")
    else:
        print("\nPrediction failed.")

    # --- Example without relevant context ---
    # This prompt has a low chance of matching any document well
    prompt_no_context = "a a"
    print(f"\n--- Running Generation with a prompt that has no context ---")
    print(f"Prompt: '{prompt_no_context}'")

    # The model should not find relevant context and will use the prompt as-is
    kb_token_no_context = model.generate_with_kb(prompt_no_context, generation_method="greedy")
    if kb_token_no_context is not None:
        print(f"\nGreedy prediction without KB context: '{detokenize([kb_token_no_context])}'")
    else:
        print("\nPrediction failed.")


if __name__ == "__main__":
    main()
