import numpy as np
import logging
import os
import json
from collections import defaultdict, Counter
import re

from ..layers import softmax
from ..model_error_handler import ConfigurationError
from ..retrieval.strategy import BayesianSimilarityStrategy # Moved import

logger = logging.getLogger(__name__)

class AlgorithmicGenerator:
    """
    Handles text generation and various sampling strategies, including experimental ones.
    """
    def __init__(self, model, config):
        self.model = model
        self.config = config
        project_root = config.get("_project_root", ".")

        # State for file-based methods
        self.session_token_counts = Counter()
        self.wordlist_path = os.path.join(project_root, "data", "wordlist.txt")
        self.temp_list_path = os.path.join(project_root, "data", "temp_list.txt") # New temp file
        self._load_known_words()

    def _load_known_words(self):
        """Loads the initial set of known words from the wordlist file."""
        self.known_words = set()
        if os.path.exists(self.wordlist_path):
            with open(self.wordlist_path, 'r', encoding='utf-8') as f:
                for line in f:
                    self.known_words.add(line.strip().lower())

    def sample(self, prompt_tokens, n_new_tokens, method="greedy", **kwargs):
        """Dispatcher for various sampling methods."""
        if method == "dynamic_token_revision":
            return self.dynamic_token_revision_sampling(prompt_tokens, n_new_tokens, **kwargs)
        elif method == "bayesian_word_expansion":
            return self.bayesian_word_expansion_sampling(prompt_tokens, n_new_tokens, **kwargs)
        elif method == "adaptive_sentiment":
            return self.adaptive_sentiment_sampling(prompt_tokens, n_new_tokens, **kwargs)
        else:
            return self.iterative_sampling(prompt_tokens, n_new_tokens, method=method, **kwargs)

    def iterative_sampling(self, prompt_tokens, n_new_tokens, method, **kwargs):
        """Standard iterative sampling loop."""
        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]
        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)

        for _ in range(n_new_tokens):
            last_logit = logits[:, -1, :]
            next_token, _ = self._predict_from_logits(last_logit, method, **kwargs)
            kwargs['past_tokens'] = current_tokens # Update past_tokens for repetition penalty
            generated_tokens.append(next_token)
            current_tokens.append(next_token)
            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)
        return generated_tokens

    def dynamic_token_revision_sampling(self, prompt_tokens, n_new_tokens, **kwargs):
        """Generates text while tracking token usage and overwriting a temp file."""
        self.session_token_counts.clear()
        underlying_method = kwargs.get('underlying_method', 'nucleus')
        save_interval = kwargs.get('save_interval', 20)
        
        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]
        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)

        for i in range(n_new_tokens):
            last_logit = logits[:, -1, :]
            next_token, _ = self._predict_from_logits(last_logit, underlying_method, **kwargs)
            kwargs['past_tokens'] = current_tokens
            self.session_token_counts[next_token] += 1
            generated_tokens.append(next_token)
            current_tokens.append(next_token)
            if (i + 1) % save_interval == 0:
                self._save_session_tokens_to_temp_file()
            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)
        self._save_session_tokens_to_temp_file()
        return generated_tokens

    def _save_session_tokens_to_temp_file(self):
        """Helper to save the current session's token counts to temp_list.txt."""
        if not self.session_token_counts: return

        try:
            with open(self.temp_list_path, 'w', encoding='utf-8') as f:
                f.write("# Token counts from last generation session\n")
                for token_id, count in self.session_token_counts.most_common():
                    f.write(f"{token_id}: {count}\n")
            logger.info(f"Wrote session token counts to {self.temp_list_path}")
        except IOError as e:
            logger.error(f"Could not write to {self.temp_list_path}: {e}")
        # Clear session counts after saving
        self.session_token_counts.clear()

    def bayesian_word_expansion_sampling(self, prompt_tokens, n_new_tokens, **kwargs):
        """Generates text using Bayesian-influenced sampling and expands wordlist.txt."""
        tokenizer = kwargs.get('tokenizer')
        if not tokenizer: raise ConfigurationError("bayesian_word_expansion requires a 'tokenizer' in kwargs.")
        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]
        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)
        for _ in range(n_new_tokens):
            last_logit = logits[:, -1, :]
            next_token, probs = self._predict_from_logits(
                last_logit, 
                'bayesian_influenced', 
                **kwargs
            )
            kwargs['past_tokens'] = current_tokens
            generated_tokens.append(next_token)
            current_tokens.append(next_token)
            logger.debug(f"BWE: Generated token: {next_token}, Prob: {probs[next_token]:.4f}, Max Prob: {np.max(probs):.4f}")
            current_text = tokenizer.detokenize(np.array(current_tokens))
            last_word = current_text.split(' ')[-1].lower()
            if last_word and len(last_word) > 2 and last_word not in self.known_words:
                if probs[next_token] > kwargs.get('expansion_threshold', 0.7):
                    self.known_words.add(last_word)
                    try:
                        with open(self.wordlist_path, 'a', encoding='utf-8') as f:
                            f.write(f"{last_word}\n")
                        logger.info(f"Expanded wordlist with new word: {last_word}")
                    except IOError as e:
                        logger.error(f"Could not write to {self.wordlist_path}: {e}")
            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)
        return generated_tokens

    def adaptive_sentiment_sampling(self, prompt_tokens, n_new_tokens, **kwargs):
        """
        Generates text by adaptively adjusting bias based on the sentiment of generated chunks.
        """
        tokenizer = kwargs.get('tokenizer')
        sentiment_analyzer = kwargs.get('sentiment_analyzer')
        target_sentiment = kwargs.get('target_sentiment', 'neutral')
        target_strength = kwargs.get('target_strength', 0.5)
        evaluation_interval = kwargs.get('evaluation_interval', 10) # Re-evaluate sentiment every N tokens
        underlying_method = kwargs.get('underlying_method', 'nucleus')

        if not tokenizer or not sentiment_analyzer:
            raise ConfigurationError("adaptive_sentiment_sampling requires 'tokenizer' and 'sentiment_analyzer' in kwargs.")

        generated_tokens = []
        current_tokens = [int(t) for t in prompt_tokens]
        prompt_array = np.array([current_tokens])
        logits, _ = self.model.forward(prompt_array, start_pos=0)

        current_sentiment_bias = sentiment_analyzer.get_sentiment_bias(target_sentiment, target_strength)

        for i in range(n_new_tokens):
            last_logit = logits[:, -1, :]
            
            # Apply current sentiment bias to logits before prediction
            biased_logits = np.copy(last_logit) # Work on a copy
            for token_id, bias_val in current_sentiment_bias.items():
                if token_id < len(biased_logits):
                    biased_logits[token_id] += bias_val

            next_token, _ = self._predict_from_logits(biased_logits, underlying_method, **kwargs)
            kwargs['past_tokens'] = current_tokens
            generated_tokens.append(next_token)
            current_tokens.append(next_token)

            # Re-evaluate sentiment and adjust bias periodically
            if (i + 1) % evaluation_interval == 0:
                chunk_tokens = generated_tokens[-evaluation_interval:]
                chunk_text = tokenizer.detokenize(np.array(chunk_tokens))
                
                # Calculate average sentiment of the chunk
                chunk_sentiment_sum = 0.0
                scored_words_in_chunk = 0
                words_in_chunk = re.findall(r'\b\w+\b', chunk_text.lower())
                for word in words_in_chunk:
                    score = sentiment_analyzer.sentiment_scores.get(word, 0.0)
                    if score != 0.0:
                        chunk_sentiment_sum += score
                        scored_words_in_chunk += 1
                
                avg_chunk_sentiment = chunk_sentiment_sum / scored_words_in_chunk if scored_words_in_chunk > 0 else 0.0

                # Adjust bias for the next chunk based on deviation from target
                target_score = 0.0
                if target_sentiment == 'positive': target_score = 1.0
                elif target_sentiment == 'negative': target_score = -1.0

                deviation = target_score - avg_chunk_sentiment
                
                # Simple adaptive logic: increase strength if deviating, decrease if on target
                # This is a basic heuristic and can be refined.
                adjustment_factor = 0.1 # How much to adjust strength by
                if abs(deviation) > 0.1: # If significant deviation
                    if deviation > 0: # Need more of target sentiment
                        target_strength += adjustment_factor
                    else: # Too much of target sentiment, or wrong sentiment
                        target_strength -= adjustment_factor
                
                target_strength = max(0.0, min(1.0, target_strength)) # Clamp strength
                current_sentiment_bias = sentiment_analyzer.get_sentiment_bias(target_sentiment, target_strength)
                logger.debug(f"Adapted sentiment bias. New strength: {target_strength:.2f}")

            next_token_array = np.array([[next_token]])
            logits, _ = self.model.forward(next_token_array, start_pos=len(current_tokens) - 1)
        return generated_tokens

    def _predict_from_logits(self, logits, method="greedy", **kwargs):
        if logits.ndim > 1:
            logits = np.squeeze(logits)
        if kwargs.get('past_tokens') and kwargs.get('repetition_penalty', 1.0) != 1.0:
            for token_id in set(kwargs['past_tokens']):
                if token_id < len(logits):
                    if logits[token_id].item() > 0:
                        logits[token_id] /= kwargs['repetition_penalty']
                    else:
                        logits[token_id] *= kwargs['repetition_penalty']
        if method == "bayesian_influenced":
            query_embedding = kwargs.get('query_embedding')
            hessian_matrix = kwargs.get('hessian_matrix')
            if query_embedding is None or hessian_matrix is None: raise ValueError("bayesian_influenced requires 'query_embedding' and 'hessian_matrix'")
            all_vocab_embeddings = self.model.embeddings.value
            similarity_scores = BayesianSimilarityStrategy().calculate_similarity(query_embedding, all_vocab_embeddings, hessian_matrix=hessian_matrix)
            logits += similarity_scores * kwargs.get('bayesian_influence_scale', 1.0)
        probs = softmax(logits, temperature=kwargs.get('temperature', 1.0))
        if probs.ndim > 1:
            probs = np.squeeze(probs)
        if method in ["greedy", "bayesian_influenced"]:
            next_token = int(np.argmax(probs))
        elif method == "nucleus":
            top_p = kwargs.get('top_p', 0.9)
            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)
            cutoff_idx = np.where(cumulative_probs >= top_p)[0][0]
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
            nucleus_probs = np.zeros_like(probs)
            nucleus_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]
            nucleus_probs /= np.sum(nucleus_probs)
            next_token = np.random.choice(len(probs), p=nucleus_probs)
        else:
            raise ConfigurationError(f"Unsupported sampling method for iterative generation: {method}")
        return next_token, probs
