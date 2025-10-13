import sys
import os
import logging

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
sys.path.insert(0, project_root)

import numpy as np
import argparse

from .model import QuantaTissu
from .tokenizer import Tokenizer
from ..config import model_config
from .db.client import TissDBClient
from .embedding.embedder import Embedder
from .knowledge_base import KnowledgeBase

logger = logging.getLogger(__name__)

def generate_text(model: QuantaTissu, tokenizer: Tokenizer, prompt: str, length: int, method: str, temperature: float, top_k: int, top_p: float, repetition_penalty: float = 1.0, bias_token_id=None, bias_strength=0.0, rag_query: str = None, num_retrieved_docs: int = 1, db_port: int = 9876) -> str:
    """
    Generates text of a specified length, starting with a prompt.
    """
    logger.debug(f"Starting text generation with prompt: '{prompt}', length: {length}, method: {method}, temp: {temperature}, top_k: {top_k}, top_p: {top_p}, repetition_penalty: {repetition_penalty}")

    # Tokenize the initial prompt
    # --- RAG Integration ---
    if rag_query:
        logger.info("--- RAG Integration: Retrieving documents ---")
        embedder = Embedder(tokenizer, model.embeddings.value)
        db_client = TissDBClient(db_port=db_port)
        kb = KnowledgeBase(embedder, db_client)
        retrieved_docs = kb.retrieve(rag_query, k=num_retrieved_docs, use_db=True)
        logger.info(f"Retrieved {len(retrieved_docs)} documents.")

        if retrieved_docs:
            retrieved_text = "\n".join(retrieved_docs)
            prompt = f"Based on the following information:\n{retrieved_text}\n\nAnswer the question: {prompt}"
            logger.debug(f"RAG-augmented prompt: {prompt}")
        else:
            logger.warning("No documents retrieved for RAG query. Proceeding with original prompt.")

    logger.debug(f"Tokenizing prompt: '{prompt}'")
    prompt_token_ids = tokenizer.tokenize(prompt)
    if not prompt_token_ids.any(): # Check if the array is not empty
        logger.warning("Prompt is empty or contains only unknown tokens.")
        return ""
    logger.debug(f"Prompt token IDs: {prompt_token_ids.tolist()}")

    # Use the new efficient generate method
    logger.debug("Calling model.generate() for token generation.")
    generated_ids = model.sample(
        prompt_token_ids.tolist(),
        n_new_tokens=length,
        method=method,
        temperature=temperature,
        top_k=top_k,
        top_p=top_p,
        repetition_penalty=repetition_penalty,
        bias_token_id=bias_token_id,
        bias_strength=bias_strength
    )
    logger.debug(f"Generated token IDs from model: {generated_ids}")

    # Detokenize the generated IDs and append to the original prompt.
    logger.debug("Detokenizing generated IDs.")
    generated_text = tokenizer.detokenize(np.array(generated_ids))
    logger.debug(f"Detokenized text: '{generated_text}'")

    final_text = prompt + generated_text
    logger.debug(f"Final generated text: '{final_text}'")
    return final_text

def main():
    """
    Main function to parse arguments and run the text generation.
    """
    logging.basicConfig(level=logging.DEBUG, stream=sys.stdout, format='%(asctime)s - %(levelname)s - %(message)s')
    parser = argparse.ArgumentParser(description="Generate text using the QuantaTissu model.")
    parser.add_argument(
        "--length",
        type=int,
        default=10,
        help="The number of tokens to generate."
    )
    parser.add_argument(
        "--prompt",
        type=str,
        default="hello",
        help="The initial prompt to start generation."
    )
    parser.add_argument(
        "--checkpoint_path",
        type=str,
        required=True,
        help="Path to the model checkpoint (.npz file)."
    )
    # Arguments for inference tuning
    parser.add_argument("--method", type=str, default="nucleus", help="Generation method: greedy, top_k, nucleus, random.")
    parser.add_argument("--temperature", type=float, default=0.8, help="Controls randomness. Higher is more random.")
    parser.add_argument("--top_k", type=int, default=20, help="K for top-k sampling.")
    parser.add_argument("--top_p", type=float, default=0.9, help="P for nucleus sampling.")
    parser.add_argument("--tokenizer_prefix", type=str, default=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'models', 'tokenizers', 'revised_tokenizer'), help="Path to the trained tokenizer files.")
    parser.add_argument("--repetition_penalty", type=float, default=1.0, help="Penalty for repeating tokens.")
    parser.add_argument("--bias_token_id", type=int, default=None, help="Token ID to bias during generation.")
    parser.add_argument("--bias_strength", type=float, default=0.0, help="Strength of the bias.")

    args = parser.parse_args()

    # --- Initialize Tokenizer ---
    # The Tokenizer class automatically loads the default tokenizer files.
    # Make sure you have run tisslm/train_bpe.py first.
    try:
        tokenizer = Tokenizer(tokenizer_prefix=args.tokenizer_prefix)
    except FileNotFoundError as e:
        print(f"Error: Tokenizer files not found. {e}", file=sys.stderr)
        print("Please run `python3 -m quanta_tissu.tisslm.train_bpe` to train and save the tokenizer.", file=sys.stderr) # Updated path
        sys.exit(1)

    # --- Initialize Model ---
    # Update vocab_size in the model config based on the loaded tokenizer
    model_config["vocab_size"] = tokenizer.get_vocab_size()

    np.random.seed(42)  # for reproducibility
    model = QuantaTissu(model_config)

    # --- Load Checkpoint ---
    if not os.path.exists(args.checkpoint_path):
         print(f"Error: Checkpoint file not found at '{args.checkpoint_path}'", file=sys.stderr)
         print("Please run `python3 -m quanta_tissu.tisslm.core.run_training` to train and save a model checkpoint.", file=sys.stderr)
         sys.exit(1)
    model.load_weights(args.checkpoint_path)


    generated_text = generate_text(
        model,
        tokenizer,
        args.prompt,
        args.length,
        args.method,
        args.temperature,
        args.top_k,
        args.top_p,
        args.repetition_penalty,
        args.bias_token_id,
        args.bias_strength
    )
    print(generated_text)

if __name__ == "__main__":
    main()