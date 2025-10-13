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
from quanta_tissu.tisslm.config import model_config

# Setup basic logging for this script
logging.basicConfig(
    level=logging.DEBUG,
    stream=sys.stdout,
    format='%(asctime)s - %(levelname)s - %(message)s',
    encoding='utf-8'  # Ensure UTF-8 encoding for all log output, preventing errors on redirection.
)
logger = logging.getLogger(__name__)

def main():
    sys.stdout.reconfigure(encoding='utf-8')

    # Paths should have sensible defaults if possible, or be clearly documented
    # For now, making them optional with a warning if not found.
    tokenizer_prefix = "tokenizer/trained_tokenizer"
    checkpoint_path = "C:/Users/tamar/Documents/DataAnnotation/Gemini/quanta_tissu/checkpoints/checkpoint_step_115500.npz"
    db_port = 9876

    logger.info("--- Starting Algorithmic Generation Suite ---")

    logger.info("Initializing Tokenizer...")
    tokenizer_full_prefix = os.path.normpath(os.path.join(project_root, tokenizer_prefix))
    try:
        tokenizer = Tokenizer(tokenizer_prefix=tokenizer_full_prefix)
    except FileNotFoundError as e:
        logger.error(f"Error: Tokenizer files not found at {tokenizer_full_prefix}. {e}")
        logger.error("Please ensure the tokenizer is trained and available at the specified path.")
        sys.exit(1)

    logger.info("Initializing Model...")
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)

    logger.info(f"Loading model weights from {checkpoint_path}...")
    checkpoint_full_path = os.path.normpath(os.path.join(project_root, checkpoint_path))
    if not os.path.exists(checkpoint_full_path):
        logger.error(f"Error: Checkpoint file not found at '{checkpoint_full_path}'")
        sys.exit(1)
    model.load_weights(checkpoint_full_path)
    logger.info("Model loaded successfully.")

    test_configurations = [
        # Basic Greedy
        {"prompt": "The meaning of life is", "length": 50, "method": "greedy"},
        # Greedy with Temperature
        {"prompt": "The meaning of life is", "length": 50, "method": "greedy_with_temp", "temperature": 0.8},
        {"prompt": "The meaning of life is", "length": 50, "method": "greedy_with_temp", "temperature": 1.2},
        # Top-K Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "top_k", "top_k": 10},
        {"prompt": "The quick brown fox", "length": 50, "method": "top_k", "top_k": 50},
        # Nucleus (Top-P) Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "nucleus", "top_p": 0.9},
        {"prompt": "The quick brown fox", "length": 50, "method": "nucleus", "top_p": 0.95},
        # Nucleus with Temperature
        {"prompt": "The quick brown fox", "length": 50, "method": "nucleus_with_temp", "top_p": 0.9, "temperature": 0.7},
        # Beam Search
        {"prompt": "The quick brown fox", "length": 50, "method": "beam", "beam_width": 3},
        {"prompt": "The quick brown fox", "length": 50, "method": "beam", "beam_width": 5},
        # Mirostat Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "mirostat", "tau": 5.0, "eta": 0.1},
        {"prompt": "The quick brown fox", "length": 50, "method": "mirostat", "tau": 2.0, "eta": 0.5},
        # Typical Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "typical_sampling"},
        # Stacked Configurations
        # Stacked Configurations
        {"prompt": "The quick brown fox", "length": 50, "method": ["top_k", "nucleus"], "top_k": 50, "top_p": 0.9},
        {"prompt": "The quick brown fox", "length": 50, "method": ["greedy_with_temp", "top_k"], "temperature": 0.8, "top_k": 20},
        # RAG Integration
        {"prompt": "What is a Gromp?", "length": 50, "method": "greedy", "rag_query": "Gromp characteristics", "num_retrieved_docs": 1, "db_port": db_port},
        {"prompt": "What is a Gromp's diet?", "length": 50, "method": "greedy", "rag_query": "Gromp diet", "num_retrieved_docs": 1, "db_port": db_port},
        {"prompt": "Where do Gromps live?", "length": 50, "method": "greedy", "rag_query": "Gromp habitat", "num_retrieved_docs": 1, "db_port": db_port},
        # More Greedy with Temperature
        {"prompt": "The meaning of life is", "length": 50, "method": "greedy_with_temp", "temperature": 0.5},
        {"prompt": "The meaning of life is", "length": 50, "method": "greedy_with_temp", "temperature": 1.5},
        # More Top-K Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "top_k", "top_k": 5},
        {"prompt": "The quick brown fox", "length": 50, "method": "top_k", "top_k": 100},
        # More Nucleus (Top-P) Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "nucleus", "top_p": 0.8},
        {"prompt": "The quick brown fox", "length": 50, "method": "nucleus", "top_p": 0.99},
        # More Beam Search
        {"prompt": "The quick brown fox", "length": 50, "method": "beam", "beam_width": 2},
        {"prompt": "The quick brown fox", "length": 50, "method": "beam", "beam_width": 7},
        # More Mirostat Sampling
        {"prompt": "The quick brown fox", "length": 50, "method": "mirostat", "tau": 3.0, "eta": 0.2},
        # More Stacked Configurations
        {"prompt": "The quick brown fox", "length": 50, "method": ["nucleus", "greedy_with_temp"], "top_p": 0.9, "temperature": 0.6},
        # More RAG Integration
        {"prompt": "What is the capital of France?", "length": 50, "method": "greedy", "rag_query": "capital of France", "num_retrieved_docs": 1, "db_port": db_port},
    ]

    output_file_path = os.path.join(project_root, "out3.txt")
    with open(output_file_path, "w", encoding="utf-8") as f:
        for i, config in enumerate(test_configurations):
            logger.info(f"\n--- Running Test Configuration {i+1}/{len(test_configurations)} ---")
            
            prompt = config.get("prompt")
            length = config.get("length")
            method = config.get("method")
            temperature = config.get("temperature", 1.0)
            top_k = config.get("top_k")
            top_p = config.get("top_p")
            repetition_penalty = config.get("repetition_penalty", 1.0)
            bias_token_id = config.get("bias_token_id")
            bias_strength = config.get("bias_strength")
            beam_width = config.get("beam_width", 3)
            tau = config.get("tau", 5.0)
            eta = config.get("eta", 0.1)
            rag_query = config.get("rag_query")
            num_retrieved_docs = config.get("num_retrieved_docs", 1)
            db_port = config.get("db_port", 9876)

            f.write(f"\n--- Test Configuration {i+1} ---\n")
            f.write(f"Method: {method}\n")
            f.write(f"Prompt: {prompt}\n")
            if rag_query: f.write(f"RAG Query: {rag_query}\n")
            f.write(f"Length: {length}\n")
            if isinstance(method, list): f.write(f"Stacked Methods: {method}\n")
            if temperature != 1.0: f.write(f"Temperature: {temperature}\n")
            if top_k is not None: f.write(f"Top-K: {top_k}\n")
            if top_p is not None: f.write(f"Top-P: {top_p}\n")
            if repetition_penalty != 1.0: f.write(f"Repetition Penalty: {repetition_penalty}\n")
            if bias_token_id is not None: f.write(f"Bias Token ID: {bias_token_id}\n")
            if bias_strength != 0.0: f.write(f"Bias Strength: {bias_strength}\n")
            if beam_width != 3: f.write(f"Beam Width: {beam_width}\n")
            if tau != 5.0: f.write(f"Tau: {tau}\n")
            if eta != 0.1: f.write(f"Eta: {eta}\n")
            if num_retrieved_docs != 1: f.write(f"Num Retrieved Docs: {num_retrieved_docs}\n")
            if db_port != 9876: f.write(f"DB Port: {db_port}\n")

            try:
                generated_text = generate_text(
                    model=model,
                    tokenizer=tokenizer,
                    prompt=prompt,
                    length=length,
                    method=method,
                    temperature=temperature,
                    top_k=top_k,
                    top_p=top_p,
                    repetition_penalty=repetition_penalty,
                    bias_token_id=bias_token_id,
                    bias_strength=bias_strength,
                    beam_width=beam_width,
                    tau=tau,
                    eta=eta,
                    rag_query=rag_query,
                    num_retrieved_docs=num_retrieved_docs,
                    db_port=db_port
                )
                f.write(f"Generated Text:\n{generated_text}\n")
            except Exception as e:
                f.write(f"Error during generation: {e}\n")
            f.write("----------------------\n")

    logger.info("Algorithmic Generation Suite Finished. Results written to out3.txt")

if __name__ == "__main__":
    main()
