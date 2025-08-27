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

    def sample(self, prompt_tokens, n_new_tokens, method="greedy", temperature=1.0, top_k=None, top_p=None, repetition_penalty=1.0, eos_id=None):
        """
        Generates a sequence of tokens based on a prompt.
        This version assumes the model manages the KV cache internally and its
        forward method returns only logits.
        """
        print(f"DEBUG: Generator.sample - n_new_tokens: {n_new_tokens}")
        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]

        # First, process the entire prompt to populate the model's internal KV cache.
        # The returned logits are for the *next* token prediction after the prompt.
        # The input shape is (batch_size, seq_len), so we add a batch dimension.
        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)

        for _ in range(n_new_tokens):
            # We only need the logits for the very last token to predict the next one.
            last_logit = logits[:, -1, :]

            # Select the next token based on the specified generation strategy.
            next_token = self._predict_from_logits(
                last_logit,
                method=method,
                temperature=temperature,
                top_k=top_k,
                top_p=top_p,
                past_tokens=current_tokens,
                repetition_penalty=repetition_penalty
            )

            # If an end-of-sequence token is generated, stop.
            if eos_id is not None and next_token == eos_id:
                break

            # Add the new token to our sequences.
            generated_tokens.append(next_token)
            current_tokens.append(next_token)

            # Prepare the input for the next iteration. It's just the new token.
            # The model will use its internal KV cache because start_pos > 0.
            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)

        return generated_tokens

    def _predict_from_logits(self, logits, method="greedy", temperature=1.0, top_k=None, top_p=None, past_tokens=None, repetition_penalty=1.0):
        """
        Selects the next token from logits based on the chosen sampling method.
        Injects telemetry and orchestration hooks.
        """
        logger.debug(f"_predict_from_logits: method={method}, temp={temperature}, top_k={top_k}, top_p={top_p}, logits shape={logits.shape}")
        if logits.ndim > 1:
            logits = np.squeeze(logits)

        # Apply repetition penalty
        if past_tokens and repetition_penalty != 1.0:
            for token_id in set(past_tokens):
                if logits[token_id].item() > 0:
                    logits[token_id] /= repetition_penalty
                else:
                    logits[token_id] *= repetition_penalty

        # --- Telemetry Hook: Log initial logits and probabilities ---
        logger.debug(f"Telemetry: Logits (first 10): {logits[:10].tolist()}")
        
        probs = softmax(logits, temperature=temperature)
        if probs.ndim > 1:
            probs = np.squeeze(probs)
        logger.debug(f"Telemetry: Probabilities (first 10): {probs[:10].tolist()}")

        # --- Orchestration Hook: Apply external policies to probabilities ---
        # This method can be extended to modify 'probs' based on external rules,
        # constraints, or feedback from other agents.
        orchestrated_probs = self._apply_orchestration_policy(probs, method, temperature, top_k, top_p, past_tokens, repetition_penalty)
        if not np.array_equal(probs, orchestrated_probs):
            logger.debug(f"Orchestration: Probabilities modified by policy. New probs (first 10): {orchestrated_probs[:10].tolist()}")
        probs = orchestrated_probs # Use the potentially modified probabilities

        next_token = -1 # Initialize with an invalid token ID

        if method == "greedy":
            next_token = int(np.argmax(probs)) # Use probs here, not logits
            logger.debug(f"Telemetry: Greedy selected token ID: {next_token}, probability: {probs[next_token]:.4f}")

        elif method == "top_k":
            if top_k is None:
                raise ConfigurationError("top_k must be specified for top_k sampling")
            top_k_indices = np.argsort(probs)[-top_k:]
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            next_token = np.random.choice(len(probs), p=top_k_probs)
            logger.debug(f"Telemetry: Top-K selected token ID: {next_token}, probability: {probs[next_token]:.4f}")
            logger.debug(f"Telemetry: Top-K indices: {top_k_indices.tolist()}")
            logger.debug(f"Telemetry: Top-K probabilities (normalized): {top_k_probs[top_k_indices].tolist()}")


        elif method == "nucleus":
            if top_p is None:
                raise ConfigurationError("top_p must be specified for nucleus sampling")

            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)
            
            logger.debug(f"Telemetry: Nucleus sorted indices (first 10): {sorted_indices[:10].tolist()}")
            logger.debug(f"Telemetry: Nucleus sorted probabilities (first 10): {sorted_probs[:10].tolist()}")
            logger.debug(f"Telemetry: Nucleus cumulative probabilities (first 10): {cumulative_probs[:10].tolist()}")

            cutoff_idx = np.where(cumulative_probs >= top_p)[0][0]
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]

            nucleus_probs = np.zeros_like(probs)
            nucleus_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]

            if np.sum(nucleus_probs) == 0:
                raise ModelProcessingError("Nucleus sampling resulted in all probabilities being zeroed out.")

            nucleus_probs /= np.sum(nucleus_probs)
            next_token = np.random.choice(len(probs), p=nucleus_probs)
            logger.debug(f"Telemetry: Nucleus selected token ID: {next_token}, probability: {probs[next_token]:.4f}")
            logger.debug(f"Telemetry: Nucleus tokens to keep indices: {tokens_to_keep_indices.tolist()}")
            logger.debug(f"Telemetry: Nucleus probabilities (normalized): {nucleus_probs[tokens_to_keep_indices].tolist()}")


        elif method == "random" or method == "sampling":
             next_token = np.random.choice(len(probs), p=probs.flatten())
             logger.debug(f"Telemetry: Random selected token ID: {next_token}, probability: {probs[next_token]:.4f}")

        else:
            raise ConfigurationError(f"Unknown sampling method: {method}")
        
        return next_token

    def _apply_orchestration_policy(self, probs, method, temperature, top_k, top_p, past_tokens, repetition_penalty):
        """
        Placeholder for applying external orchestration policies to token probabilities.
        This method can be extended to implement custom logic for influencing token selection.
        For now, it simply returns the probabilities unchanged.
        """
        # Example: You could modify 'probs' here based on some external state or rules.
        # e.g., penalize certain tokens, boost others, or enforce specific sequences.
        return probs