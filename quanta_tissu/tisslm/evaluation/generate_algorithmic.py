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
    parser.add_argument("--tokenizer_path", type=str, default="tokenizer/trained_tokenizer", help="Path prefix for the tokenizer files.")
    parser.add_argument("--checkpoint_path", type=str, default="checkpoints/checkpoint_step_248860.npz", help="Path to the model checkpoint (.npz file).")
    
    parser.add_argument("--temperature", type=float, default=0.8, help="Controls randomness. Higher is more random.")
    parser.add_argument("--top_k", type=int, default=20, help="K for top-k sampling.")
    parser.add_argument("--top_p", type=float, default=0.9, help="P for nucleus sampling.")
    parser.add_argument("--repetition_penalty", type=float, default=1.2, help="Penalty for repeating tokens.")
    parser.add_argument("--bias_token_id", type=int, default=None, help="Optional: Token ID to bias during generation.")
    parser.add_argument("--bias_strength", type=float, default=0.0, help="Optional: Strength of the bias to apply to bias_token_id.")
    parser.add_argument("--verbose", action="store_true", help="Set to true for verbose output.")

    args = parser.parse_args()

    if args.verbose:
        logger.info("--- Starting Methodical Evaluation ---")

    # Step 1: Validate all configurations to establish a baseline.
    if args.verbose:
        logger.info("Step 1: Validating configurations...")
    if not validate_all_configs(model_config, training_config, tokenizer_config, system_config):
        logger.error("Configuration validation failed. Please check your config files in 'quanta_tissu/tisslm/config'. Exiting.")
        sys.exit(1)
    if args.verbose:
        logger.info("Configurations are valid.")

    if args.verbose:
        logger.info("\nStep 2: Initializing Components...")
    if args.verbose:
        logger.info("Initializing Tokenizer...")
    tokenizer_full_path = os.path.normpath(os.path.join(project_root, args.tokenizer_path))
    try:
        tokenizer = Tokenizer(tokenizer_path=tokenizer_full_path)
    except FileNotFoundError as e:
        logger.error(f"Error: Tokenizer files not found at {tokenizer_full_path}. {e}")
        logger.error("Please ensure the tokenizer is trained and available at the specified path.")
        sys.exit(1)

    if args.verbose:
        logger.info("Initializing Model...\n")
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)

    if args.verbose:
        logger.info(f"Loading model weights from {args.checkpoint_path}...")
    checkpoint_full_path = os.path.normpath(os.path.join(project_root, args.checkpoint_path))
    if not os.path.exists(checkpoint_full_path):
        logger.error(f"Error: Checkpoint file not found at '{checkpoint_full_path}'")
        sys.exit(1)
    model.load_weights(checkpoint_full_path)
    if args.verbose:
        logger.info("Model loaded successfully.")

    # The following inspection steps will now be run to analyze the model's state.
    # Add a diagnostic step to trace the forward pass and see where activations might be failing.
    # This helps correlate the "dead parameters" with their effect on data flow.\n"
    if args.verbose:
        logger.info("\n--- Inspecting model forward pass ---")
    inspect_model_forward_pass(model, tokenizer, args.prompt, model_config["block_size"])

    # Add a diagnostic step to inspect the loaded parameters for potential issues.
    # Corrupted weights from a legacy load will often have strange statistics.
    if args.verbose:
        logger.info("--- Inspecting loaded model parameters ---")
    inspect_model_parameters(model)

    generation_methods = ["greedy", "top_k", "nucleus", "random", "cnn_influenced", "bayesian_influenced", "cosine_influenced", "genetic_influenced"]
    output_file_path = os.path.join(project_root, "out2.txt")

    # Determine embedding dimension from model config for dummy data
    embedding_dim = model_config.get('embedding_dim', 128)

    # Generate dummy query_embedding and hessian_matrix for testing
    dummy_query_embedding = np.random.rand(embedding_dim)
    dummy_hessian_matrix = {'eigenvalues': np.random.rand(embedding_dim)}

    configurations_to_test = [
        {"name": "Default", "temperature": 0.8, "top_k": 20, "top_p": 0.9, "repetition_penalty": 1.2},
        {"name": "More Creative", "temperature": 1.0, "top_k": 0, "top_p": 0.95, "repetition_penalty": 1.0},
        {"name": "More Focused", "temperature": 0.5, "top_k": 10, "top_p": 0.8, "repetition_penalty": 1.5},
        {"name": "High Repetition Penalty", "temperature": 0.8, "top_k": 20, "top_p": 0.9, "repetition_penalty": 2.0},
        {"name": "Low Temperature", "temperature": 0.3, "top_k": 20, "top_p": 0.9, "repetition_penalty": 1.2},
        {"name": "High Top-K", "temperature": 0.8, "top_k": 50, "top_p": 0.9, "repetition_penalty": 1.2},
        {"name": "Low Top-P", "temperature": 0.8, "top_k": 0, "top_p": 0.7, "repetition_penalty": 1.2},
        {"name": "No Repetition Penalty", "temperature": 0.8, "top_k": 20, "top_p": 0.9, "repetition_penalty": 1.0},
    ]

    with open(output_file_path, "w", encoding="utf-8") as f_out:
        f_out.write("-- Generated Text Output ---\n")

    for config in configurations_to_test:
        config_name = config["name"]
        current_temperature = config["temperature"]
        current_top_k = config["top_k"]
        current_top_p = config["top_p"]
        current_repetition_penalty = config["repetition_penalty"]

        logger.info(f"\n--- Running Configuration: {config_name} ---")
        with open(output_file_path, "a", encoding="utf-8") as f_out:
            f_out.write(f"--- Configuration: {config_name} (Temp: {current_temperature}, TopK: {current_top_k}, TopP: {current_top_p}, RepPenalty: {current_repetition_penalty}) ---\n")

        for method in generation_methods:
            logger.info(f"\n--- Generating text with prompt: '{args.prompt}' using method: {method} ---")
            
            # Prepare additional arguments for cnn_influenced and bayesian_influenced methods
            extra_args = {}
            if method == "cnn_influenced":
                extra_args["query_embedding"] = dummy_query_embedding
            elif method == "bayesian_influenced":
                extra_args["query_embedding"] = dummy_query_embedding
                extra_args["hessian_matrix"] = dummy_hessian_matrix
            elif method == "cosine_influenced":
                extra_args["query_embedding"] = dummy_query_embedding
            elif method == "genetic_influenced":
                extra_args["query_embedding"] = dummy_query_embedding

            generated_text = generate_text(
                model=model,
                tokenizer=tokenizer,
                prompt=args.prompt,
                length=args.length,
                method=method,  # Use the current method from the loop
                temperature=current_temperature,
                top_k=current_top_k,
                top_p=current_top_p,
                repetition_penalty=current_repetition_penalty,
                bias_token_id=args.bias_token_id,
                bias_strength=args.bias_strength,
                **extra_args # Pass the additional arguments here
            )
            logger.info(f"--- Generated Text ({method}) ---")
            logger.info(generated_text)
            logger.info("----------------------")

            # Write to out2.txt
            with open(output_file_path, "a", encoding="utf-8") as f_out:
                f_out.write(f"--- Generated Text ({method}) ---\n")
                f_out.write(generated_text + "\n")
                f_out.write("----------------------\n\n")



if __name__ == "__main__":
    main()
