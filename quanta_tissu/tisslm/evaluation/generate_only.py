import argparse
import logging
import os
import sys
import numpy as np

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.evaluation.training_debugger import inspect_model_parameters
from quanta_tissu.tisslm.evaluation.model_inspector import inspect_model_forward_pass
from quanta_tissu.tisslm.evaluation.config_validator import validate_all_configs
from quanta_tissu.tisslm.config import model_config, training_config, tokenizer_config, system_config # Import all necessary configs

# Setup basic logging for this script
logging.basicConfig(
    level=logging.INFO,
    stream=sys.stdout,
    format='%(asctime)s - %(levelname)s - %(message)s',
    encoding='utf-8'  # Ensure UTF-8 encoding for all log output, preventing errors on redirection.
)
logger = logging.getLogger(__name__)

def main():
    sys.stdout.reconfigure(encoding='utf-8')
    parser = argparse.ArgumentParser(description="Generate text using the QuantaTissu model without full evaluation.")
    
    # Default parameters from config files or sensible hardcoded values
    parser.add_argument("--prompt", type=str, default="The meaning of life is", help="The initial prompt to start generation.")
    parser.add_argument("--length", type=int, default=50, help="The number of tokens to generate.")
    
    # Paths should have sensible defaults if possible, or be clearly documented
    # For now, making them optional with a warning if not found.
    parser.add_argument("--tokenizer_prefix", type=str, default="tokenizer/trained_tokenizer", help="Path prefix for the tokenizer files.")
    parser.add_argument("--checkpoint_path", type=str, default="checkpoints/checkpoint_step_248860.npz", help="Path to the model checkpoint (.npz file).")
    
    parser.add_argument("--method", type=str, default="nucleus", help="Generation method: greedy, top_k, nucleus, random.")
    parser.add_argument("--temperature", type=float, default=0.8, help="Controls randomness. Higher is more random.")
    parser.add_argument("--top_k", type=int, default=20, help="K for top-k sampling.")
    parser.add_argument("--top_p", type=float, default=0.9, help="P for nucleus sampling.")
    parser.add_argument("--repetition_penalty", type=float, default=1.2, help="Penalty for repeating tokens.")
    parser.add_argument("--bias_token_id", type=int, default=None, help="Optional: Token ID to bias during generation.")
    parser.add_argument("--bias_strength", type=float, default=0.0, help="Optional: Strength of the bias to apply to bias_token_id.")

    args = parser.parse_args()

    logger.info("--- Starting Methodical Evaluation ---")

    # Step 1: Validate all configurations to establish a baseline.
    logger.info("Step 1: Validating configurations...")
    if not validate_all_configs(model_config, training_config, tokenizer_config, system_config):
        logger.error("Configuration validation failed. Please check your config files in 'quanta_tissu/tisslm/config'. Exiting.")
        sys.exit(1)
    logger.info("Configurations are valid.")

    logger.info("\nStep 2: Initializing Components...")
    logger.info("Initializing Tokenizer...")
    tokenizer_full_prefix = os.path.normpath(os.path.join(project_root, args.tokenizer_prefix))
    try:
        tokenizer = Tokenizer(tokenizer_prefix=tokenizer_full_prefix)
    except FileNotFoundError as e:
        logger.error(f"Error: Tokenizer files not found at {tokenizer_full_prefix}. {e}")
        logger.error("Please ensure the tokenizer is trained and available at the specified path.")
        sys.exit(1)

    logger.info("Initializing Model...")
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)

    logger.info(f"Loading model weights from {args.checkpoint_path}...")
    checkpoint_full_path = os.path.normpath(os.path.join(project_root, args.checkpoint_path))
    if not os.path.exists(checkpoint_full_path):
        logger.error(f"Error: Checkpoint file not found at '{checkpoint_full_path}'")
        sys.exit(1)
    model.load_weights(checkpoint_full_path)
    logger.info("Model loaded successfully.")

    # The following inspection steps will now be run to analyze the model's state.
    # Add a diagnostic step to trace the forward pass and see where activations might be failing.
    # This helps correlate the "dead parameters" with their effect on data flow.
    logger.info("\n--- Inspecting model forward pass ---")
    inspect_model_forward_pass(model, tokenizer, args.prompt, model_config["block_size"])

    # Add a diagnostic step to inspect the loaded parameters for potential issues.
    # Corrupted weights from a legacy load will often have strange statistics.
    logger.info("--- Inspecting loaded model parameters ---")
    inspect_model_parameters(model)

    logger.info(f"Generating text with prompt: '{args.prompt}'")
    generated_text = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt=args.prompt,
        length=args.length,
        method=args.method,
        temperature=args.temperature,
        top_k=args.top_k,
        top_p=args.top_p,
        bias_token_id=args.bias_token_id,
        bias_strength=args.bias_strength
    )
    logger.info("\n--- Generated Text ---")
    # Use the logger to print the generated text. This ensures it uses the configured
    # UTF-8 encoding and avoids the UnicodeEncodeError when redirecting output to a file.
    logger.info(generated_text)
    logger.info("----------------------")

if __name__ == "__main__":
    main()
