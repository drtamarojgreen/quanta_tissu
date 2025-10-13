import argparse
import logging
import os
import sys
from datetime import datetime
import numpy as np

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

# Import core model components
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
from quanta_tissu.tisslm.core.data import Dataset, load_corpus
from quanta_tissu.tisslm.core.generate_text import generate_text # For text_generation_analysis
from quanta_tissu.tisslm.config import model_config, training_config, tokenizer_config, system_config # Import configs directly
from quanta_tissu.tisslm.core.knowledge_base import KnowledgeBase

# Import evaluation scripts
from quanta_tissu.tisslm.evaluation.evaluation_metrics import calculate_perplexity
from quanta_tissu.tisslm.evaluation.text_generation_analysis import analyze_generated_text, compare_generation_strategies
from quanta_tissu.tisslm.evaluation.training_debugger import inspect_model_parameters, check_gradients
from quanta_tissu.tisslm.evaluation.data_analyzer import analyze_corpus_statistics, analyze_dataset_batch
from quanta_tissu.tisslm.evaluation.config_validator import validate_all_configs
from quanta_tissu.tisslm.evaluation.logging_utils import setup_evaluation_logger, log_section_header, log_subsection_header
from quanta_tissu.tisslm.evaluation.model_inspector import inspect_model_forward_pass, inspect_model_parameters_detailed # New imports

# Setup logging
logger = logging.getLogger(__name__)

