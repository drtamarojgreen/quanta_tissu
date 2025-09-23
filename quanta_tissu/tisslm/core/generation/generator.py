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

    def sample(self, prompt_tokens, n_new_tokens, method="greedy", temperature=1.0, top_k=None, top_p=None, repetition_penalty=1.0, eos_id=None, **kwargs):
        """
        Generates a sequence of tokens based on a prompt.
        This version assumes the model manages the KV cache internally and its
        forward method returns only logits.
        """
        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]

        # Extract sentiment parameters if adaptive_sentiment method is used
        sentiment_analyzer = kwargs.pop('sentiment_analyzer', None)
        target_sentiment = kwargs.pop('target_sentiment', None)
        target_strength = kwargs.pop('target_strength', None)

        sentiment_bias = None
        if method == "adaptive_sentiment" and sentiment_analyzer:
            sentiment_bias = sentiment_analyzer.get_sentiment_bias(target_sentiment, target_strength)
            logger.debug(f"Applying adaptive sentiment bias for {target_sentiment} with strength {target_strength}")


        # First, process the entire prompt to populate the model's internal KV cache.
        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)

        for _ in range(n_new_tokens):
            last_logit = logits[:, -1, :]

            # Select the next token based on the specified generation strategy.
            next_token, _ = self._predict_from_logits(
                last_logit,
                method=method,
                temperature=temperature,
                top_k=top_k,
                top_p=top_p,
                past_tokens=current_tokens,
                repetition_penalty=repetition_penalty,
                sentiment_bias=sentiment_bias # Pass sentiment bias
            )

            if eos_id is not None and next_token == eos_id:
                break

            generated_tokens.append(next_token)
            current_tokens.append(next_token)

            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)

        return generated_tokens

    def _predict_from_logits(self, logits, method="greedy", temperature=1.0, top_k=None, top_p=None, past_tokens=None, repetition_penalty=1.0, sentiment_bias=None):
        """
        Selects the next token from logits based on the chosen sampling method.
        """
        logger.debug(f"_predict_from_logits: method={method}, temp={temperature}, top_k={top_k}, top_p={top_p}, logits shape={logits.shape}")
        if logits.ndim > 1:
            logits = np.squeeze(logits)

        # Apply repetition penalty
        if past_tokens and repetition_penalty != 1.0:
            for token_id in set(past_tokens):
                if token_id < len(logits):
                    if logits[token_id].item() > 0:
                        logits[token_id] /= repetition_penalty
                    else:
                        logits[token_id] *= repetition_penalty

        # Apply sentiment bias before softmax
        if sentiment_bias:
            for token_id, bias in sentiment_bias.items():
                if token_id < len(logits):
                    logits[token_id] += bias

        probs = softmax(logits, temperature=temperature)
        if probs.ndim > 1:
            probs = np.squeeze(probs)

        next_token = -1 # Initialize with an invalid token ID

        if method == "greedy":
            next_token = np.argmax(probs).item()

        elif method == "top_k":
            if top_k is None:
                raise ConfigurationError("top_k must be specified for top_k sampling")
            top_k_indices = np.argsort(probs)[-top_k:]
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            next_token = np.random.choice(len(probs), p=top_k_probs)

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
                # Fallback for safety, though unlikely with correct logic
                next_token = tokens_to_keep_indices[0]
            else:
                nucleus_probs /= np.sum(nucleus_probs)
                next_token = np.random.choice(len(probs), p=nucleus_probs)

        elif method == "random" or method == "sampling":
             next_token = np.random.choice(len(probs), p=np.array(probs))

        elif method == "adaptive_sentiment":
            # Sentiment bias is already applied before this point.
            # We can default to greedy or nucleus sampling after bias application.
            # For now, let's default to greedy after bias.
            next_token = np.argmax(probs).item()

        else:
            raise ConfigurationError(f"Unknown sampling method: {method}")
        
        return int(next_token), probs