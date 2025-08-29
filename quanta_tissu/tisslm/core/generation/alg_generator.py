import numpy as np
import logging

from ..layers import softmax
from ..model_error_handler import ConfigurationError, ModelProcessingError
from ..retrieval.strategy import CNNSimilarityStrategy, BayesianSimilarityStrategy, CosineSimilarityStrategy, GeneticSimilarityStrategy

logger = logging.getLogger(__name__)

class AlgorithmicGenerator:
    """
    Handles text generation and various sampling strategies.

    This class takes a trained model and uses it to generate new token sequences.
    """
    def __init__(self, model, config):
        """
        Initializes the Generator with a model instance.

        Args:
            model: An instance of `quanta_tissu.tisslm.core.architecture.llm.Model`.
        """
        self.model = model
        # Initialize similarity strategies
        embedding_dim = config.get('embedding_dim', 128) # Example: get from model config
        self.cnn_strategy = CNNSimilarityStrategy(embedding_dim=embedding_dim)
        self.bayesian_strategy = BayesianSimilarityStrategy()
        self.cosine_strategy = CosineSimilarityStrategy()
        self.genetic_strategy = GeneticSimilarityStrategy()
        # Note: BayesianStrategy might need a way to get 'hessian_matrix'
        # This will require further design on how to provide this context during generation.

    def sample(self, prompt_tokens, n_new_tokens, method="greedy", temperature=1.0, top_k=None, top_p=None, repetition_penalty=1.0, eos_id=None, bias_token_id=None, bias_strength=0.0, query_embedding=None, hessian_matrix=None, beam_width=3, tau=5.0, eta=0.1):
        if method == "beam":
            return self.beam_search(prompt_tokens, n_new_tokens, beam_width, temperature, top_k, top_p, repetition_penalty, eos_id, bias_token_id, bias_strength)
        if method == "mirostat":
            return self.mirostat_sampling(prompt_tokens, n_new_tokens, tau, eta, temperature, repetition_penalty, eos_id, bias_token_id, bias_strength)
        """
        Generates a sequence of tokens based on a prompt.
        This version assumes the model manages the KV cache internally and its
        forward method returns only logits.
        """
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
            next_token, _ = self._predict_from_logits(
                last_logit,
                method=method,
                temperature=temperature,
                top_k=top_k,
                top_p=top_p,
                past_tokens=current_tokens,
                repetition_penalty=repetition_penalty,
                bias_token_id=bias_token_id,
                bias_strength=bias_strength,
                query_embedding=query_embedding,
                hessian_matrix=hessian_matrix
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

    def beam_search(self, prompt_tokens, n_new_tokens, beam_width, temperature, top_k, top_p, repetition_penalty, eos_id, bias_token_id, bias_strength):
        """Performs beam search to generate a sequence of tokens."""
        # Initialize beams. Each beam is a tuple of (sequence, score)
        beams = [([int(t) for t in prompt_tokens], 0.0)]

        for _ in range(n_new_tokens):
            all_candidates = []
            for seq, score in beams:
                if eos_id is not None and seq[-1] == eos_id:
                    all_candidates.append((seq, score))
                    continue

                prompt_array = np.array([seq])
                logits, _ = self.model.forward(prompt_array, start_pos=0)
                last_logit = logits[:, -1, :]
                logger.debug(f"Beam Search: last_logit shape: {last_logit.shape}")

                # Get top k probabilities and their indices
                probs = softmax(last_logit, temperature=temperature)
                logger.debug(f"Beam Search: probs shape: {probs.shape}")
                logger.debug(f"Beam Search: probs (first 10): {probs[:10].tolist()}")

                # Ensure beam_width does not exceed the number of available probabilities
                effective_beam_width = min(beam_width, len(probs))
                if effective_beam_width == 0:
                    logger.warning("Effective beam width is 0. Skipping this beam.")
                    continue

                top_k_indices = np.argsort(probs)[-effective_beam_width:]
                top_k_probs = probs[top_k_indices]

                for i in range(len(top_k_indices)):
                    token = top_k_indices[i]
                    prob = top_k_probs[i]
                    new_seq = seq + [token]
                    new_score = score - np.log(prob)  # Add log probability
                    all_candidates.append((new_seq, new_score))

            # Sort all candidates by score and select top `beam_width`
            ordered = sorted(all_candidates, key=lambda tup: tup[1])
            beams = ordered[:beam_width]

        # Return the best sequence (lowest score)
        best_seq, _ = beams[0]
        return best_seq[len(prompt_tokens):]

    def mirostat_sampling(self, prompt_tokens, n_new_tokens, tau, eta, temperature, repetition_penalty, eos_id, bias_token_id, bias_strength):
        """Performs Mirostat sampling to generate a sequence of tokens."""
        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]
        mu = 2 * tau

        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)

        for _ in range(n_new_tokens):
            last_logit = logits[:, -1, :]
            next_token, mu = self._predict_from_logits(
                last_logit,
                method='mirostat',
                temperature=temperature,
                past_tokens=current_tokens,
                repetition_penalty=repetition_penalty,
                bias_token_id=bias_token_id,
                bias_strength=bias_strength,
                tau=tau,
                eta=eta,
                mu=mu
            )

            if eos_id is not None and next_token == eos_id:
                break

            generated_tokens.append(next_token)
            current_tokens.append(next_token)

            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)

        return generated_tokens

    def _predict_from_logits(self, logits, method="greedy", temperature=1.0, top_k=None, top_p=None, past_tokens=None, repetition_penalty=1.0, bias_token_id=None, bias_strength=0.0, **kwargs):
        """
        Selects the next token from logits based on the chosen sampling method.
        Injects telemetry and orchestration hooks.
        """
        if isinstance(method, str):
            methods = [method]
        else:
            methods = method

        logger.debug(f"_predict_from_logits: methods={methods}, temp={temperature}, top_k={top_k}, top_p={top_p}, logits shape={logits.shape}")
        if logits.ndim > 1:
            logits = np.squeeze(logits)

        # Apply repetition penalty
        if past_tokens and repetition_penalty != 1.0:
            logger.debug(f"Repetition Penalty: Applying penalty. past_tokens: {past_tokens}")
            logger.debug(f"Repetition Penalty: Logits before penalty (first 10): {logits[:10].tolist()}")
            for token_id in set(past_tokens):
                if token_id < len(logits):
                    if logits[token_id].item() > 0:
                        logits[token_id] /= repetition_penalty
                    else:
                        logits[token_id] *= repetition_penalty
            logger.debug(f"Repetition Penalty: Logits after penalty (first 10): {logits[:10].tolist()}")

        # --- Telemetry Hook: Log initial logits and probabilities ---
        logger.debug(f"Telemetry: Logits (first 10): {logits[:10].tolist()}")
        
        print(f"DEBUG: _predict_from_logits - Logits before softmax: {logits[:25].tolist()}") # Print first 25 logits
        probs = softmax(logits, temperature=temperature)
        if probs.ndim > 1:
            probs = np.squeeze(probs)
        print(f"DEBUG: _predict_from_logits - Probs after softmax: {probs[:25].tolist()}") # Print first 25 probs
        logger.debug(f"Telemetry: Probabilities (first 10): {probs[:10].tolist()}")

        # --- Orchestration Hook: Apply external policies to probabilities ---
        # This method can be extended to modify \'probs\' based on external rules,
        # constraints, or feedback from other agents.
        orchestrated_probs = self._apply_orchestration_policy(probs, methods, temperature, top_k, top_p, past_tokens, repetition_penalty, bias_token_id, bias_strength)
        if not np.array_equal(probs, orchestrated_probs):
            logger.debug(f"Orchestration: Probabilities modified by policy. New probs (first 10): {orchestrated_probs[:10].tolist()}")
        probs = orchestrated_probs # Use the potentially modified probabilities

        for method in methods:
            # --- New Integration for CNN/Bayesian Influenced Sampling ---
            if method == "cnn_influenced":
                query_embedding = kwargs.get('query_embedding')
                if query_embedding is None:
                    raise ValueError("cnn_influenced method requires 'query_embedding' in kwargs.")

                all_vocab_embeddings = self.model.embeddings.value # Shape: (vocab_size, embedding_dim)
                
                similarity_scores = self.cnn_strategy.calculate_similarity(query_embedding, all_vocab_embeddings)
                
                scaled_similarity = similarity_scores * kwargs.get('cnn_influence_scale', 1.0)
                
                logger.debug(f"CNN Influence: Logits before influence: {logits[:10].tolist()}")
                logger.debug(f"CNN Influence: Scaled similarity (first 10): {scaled_similarity[:10].tolist()}")

                logits = logits + scaled_similarity
                probs = softmax(logits, temperature=temperature) # Re-calculate probabilities
                logger.debug(f"CNN Influence: Logits after influence: {logits[:10].tolist()}")
                logger.debug(f"CNN Influence: Probs after influence (first 10): {probs[:10].tolist()}")

                next_token = int(np.argmax(probs)) # Default to greedy after influence

            elif method == "bayesian_influenced":
                query_embedding = kwargs.get('query_embedding')
                hessian_matrix = kwargs.get('hessian_matrix')
                if query_embedding is None or hessian_matrix is None:
                    raise ValueError("bayesian_influenced method requires 'query_embedding' and 'hessian_matrix' in kwargs.")

                all_vocab_embeddings = self.model.embeddings.value # Shape: (vocab_size, embedding_dim)
                
                similarity_scores = self.bayesian_strategy.calculate_similarity(query_embedding, all_vocab_embeddings, hessian_matrix=hessian_matrix)
                
                scaled_similarity = similarity_scores * kwargs.get('bayesian_influence_scale', 1.0)
                
                logits = logits + scaled_similarity
                probs = softmax(logits, temperature=temperature) # Re-calculate probabilities
                next_token = int(np.argmax(probs)) # Default to greedy after influence

            elif method == "cosine_influenced":
                query_embedding = kwargs.get('query_embedding')
                if query_embedding is None:
                    raise ValueError("cosine_influenced method requires 'query_embedding' in kwargs.")

                all_vocab_embeddings = self.model.embeddings.value # Shape: (vocab_size, embedding_dim)
                
                similarity_scores = self.cosine_strategy.calculate_similarity(query_embedding, all_vocab_embeddings)
                
                scaled_similarity = similarity_scores * kwargs.get('cosine_influence_scale', 1.0)
                
                logits = logits + scaled_similarity
                probs = softmax(logits, temperature=temperature) # Re-calculate probabilities
                next_token = int(np.argmax(probs)) # Default to greedy after influence

            elif method == "genetic_influenced":
                query_embedding = kwargs.get('query_embedding')
                if query_embedding is None:
                    raise ValueError("genetic_influenced method requires 'query_embedding' in kwargs.")

                all_vocab_embeddings = self.model.embeddings.value # Shape: (vocab_size, embedding_dim)
                
                similarity_scores = self.genetic_strategy.calculate_similarity(query_embedding, all_vocab_embeddings)
                
                scaled_similarity = similarity_scores * kwargs.get('genetic_influence_scale', 1.0)
                
                logits = logits + scaled_similarity
                probs = softmax(logits, temperature=temperature) # Re-calculate probabilities
                next_token = int(np.argmax(probs)) # Default to greedy after influence

            else:
                next_token = -1 # Initialize with an invalid token ID

                if method == "greedy":
                    next_token = int(np.argmax(probs)) # Use probs here, not logits
                    logger.debug(f"Telemetry: Greedy selected token ID: {next_token}, probability: {probs[next_token]:.4f}")

                elif method == "greedy_with_temp":
                    logits_with_temp = logits / temperature
                    probs_with_temp = softmax(logits_with_temp)
                    next_token = int(np.argmax(probs_with_temp))
                    logger.debug(f"Telemetry: Greedy with Temp selected token ID: {next_token}, probability: {probs_with_temp[next_token]:.4f}")

                elif method == "top_k":
                    if top_k is None:
                        raise ConfigurationError("top_k must be specified for top_k sampling")
                    top_k_indices = np.argsort(probs)[-top_k:]
                    top_k_probs = np.zeros_like(probs)
                    top_k_probs[top_k_indices] = probs[top_k_indices]
                    probs = top_k_probs / np.sum(top_k_probs)
                    next_token = np.random.choice(len(probs), p=probs)
                    logger.debug(f"Telemetry: Top-K selected token ID: {next_token}, probability: {probs[next_token]:.4f}")
                    logger.debug(f"Telemetry: Top-K indices: {top_k_indices.tolist()}")
                    logger.debug(f"Telemetry: Top-K probabilities (normalized): {probs[top_k_indices].tolist()}")


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

                    probs = nucleus_probs / np.sum(nucleus_probs)
                    next_token = np.random.choice(len(probs), p=probs)
                    logger.debug(f"Telemetry: Nucleus selected token ID: {next_token}, probability: {probs[next_token]:.4f}")
                    logger.debug(f"Telemetry: Nucleus tokens to keep indices: {tokens_to_keep_indices.tolist()}")
                    logger.debug(f"Telemetry: Nucleus probabilities (normalized): {probs[tokens_to_keep_indices].tolist()}")


                elif method == "nucleus_with_temp":
                    if top_p is None:
                        raise ConfigurationError("top_p must be specified for nucleus sampling")
                    logits_with_temp = logits / temperature
                    probs_with_temp = softmax(logits_with_temp)
                    sorted_indices = np.argsort(probs_with_temp)[::-1]
                    sorted_probs = probs_with_temp[sorted_indices]
                    cumulative_probs = np.cumsum(sorted_probs)
                    cutoff_idx = np.where(cumulative_probs >= top_p)[0][0]
                    tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
                    nucleus_probs = np.zeros_like(probs)
                    nucleus_probs[tokens_to_keep_indices] = probs_with_temp[tokens_to_keep_indices]
                    probs = nucleus_probs / np.sum(nucleus_probs)
                    next_token = np.random.choice(len(probs), p=probs)
                    logger.debug(f"Telemetry: Nucleus with Temp selected token ID: {next_token}, probability: {probs[next_token]:.4f}")

                elif method == "typical_sampling":
                    # Entropy calculation
                    entropy = -np.sum(probs * np.log2(probs + 1e-9))
                    
                    # Absolute difference between negative log-likelihood and entropy
                    abs_diff = np.abs(-np.log2(probs + 1e-9) - entropy)
                    
                    # Create a mask for typical tokens
                    mask = abs_diff <= 2.0  # Threshold can be a parameter
                    
                    typical_probs = np.zeros_like(probs)
                    typical_probs[mask] = probs[mask]
                    
                    if np.sum(typical_probs) == 0:
                        # Fallback to nucleus sampling if no typical tokens are found
                        sorted_indices = np.argsort(probs)[::-1]
                        sorted_probs = probs[sorted_indices]
                        cumulative_probs = np.cumsum(sorted_probs)
                        cutoff_idx = np.where(cumulative_probs >= 0.1)[0][0] # Small p
                        tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
                        typical_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]

                    probs = typical_probs / np.sum(typical_probs)
                    next_token = np.random.choice(len(probs), p=probs)
                    logger.debug(f"Telemetry: Typical Sampling selected token ID: {next_token}, probability: {probs[next_token]:.4f}")

                elif method == "random" or method == "sampling":
                     print(f"DEBUG: _predict_from_logits - Probs before choice: {probs[:25].tolist()}") # Print first 25
                     print(f"DEBUG: _predict_from_logits - Sum of probs before choice: {np.sum(probs)}")
                     next_token = np.random.choice(len(probs), p=np.array(probs)) # Pass a new numpy array copy of probs
                     logger.debug(f"Telemetry: Random selected token ID: {next_token}, probability: {probs[next_token]:.4f}")

                elif method == "mirostat":
                    mu = kwargs.get('mu')
                    tau = kwargs.get('tau')
                    eta = kwargs.get('eta')
                    sorted_indices = np.argsort(probs)[::-1]
                    sorted_probs = probs[sorted_indices]
                    cumulative_probs = np.cumsum(sorted_probs)
                    k = np.argmax(cumulative_probs > mu) + 1
                    indices_to_keep = sorted_indices[:k]
                    mirostat_probs = np.zeros_like(probs)
                    mirostat_probs[indices_to_keep] = probs[indices_to_keep]
                    probs = mirostat_probs / np.sum(mirostat_probs)
                    next_token = np.random.choice(len(probs), p=probs)
                    
                    # Update mu
                    error = -np.log2(probs[next_token]) - tau
                    mu -= eta * error
                    return next_token, mu

                else:
                    raise ConfigurationError(f"Unknown sampling method: {method}")
        
        return next_token, probs

    def _apply_orchestration_policy(self, probs, method, temperature, top_k, top_p, past_tokens, repetition_penalty, bias_token_id, bias_strength):
        """
        Applies external orchestration policies to token probabilities.
        This implementation applies a bias to a specific token ID.
        """
        if bias_token_id is not None and bias_strength != 0.0:
            # Ensure bias_token_id is within the valid range of probabilities
            if 0 <= bias_token_id < len(probs):
                # Apply bias: increase probability for the biased token
                # A simple additive bias, ensure it doesn't exceed 1.0
                probs[bias_token_id] = min(probs[bias_token_id] + bias_strength, 1.0)
                # Re-normalize probabilities after biasing
                probs /= np.sum(probs)
                logger.debug(f"Orchestration: Applied bias to token {bias_token_id} with strength {bias_strength}. New prob: {probs[bias_token_id]:.4f}")
            else:
                logger.debug(f"Orchestration: bias_token_id {bias_token_id} is out of valid range [0, {len(probs)-1}]. Bias not applied.")
        return probs
