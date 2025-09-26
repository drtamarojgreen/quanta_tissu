import numpy as np
import logging

from ..layers import softmax
from ..model_error_handler import ConfigurationError, ModelProcessingError
from .config import GenerationConfig

logger = logging.getLogger(__name__)

class Generator:
    """
    Handles text generation and various sampling strategies.

    This class takes a trained model and uses it to generate new token sequences.
    """
    def __init__(self, model, tokenizer):
        """
        Initializes the Generator with a model instance and a tokenizer.

        Args:
            model: An instance of `quanta_tissu.tisslm.core.architecture.llm.Model`.
            tokenizer: A tokenizer instance.
        """
        self.model = model
        self.tokenizer = tokenizer

    def sample(self, prompt_tokens_list, n_new_tokens, config, **kwargs):
        """
        Generates sequences of tokens for a batch of prompts.
        Returns a tuple of (generated_tokens_list, attention_weights_history).
        """
        if not prompt_tokens_list or all(not p for p in prompt_tokens_list):
            return [], [] # Return empty generated tokens and attention history

        batch_size = len(prompt_tokens_list)
        max_len = max(len(p) for p in prompt_tokens_list)
        
        # Pad prompts to the same length
        padded_prompts = np.zeros((batch_size, max_len), dtype=int)
        for i, p in enumerate(prompt_tokens_list):
            padded_prompts[i, :len(p)] = p

        # Initialize generated sequences for each prompt in the batch
        generated_tokens_list = [[] for _ in range(batch_size)]
        current_tokens_list = [list(p) for p in prompt_tokens_list]
        attention_weights_history = []
        
        # Track which sequences are still being generated
        active_seqs = np.ones(batch_size, dtype=bool)

        # Initial forward pass with padded prompts
        logits, _, attention_weights_list = self.model.forward(padded_prompts, start_pos=0)
        attention_weights_history.append(attention_weights_list)

        if isinstance(config.eos_id, int):
            config.eos_id = [config.eos_id]

        for i in range(n_new_tokens):
            if not np.any(active_seqs):
                break

            last_logits = logits[active_seqs, -1, :]
            
            # Create a config for each active sequence
            configs = [config] * last_logits.shape[0]

            next_tokens = []
            for j, (logit, conf) in enumerate(zip(last_logits, configs)):
                past_tokens = current_tokens_list[np.where(active_seqs)[0][j]]
                next_token, _ = self._predict_from_logits(
                    logit,
                    conf,
                    past_tokens=past_tokens
                )
                next_tokens.append(next_token)
            
            next_tokens = np.array(next_tokens)
            
            # Update generated sequences and check for EOS
            active_indices = np.where(active_seqs)[0]
            for idx, next_token in zip(active_indices, next_tokens):
                if config.eos_id is not None and next_token in config.eos_id:
                    if not config.suppress_eos:
                        generated_tokens_list[idx].append(next_token)
                    active_seqs[idx] = False
                else:
                    generated_tokens_list[idx].append(next_token)
                    current_tokens_list[idx].append(next_token)

            if not np.any(active_seqs):
                break

            # Prepare for next forward pass
            next_token_array = next_tokens.reshape(-1, 1)
            
            # Get the start position for the next forward pass
            start_pos = max_len + i
            
            logits, _, attention_weights_list = self.model.forward(next_token_array, start_pos=start_pos)
            attention_weights_history.append(attention_weights_list)


        return generated_tokens_list, attention_weights_history

    def _predict_from_logits(self, logits, config, past_tokens=None):
        """
        Selects the next token from logits based on the chosen sampling method.
        """
        logger.debug(f"_predict_from_logits: method={config.method}, temp={config.temperature}, top_k={config.top_k}, top_p={config.top_p}, top_a={config.top_a}, logits shape={logits.shape}")
        if logits.ndim > 1:
            logits = np.squeeze(logits)

        # Apply n-gram repetition penalty
        if config.no_repeat_ngram_size > 0 and past_tokens and len(past_tokens) >= config.no_repeat_ngram_size:
            ngram_prefix = past_tokens[-(config.no_repeat_ngram_size - 1):]
            
            # Find all occurrences of the n-gram prefix
            for i in range(len(past_tokens) - (config.no_repeat_ngram_size - 1)):
                if past_tokens[i:i + config.no_repeat_ngram_size - 1] == ngram_prefix:
                    banned_token = past_tokens[i + config.no_repeat_ngram_size - 1]
                    logits[banned_token] = -np.inf

        # Apply repetition penalty
        if past_tokens and config.repetition_penalty != 1.0:
            for token_id in set(past_tokens):
                if token_id < len(logits):
                    if logits[token_id].item() > 0:
                        logits[token_id] /= config.repetition_penalty
                    else:
                        logits[token_id] *= config.repetition_penalty

        # Apply logit bias before softmax
        if config.logit_bias:
            for token_id, bias in config.logit_bias.items():
                if token_id < len(logits):
                    logits[token_id] += bias

        probs = softmax(logits, temperature=config.temperature)
        if probs.ndim > 1:
            probs = np.squeeze(probs)

        next_token = -1 # Initialize with an invalid token ID

        if config.method == "greedy":
            next_token = np.argmax(probs).item()

        elif config.method == "top_k":
            if config.top_k is None:
                raise ConfigurationError("top_k must be specified for top_k sampling")
            top_k_indices = np.argsort(probs)[-config.top_k:]
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            next_token = np.random.choice(len(probs), p=top_k_probs)

        elif config.method == "nucleus":
            if config.top_p is None:
                raise ConfigurationError("top_p must be specified for nucleus sampling")
            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)
            cutoff_idx = np.where(cumulative_probs >= config.top_p)[0][0]
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
            nucleus_probs = np.zeros_like(probs)
            nucleus_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]
            if np.sum(nucleus_probs) == 0:
                # Fallback for safety, though unlikely with correct logic
                next_token = tokens_to_keep_indices[0]
            else:
                nucleus_probs /= np.sum(nucleus_probs)
                next_token = np.random.choice(len(probs), p=nucleus_probs)
        
        elif config.method == "top_a":
            if config.top_a <= 0:
                raise ConfigurationError("top_a must be greater than 0 for top_a sampling")
            
            top_a_indices = np.where(probs > config.top_a)[0]
            if len(top_a_indices) == 0:
                # Fallback to greedy if no token is above the threshold
                next_token = np.argmax(probs).item()
            else:
                top_a_probs = np.zeros_like(probs)
                top_a_probs[top_a_indices] = probs[top_a_indices]
                top_a_probs /= np.sum(top_a_probs)
                next_token = np.random.choice(len(probs), p=top_a_probs)

        elif config.method == "random" or config.method == "sampling":
             next_token = np.random.choice(len(probs), p=np.array(probs))

        elif config.method == "adaptive_sentiment":
            # Sentiment bias is already applied before this point.
            # We can default to greedy or nucleus sampling after bias application.
            # For now, let's default to greedy after bias.
            next_token = np.argmax(probs).item()

        else:
            raise ConfigurationError(f"Unknown sampling method: {config.method}")
        
        return int(next_token), probs

    def beam_search(self, prompt_tokens, n_new_tokens, beam_width=3, eos_id=None):
        """
        Generates a sequence of tokens using beam search.
        """
        beams = [(list(prompt_tokens), 0.0)]
        
        for _ in range(n_new_tokens):
            new_beams = []
            for seq, score in beams:
                if eos_id is not None and seq[-1] == eos_id:
                    new_beams.append((seq, score))
                    continue

                prompt_array = np.array([seq])
                logits, _ = self.model.forward(prompt_array, start_pos=0)
                last_logit = logits[:, -1, :]
                
                probs = softmax(last_logit)
                
                top_k_indices = np.argsort(probs.squeeze())[-beam_width:]
                
                for token_id in top_k_indices:
                    new_seq = seq + [token_id]
                    new_score = score + np.log(probs.squeeze()[token_id])
                    new_beams.append((new_seq, new_score))
            
            beams = sorted(new_beams, key=lambda x: x[1], reverse=True)[:beam_width]

        best_seq, _ = beams[0]
        return best_seq[len(prompt_tokens):]

    def contrastive_search(self, prompt_tokens, n_new_tokens, beam_width=5, alpha=0.6, eos_id=None):
        """
        Generates a sequence of tokens using contrastive search.
        """
        current_tokens = list(prompt_tokens)
        generated_tokens = []

        for _ in range(n_new_tokens):
            prompt_array = np.array([current_tokens])
            logits, cache = self.model.forward(prompt_array, start_pos=0)
            last_logit = logits[:, -1, :]
            
            probs = softmax(last_logit).squeeze()
            
            # Get top-k candidates
            top_k_indices = np.argsort(probs)[-beam_width:]
            
            # Calculate context embeddings
            context_embeddings = self.model.embeddings.value[current_tokens]

            best_token = -1
            max_score = -np.inf

            for token_id in top_k_indices:
                # Model confidence
                model_confidence = probs[token_id]
                
                # Similarity penalty
                candidate_embedding = self.model.embeddings.value[token_id]
                
                # Cosine similarity
                sim = np.dot(context_embeddings, candidate_embedding) / (np.linalg.norm(context_embeddings, axis=1) * np.linalg.norm(candidate_embedding))
                
                # Degeneration penalty
                degeneration_penalty = np.max(sim)
                
                # Contrastive score
                score = (1 - alpha) * model_confidence - alpha * degeneration_penalty
                
                if score > max_score:
                    max_score = score
                    best_token = token_id
            
            if eos_id is not None and best_token == eos_id:
                break

            generated_tokens.append(best_token)
            current_tokens.append(best_token)

        return generated_tokens

    def mirostat_sampling(self, prompt_tokens, n_new_tokens, tau=5.0, eta=0.1, eos_id=None):
        """
        Generates a sequence of tokens using Mirostat sampling.
        """
        current_tokens = list(prompt_tokens)
        generated_tokens = []
        max_surprise = 2 * tau

        for _ in range(n_new_tokens):
            prompt_array = np.array([current_tokens])
            logits, _ = self.model.forward(prompt_array, start_pos=0)
            last_logit = logits[:, -1, :]
            
            probs = softmax(last_logit).squeeze()
            
            # Sort probabilities and calculate cumulative probabilities
            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            
            # Calculate surprise of each token
            surprises = -np.log2(sorted_probs)
            
            # Find the nucleus of tokens to sample from
            k = 0
            for k in range(len(surprises)):
                if surprises[k] > max_surprise:
                    break
            
            # If all tokens have surprise less than max_surprise, use all tokens
            if k == 0:
                k = len(surprises)

            nucleus_indices = sorted_indices[:k]
            nucleus_probs = probs[nucleus_indices]
            nucleus_probs /= np.sum(nucleus_probs)
            
            # Sample from the nucleus
            next_token = np.random.choice(nucleus_indices, p=nucleus_probs)
            
            # Calculate observed surprise
            observed_surprise = -np.log2(probs[next_token])
            
            # Update max_surprise
            max_surprise -= eta * (observed_surprise - tau)
            
            if eos_id is not None and next_token == eos_id:
                break

            generated_tokens.append(next_token)
            current_tokens.append(next_token)

        return generated_tokens

    def speculative_sampling(self, prompt_tokens, n_new_tokens, config, draft_model, **kwargs):
        """
        Generates a sequence of tokens using speculative sampling.
        This is a placeholder and currently defaults to greedy sampling.
        """
        logger.warning("Speculative sampling is not fully implemented and defaults to greedy sampling.")
        config.method = 'greedy'
        return self.sample([prompt_tokens], n_new_tokens, config, **kwargs)[0]

    def sample_with_context(self, query: str, documents: list, n_new_tokens: int, config: GenerationConfig, **kwargs):
        """
        Generates a sequence of tokens using a query and a list of documents as context.
        Returns the generated tokens, attention weights, and document token spans.
        """
        context_tokens = []
        doc_token_spans = []
        start = 0
        for doc in documents:
            doc_tokens = self.tokenizer.encode(doc)
            end = start + len(doc_tokens)
            doc_token_spans.append((start, end))
            context_tokens.extend(doc_tokens)
            start = end
            
        query_tokens = self.tokenizer.encode(f"\n\nQuery: {query}\n\nAnswer:")
        
        prompt_tokens = context_tokens + query_tokens
        
        generated_tokens, attention_weights = self.sample([prompt_tokens], n_new_tokens, config, **kwargs)
        return generated_tokens[0], attention_weights, doc_token_spans

    def calculate_source_attribution(self, attention_weights, doc_token_spans):
        """
        Calculates source attribution based on attention weights.
        """
        num_documents = len(doc_token_spans)
        attribution_scores = np.zeros(num_documents)

        # attention_weights is a list of lists of attention weights.
        # The outer list is for the generation steps, and the inner list is for the layers.
        # Each element in the inner list is a numpy array of shape (batch_size, num_heads, seq_len, seq_len).
        
        # We are only interested in the attention from the generated tokens to the source documents.
        # For simplicity, we will only use the attention weights from the last layer.
        
        for step_attention in attention_weights:
            last_layer_attention = step_attention[-1]
            # last_layer_attention shape: (batch_size, num_heads, seq_len, seq_len)
            
            # We only have one batch item here.
            last_layer_attention = last_layer_attention[0]
            # last_layer_attention shape: (num_heads, seq_len, seq_len)
            
            # We will average the attention scores over all heads.
            avg_attention = np.mean(last_layer_attention, axis=0)
            # avg_attention shape: (seq_len, seq_len)
            
            # The attention from the last generated token to the source documents.
            last_token_attention = avg_attention[-1, :]
            
            for i, (start, end) in enumerate(doc_token_spans):
                attribution_scores[i] += np.sum(last_token_attention[start:end])

        return attribution_scores
