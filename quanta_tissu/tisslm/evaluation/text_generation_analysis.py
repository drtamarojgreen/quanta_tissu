import logging
from typing import List, Dict, Any

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.model import QuantaTissu
# from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.generate_text import generate_text



def analyze_generated_text(model, tokenizer, prompt: str, num_samples: int = 3, **generation_kwargs):
    """
    Generates multiple text samples and provides basic analysis.

    Args:
        model: The language model.
        tokenizer: The tokenizer.
        prompt: The starting prompt for text generation.
        num_samples: Number of text samples to generate.
        generation_kwargs: Additional keyword arguments for the generate_text function
                           (e.g., length, temperature, method, top_k, top_p).
    """
    logging.info(f"--- Analyzing Text Generation for Prompt: '{prompt}' ---")
    generated_texts = []
    for i in range(num_samples):
        logging.info(f"Generating sample {i+1}/{num_samples}...")
        try:
            # The actual generate_text function would be imported from core.generate_text
            # For now, we'll assume it exists and works as expected.
            generated_text = generate_text(model=model, tokenizer=tokenizer, prompt=prompt, length=generation_kwargs.get('length', 50), method=generation_kwargs.get('method', 'greedy'), temperature=generation_kwargs.get('temperature', 1.0), top_k=generation_kwargs.get('top_k'), top_p=generation_kwargs.get('top_p'), repetition_penalty=generation_kwargs.get('repetition_penalty', 1.0))
            generated_texts.append(generated_text)
            logging.info(f"Sample {i+1}: {generated_text}")
        except Exception as e:
            logging.error(f"Error generating text sample {i+1}: {e}")
            generated_texts.append(f"ERROR: {e}")

    logging.info("\n--- Summary of Generated Texts ---")
    for i, text in enumerate(generated_texts):
        logging.info(f"Sample {i+1} Length: {len(text.split())} words")
        # Add more analysis here, e.g., check for repetition, coherence scores, etc.
        # For example:
        # from collections import Counter
        # words = text.lower().split()
        # word_counts = Counter(words)
        # logging.info(f"  Most common words: {word_counts.most_common(5)}")

    return generated_texts

def compare_generation_strategies(model, tokenizer, prompt: str, length: int = 50, repetition_penalty: float = 1.0):
    """
    Compares text generation using different decoding strategies.
    """
    logging.info(f"\n--- Comparing Generation Strategies for Prompt: '{prompt}' ---")

    strategies = {
        "greedy": {"method": "greedy"},
        "temperature_0.5": {"method": "sampling", "temperature": 0.5},
        "temperature_1.0": {"method": "sampling", "temperature": 1.0},
        "top_k_50": {"method": "top_k", "top_k": 50},
        "top_p_0.9": {"method": "nucleus", "top_p": 0.9},
    }

    results = {}
    for name, kwargs in strategies.items():
        logging.info(f"Generating with strategy: {name}")
        try:
            generated_text = generate_text(model=model, tokenizer=tokenizer, prompt=prompt, length=length, method=kwargs.get('method', 'greedy'), temperature=kwargs.get('temperature', 1.0), top_k=kwargs.get('top_k'), top_p=kwargs.get('top_p'), repetition_penalty=repetition_penalty, bias_token_id=kwargs.get('bias_token_id'), bias_strength=kwargs.get('bias_strength'))
            results[name] = generated_text
            logging.info(f"  {name}: {generated_text}")
        except Exception as e:
            logging.error(f"Error with strategy {name}: {e}")
            results[name] = f"ERROR: {e}"
    return results

# You can add more functions for specific analysis, e.g.,
# - analyze_repetition(text)
# - calculate_coherence_score(text)


def main():
    """
    Main function to demonstrate the text generation analysis capabilities.
    When run as a script, this will load a model and tokenizer, then generate
    and analyze text based on a sample prompt.
    """
    import os
    import sys

    # --- Setup Project Root ---
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
    if project_root not in sys.path:
        sys.path.insert(0, project_root)

    # --- Imports ---
    from quanta_tissu.tisslm.core.model import QuantaTissu
    from quanta_tissu.tisslm.core.tokenizer import Tokenizer
    from quanta_tissu.tisslm.config import model_config

    # --- Configuration ---
    TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
    TEST_MODEL_DIR = os.path.join(project_root, "test_model")
    TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
    FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")

    # --- Logging ---
    # Use a basic logger for demonstration purposes
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

    print("="*80)
    print("Running Text Generation Analysis Demonstration")
    print("="*80)

    # --- Load Model and Tokenizer ---
    try:
        print("Loading tokenizer...")
        tokenizer = Tokenizer(tokenizer_prefix=TOKENIZER_SAVE_PREFIX)
        model_config["vocab_size"] = tokenizer.get_vocab_size()

        print("Loading model...")
        model = QuantaTissu(model_config)
        model.load_weights(FINAL_CHECKPOINT_PATH)
        print("Model and tokenizer loaded successfully.")
    except Exception as e:
        print(f"FATAL: Could not load model or tokenizer. {e}")
        print("Please ensure that a test model and tokenizer exist.")
        sys.exit(1)

    # --- Define Sample Prompt ---
    prompt = "In a world where algorithms write symphonies,"

    # --- Run Analysis ---
    print(f"\n--- Running analyze_generated_text ---")
    analyze_generated_text(
        model,
        tokenizer,
        prompt=prompt,
        num_samples=2,
        length=30,
        temperature=0.8
    )

    # --- Run Comparison ---
    print(f"\n--- Running compare_generation_strategies ---")
    compare_generation_strategies(
        model,
        tokenizer,
        prompt=prompt,
        length=30
    )

    print("\n" + "="*80)
    print("Demonstration Complete.")
    print("="*80)


if __name__ == "__main__":
    main()
