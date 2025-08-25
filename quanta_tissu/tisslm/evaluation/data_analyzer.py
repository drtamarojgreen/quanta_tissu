import logging
from collections import Counter
from typing import List, Dict, Any
import numpy as np

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.data import load_corpus



def analyze_corpus_statistics(corpus_path: str, tokenizer):
    """
    Analyzes the statistics of the training corpus.

    Args:
        corpus_path: Path to the training corpus directory.
        tokenizer: The tokenizer used for the model.
    """
    logging.info(f"--- Analyzing Corpus Statistics from: {corpus_path} ---")
    try:
        token_ids = load_corpus(corpus_path, tokenizer)
        logging.info(f"Total tokens in corpus: {len(token_ids)}")

        # Vocabulary size
        vocab_size = tokenizer.get_vocab_size()
        logging.info(f"Tokenizer vocabulary size: {vocab_size}")

        # Token frequency distribution
        token_counts = Counter(token_ids)
        logging.info(f"Most common tokens: {token_counts.most_common(20)}")
        logging.info(f"Least common tokens (excluding 0-count): {token_counts.most_common()[:-21:-1]}")

        # Percentage of unique tokens
        unique_tokens = len(token_counts)
        logging.info(f"Number of unique tokens: {unique_tokens} ({unique_tokens / vocab_size:.2%} of vocabulary)")

        # Check for unknown tokens if tokenizer supports it
        # if hasattr(tokenizer, 'unk_token_id') and tokenizer.unk_token_id in token_counts:
        #     logging.warning(f"Unknown token count: {token_counts[tokenizer.unk_token_id]}")

    except Exception as e:
        logging.error(f"Error analyzing corpus: {e}")

def analyze_dataset_batch(dataset_batch: List[List[int]], tokenizer):
    """
    Analyzes a single batch from the dataset.

    Args:
        dataset_batch: A batch of token IDs (e.g., x_batch or y_batch).
        tokenizer: The tokenizer used for the model.
    """
    logging.info("\n--- Analyzing Dataset Batch ---")
    if not dataset_batch:
        logging.info("Batch is empty.")
        return

    logging.info(f"Batch shape: {np.array(dataset_batch).shape}")

    # Decode a few samples from the batch
    for i, sample_ids in enumerate(dataset_batch[:min(3, len(dataset_batch))]):
        try:
            decoded_text = tokenizer.detokenize(sample_ids)
            logging.info(f"Sample {i+1} (decoded): {decoded_text[:100]}...") # Limit output length
        except Exception as e:
            logging.error(f"Error decoding sample {i+1}: {e}")

    # Check for padding tokens if applicable
    # if hasattr(tokenizer, 'pad_token_id'):
    #     pad_count = sum(1 for seq in dataset_batch for token_id in seq if token_id == tokenizer.pad_token_id)
    #     logging.info(f"Padding token count in batch: {pad_count}")

# You can add more functions for specific data analysis, e.g.,
# - visualize_token_distribution(token_counts)
# - analyze_sequence_lengths(dataset)