def main():
    """
    Main function to orchestrate the complete evaluation suite for the QuantaTissu model.
    This script provides a methodical, step-by-step approach to understanding the model's
    internal operations, data handling, and performance, rather than just evaluating
    a final checkpoint. It allows for inspection of the model even without a pre-trained
    checkpoint, focusing on how each component behaves.
    """
    parser = argparse.ArgumentParser(description="Run a complete evaluation suite for the QuantaTissu model.")
    parser.add_argument("--tokenizer_path", type=str, default="models/trained_tokenizer", help="Path prefix for the tokenizer files, relative to project root.")
    parser.add_argument("--corpus_path", type=str, default=os.path.join(project_root, "corpus"), help="Path to the training corpus directory.")
    parser.add_argument("--checkpoint_path", type=str, default=None, help="Optional path to the model checkpoint (.npz) to evaluate. If not provided, a fresh model is initialized.")
    parser.add_argument("--log_dir", type=str, default="evaluation_logs", help="Directory to save evaluation logs.")
    parser.add_argument("--prompt", type=str, default="The meaning of life is", help="Prompt for text generation analysis.")
    parser.add_argument("--generation_length", type=int, default=50, help="Length of generated text samples.")
    parser.add_argument("--num_generation_samples", type=int, default=3, help="Number of text samples to generate for analysis.")

    args = parser.parse_args()

    # Setup logging to file and console
    # This ensures all subsequent logs are captured and organized.
    log_filename = f"evaluation_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
    if args.checkpoint_path:
        # If a checkpoint is used, include its name in the log file for easy identification.
        log_filename = f"evaluation_{os.path.basename(args.checkpoint_path).replace('.npz', '')}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
    log_file = os.path.join(args.log_dir, log_filename)
    setup_evaluation_logger(log_file=log_file, level=logging.DEBUG)

    log_section_header(logger, "Starting Complete Model Evaluation")

    # 1. Configuration Validation
    # This step ensures that the model, training, and tokenizer configurations are
    # well-formed and meet basic requirements before proceeding with resource-intensive
    # operations. It helps catch common errors early.
    log_subsection_header(logger, "Configuration Validation")
    try:
        # Import configuration settings from the project's config module.
        # These define the architecture of the model, training hyperparameters, etc.
        from quanta_tissu.tisslm.config import model_config, training_config, tokenizer_config, system_config
        validate_all_configs(model_config, training_config, tokenizer_config, system_config)
    except Exception as e:
        logger.error(f"Error during configuration validation: {e}. Please check your config files.")
        # Depending on severity, you might want to exit here: sys.exit(1)

    # 2. Initialize Components
    # This section sets up the core components required for the model's operation:
    # the tokenizer for text processing, the QuantaTissu model itself, and the loss function.
    # It also handles loading a pre-trained checkpoint if specified, or initializes a fresh model.
    log_subsection_header(logger, "Initializing Model Components")
    model = None
    tokenizer = None
    loss_fn = None
    try:
        # Construct the full path to the tokenizer files for robustness
        tokenizer_full_path = os.path.normpath(os.path.join(project_root, args.tokenizer_path))
        logger.info(f"Initializing tokenizer from: {tokenizer_full_path}")
        tokenizer = Tokenizer(tokenizer_path=tokenizer_full_path)

        # Update the model's vocabulary size based on the initialized tokenizer.
        model_config["vocab_size"] = tokenizer.get_vocab_size()
        # Instantiate the QuantaTissu model with the defined architecture.
        model = QuantaTissu(model_config)
        # Initialize the loss function, crucial for training and perplexity calculation.
        loss_fn = CrossEntropyLoss()
        
        if args.checkpoint_path:
            # If a checkpoint path is provided, load the model's weights from it.
            # This allows evaluating a specific state of a trained model.
            logger.info(f"Attempting to load model from checkpoint: {args.checkpoint_path}")
            # If a checkpoint path is provided, load the model's weights from it.
            # This allows evaluating a specific state of a trained model.
            logger.info(f"Attempting to load model from checkpoint: {args.checkpoint_path}")
            model.load_weights(args.checkpoint_path)
            logger.info("Model and tokenizer initialized and checkpoint loaded successfully.")
        else:
            # If no checkpoint is specified, the model starts with random/default initialized weights.
            # This is useful for inspecting the model's behavior from scratch.
            logger.info("No checkpoint path provided. Initializing a fresh model with default weights.")
        
    except Exception as e:
        logger.error(f"Failed to initialize model components or load checkpoint: {e}. Exiting evaluation.")
        return # Exit if core components cannot be initialized, as subsequent steps depend on them.

    # 3. Data Analysis
    # This section analyzes the raw input data (corpus) and how it's processed
    # by the tokenizer and data loader. Understanding the data is fundamental
    # to debugging model behavior.
    log_subsection_header(logger, "Data Analysis")
    try:
        # Analyze overall corpus statistics (e.g., token frequencies, vocabulary coverage).
        analyze_corpus_statistics(args.corpus_path, tokenizer)
        # Load a portion of the corpus to create a sample dataset for batch-level analysis.
        token_ids = load_corpus(args.corpus_path, tokenizer)
        if len(token_ids) > 0:
            # Create a small sample dataset to inspect how data batches are formed.
            sample_dataset = Dataset(token_ids[:min(1000, len(token_ids))], batch_size=1, seq_len=model_config["block_size"])
            for x_batch, y_batch in sample_dataset:
                # Analyze a single batch to see token IDs and their decoded text.
                analyze_dataset_batch([x_batch.flatten().tolist()], tokenizer)
                break # Only analyze the first batch for a quick overview.
        else:
            logger.warning("Corpus is empty or could not be loaded. Skipping dataset batch analysis.")
    except Exception as e:
        logger.error(f"Error during data analysis: {e}. Data analysis skipped.")

    # 4. Model Internal Inspection (Focus on "each step of the model")
    # This is a crucial section for understanding the model's internal mechanics.
    # It involves passing input through the model and observing intermediate outputs
    # and the state of its parameters.
    if model and tokenizer: # Ensure model and tokenizer were successfully initialized.
        log_subsection_header(logger, "Model Internal Inspection (Forward Pass & Parameters)")
        try:
            # Inspect the forward pass: trace how an input sequence is transformed
            # as it moves through the model's layers (e.g., embeddings, transformer blocks).
            inspect_model_forward_pass(model, tokenizer, args.prompt, model_config["block_size"])
            # Inspect the model's parameters (weights and biases) in detail,
            # checking for issues like NaNs, Infs, or unusual distributions.
            inspect_model_parameters_detailed(model)
        except Exception as e:
            logger.error(f"Error during model internal inspection: {e}. Internal inspection skipped.")
    else:
        logger.warning("Model or Tokenizer not initialized. Skipping internal inspection.")

    # 5. Perplexity Calculation
    # Perplexity is a standard metric for language models, indicating how well
    # the model predicts a sample. Lower perplexity generally means a better model.
    # This step requires a loaded model and a validation dataset.
    if model and loss_fn: # Ensure model and loss function were successfully initialized.
        log_subsection_header(logger, "Perplexity Calculation")
        try:
            # Load the corpus again to split into training and validation sets for perplexity.
            token_ids = load_corpus(args.corpus_path, tokenizer)
            split_idx = int(len(token_ids) * 0.9) # Common split for validation data.
            val_token_ids = token_ids[split_idx:]
            if len(val_token_ids) > 0:
                # Create a dataset specifically for validation.
                val_dataset = Dataset(val_token_ids, batch_size=training_config["batch_size"], seq_len=tokenizer_config["max_len"])
                calculate_perplexity(model, val_dataset, loss_fn)
            else:
                logger.warning("Validation corpus is empty. Skipping perplexity calculation.")
        except Exception as e:
            logger.error(f"Error during perplexity calculation: {e}. Perplexity calculation skipped.")
    else:
        logger.warning("Model or Loss Function not initialized. Skipping perplexity calculation.")

    # 6. Text Generation Analysis
    # This step evaluates the model's ability to generate coherent and relevant text.
    # It includes generating multiple samples and comparing different decoding strategies.
    if model and tokenizer: # Ensure model and tokenizer were successfully initialized.
        log_subsection_header(logger, "Text Generation Analysis")
        try:
            # Generate and analyze text samples using a specified prompt and parameters.
            analyze_generated_text(
                model=model,
                tokenizer=tokenizer,
                prompt=args.prompt,
                num_samples=args.num_generation_samples,
                length=20, # Shortened length
                method="nucleus", # Default method, can be made configurable via args
                temperature=0.8,
                top_k=20,
                top_p=0.9,
                repetition_penalty=1.2
            )
            # Compare how different decoding strategies (e.g., greedy, sampling) affect generation quality.
            compare_generation_strategies(
                model=model,
                tokenizer=tokenizer,
                prompt=args.prompt,
                length=20, # Shortened length
                repetition_penalty=1.2 # Added repetition penalty
            )
        except Exception as e:
            logger.error(f"Error during text generation analysis: {e}. Text generation analysis skipped.")
    else:
        logger.warning("Model or Tokenizer not initialized. Skipping text generation analysis.")

    # 7. Knowledge Base Retrieval Analysis
    if model and tokenizer:
        log_subsection_header(logger, "Knowledge Base Retrieval Analysis")
        try:
            # Initialize KnowledgeBase
            knowledge_base = KnowledgeBase(model.embeddings, tokenizer)

            # Create some dummy documents and embeddings for testing
            dummy_documents = [
                "The sky is blue.",
                "The sun is bright.",
                "The moon is a natural satellite.",
                "The earth is a planet.",
                "The universe is vast."
            ]
            dummy_embeddings = [knowledge_base._embed_text(doc) for doc in dummy_documents]

            # Define a list of test configurations for the retrieve function
            retrieval_configs = [
                {'method': 'cosine', 'use_db': True},
                {'method': 'cosine', 'use_db': False, 'backward_pass_data': {'receptor_field': {'documents': dummy_documents, 'embeddings': dummy_embeddings}}},
                {'method': 'cnn', 'use_db': False, 'backward_pass_data': {'receptor_field': {'documents': dummy_documents, 'embeddings': dummy_embeddings}}},
                {'method': 'genetic', 'use_db': False, 'backward_pass_data': {'receptor_field': {'documents': dummy_documents, 'embeddings': dummy_embeddings}}},
                {'method': 'bayes', 'use_db': False, 'backward_pass_data': {'receptor_field': {'documents': dummy_documents, 'embeddings': dummy_embeddings}, 'hessian_matrix': {'type': 'eigenvalues', 'eigenvalues': np.random.rand(model_config['n_embd']).tolist()}}},
            ]

            for config in retrieval_configs:
                logger.info(f"--- Testing retrieval with config: {config} ---")
                retrieved_docs = knowledge_base.retrieve(
                    query_text=args.prompt,
                    k=3,
                    **config
                )
                logger.info(f"Retrieved documents: {retrieved_docs}")

        except Exception as e:
            logger.error(f"Error during knowledge base retrieval analysis: {e}. Analysis skipped.")

    log_section_header(logger, "Complete Model Evaluation Finished")

if __name__ == "__main__":
    main()