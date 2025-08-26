import numpy as np
import logging

from ..layers import softmax
from ..model_error_handler import ConfigurationError, ModelProcessingError

logger = logging.getLogger(__name__)

class Generator:
    """
    Handles text generation and various sampling strategies.

    This class takes a trained model and uses it to generate new token sequences.
    """
    def __init__(self, model):
        """
        Initializes the Generator with a model instance.

        Args:
            model: An instance of `quanta_tissu.tisslm.core.architecture.llm.Model`.
        """
        self.model = model

    def _predict_from_logits(self, logits, method="greedy", temperature=1.0, top_k=None, top_p=None, past_tokens=None, repetition_penalty=1.0):
        """
        Selects the next token from logits based on the chosen sampling method.
        """
        logger.debug(f"_predict_from_logits: method={method}, temp={temperature}, top_k={top_k}, top_p={top_p}, logits shape={logits.shape}")
        if logits.ndim > 1:
            logits = np.squeeze(logits)

        # Apply repetition penalty
        if past_tokens and repetition_penalty != 1.0:
            for token_id in set(past_tokens):
                if logits[token_id] > 0:
                    logits[token_id] /= repetition_penalty
                else:
                    logits[token_id] *= repetition_penalty

        if method == "greedy":
            return int(np.argmax(logits))

        probs = softmax(logits, temperature=temperature)
        if probs.ndim > 1:
            probs = np.squeeze(probs)

        if method == "top_k":
            if top_k is None:
                raise ConfigurationError("top_k must be specified for top_k sampling")
            top_k_indices = np.argsort(probs)[-top_k:]
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            return np.random.choice(len(probs), p=top_k_probs)

        elif method == "nucleus":
            if top_p is None:
                raise ConfigurationError("top_p must be specified for nucleus sampling")

            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)

            cutoff_idx = np.where(cumulative_probs >= top_p)[0][0]
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]

            nucleus_probs = np.zeros_like(probs)
            nucleus_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]

            if np.sum(nucleus_probs) == 0:
                raise ModelProcessingError("Nucleus sampling resulted in all probabilities being zeroed out.")

            nucleus_probs /= np.sum(nucleus_probs)
            return np.random.choice(len(probs), p=nucleus_probs)

        elif method == "random" or method == "sampling":
             return np.random.choice(len(probs), p=probs.flatten())

        else:
            raise ConfigurationError(f"Unknown sampling method: {method}")

    def generate(self, prompt_tokens, n_new_tokens, method="greedy", temperature=1.0, top_k=None, top_p=None, use_cache=True, repetition_penalty=1.0):
        """
        Generates a sequence of tokens starting from a prompt.
        """
        logger.debug(f"Starting generation: prompt_tokens_len={len(prompt_tokens)}, n_new_tokens={n_new_tokens}, method={method}")

        if not use_cache:
            # This is an inefficient way to generate without cache, kept for compatibility.
            # A proper implementation would still batch where possible.
            generated_ids = []
            current_tokens = list(prompt_tokens)
            for _ in range(n_new_tokens):
                token_ids_np = np.array([current_tokens])
                logits = self.model.forward(token_ids_np, start_pos=0) # Re-evaluating all tokens each time
                next_token_id = self._predict_from_logits(logits[:, -1, :], method, temperature, top_k, top_p, past_tokens=current_tokens, repetition_penalty=repetition_penalty)
                generated_ids.append(next_token_id)
                current_tokens.append(next_token_id)
            return generated_ids

        # 1. Initialize KV cache for each transformer block
        kv_cache = [{} for _ in self.model.transformer_blocks]

        # 2. Process the prompt tokens to populate the cache
        prompt_token_ids = np.array(prompt_tokens)[np.newaxis, :]
        prompt_logits = self.model.forward(prompt_token_ids, kv_cache=kv_cache, start_pos=0)

        # 3. Predict the first token
        next_token_id = self._predict_from_logits(prompt_logits[:, -1, :], method, temperature, top_k, top_p, past_tokens=prompt_tokens, repetition_penalty=repetition_penalty)

        generated_ids = [next_token_id]

        # 4. Sequentially generate remaining tokens using the cache
        for i in range(n_new_tokens - 1):
            current_token_id = np.array([[next_token_id]])
            start_pos = len(prompt_tokens) + i

            logits = self.model.forward(current_token_id, kv_cache=kv_cache, start_pos=start_pos)

            all_past_tokens = list(prompt_tokens) + generated_ids
            next_token_id = self._predict_from_logits(logits[:, -1, :], method, temperature, top_k, top_p, past_tokens=all_past_tokens, repetition_penalty=repetition_penalty)
            generated_ids.append(next_token_id)

        logger.debug(f"Final generated IDs: {generated_ids}")
        return generated_ids
