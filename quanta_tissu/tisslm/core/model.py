import numpy as np
import os
import logging
import traceback
import functools
from .layers import MultiHeadAttention, FeedForward, LayerNorm, softmax
from .knowledge_base import KnowledgeBase
from .tokenizer import tokenize
from .parameter import Parameter
from .model_error_handler import (
    TissModelError,
    ModelInitializationError,
    InferenceError,
    InputValidationError,
    ConfigurationError,
    ModelProcessingError,
    TissAssertionError,
    handle_model_errors
)
from .system_error_handler import SystemError, handle_errors, ConfigurationError, DatabaseConnectionError, FileIOError
from .system_error_handler import TissSystemError, DatabaseConnectionError, handle_errors

logger = logging.getLogger(__name__)

class TransformerBlock:
    def __init__(self, d_model, num_heads, d_ff, name=""):
        self.mha = MultiHeadAttention(d_model, num_heads, name=f"{name}.mha")
        self.ffn = FeedForward(d_model, d_ff, name=f"{name}.ffn")
        self.ln1 = LayerNorm(d_model, name=f"{name}.ln1")
        self.ln2 = LayerNorm(d_model, name=f"{name}.ln2")
        self.cache = {}

    @handle_errors
    def __call__(self, x, mask=None, kv_cache=None):
        logger.debug(f"TransformerBlock {self.mha.name} input x shape: {x.shape}")
        # Store input for backward pass
        self.cache['x'] = x

        # Attention sub-layer
        attn_out = self.mha(x, mask=mask, kv_cache=kv_cache)
        logger.debug(f"TransformerBlock {self.mha.name} attn_out shape: {attn_out.shape}")
        self.cache['attn_out'] = attn_out

        # Add & Norm
        x_plus_attn = x + attn_out
        self.cache['x_plus_attn'] = x_plus_attn
        x_norm1 = self.ln1(x_plus_attn)
        logger.debug(f"TransformerBlock {self.mha.name} x_norm1 shape: {x_norm1.shape}")
        self.cache['x_norm1'] = x_norm1

        # FFN sub-layer
        ffn_out = self.ffn(x_norm1)
        logger.debug(f"TransformerBlock {self.mha.name} ffn_out shape: {ffn_out.shape}")
        self.cache['ffn_out'] = ffn_out

        # Add & Norm
        x_plus_ffn = x_norm1 + ffn_out
        self.cache['x_plus_ffn'] = x_plus_ffn
        x_norm2 = self.ln2(x_plus_ffn)
        logger.debug(f"TransformerBlock {self.mha.name} output x_norm2 shape: {x_norm2.shape}")

        return x_norm2

    @handle_errors
    def backward(self, d_out):
        logger.debug(f"TransformerBlock {self.mha.name} backward d_out shape: {d_out.shape}")
        # Backprop through second Add & Norm
        dx_plus_ffn = self.ln2.backward(d_out)
        logger.debug(f"TransformerBlock {self.mha.name} backward dx_plus_ffn shape: {dx_plus_ffn.shape}")
        dx_norm1 = dx_plus_ffn # from residual
        d_ffn_out = dx_plus_ffn # from main path

        # Backprop through FFN
        dx_norm1 += self.ffn.backward(d_ffn_out)
        logger.debug(f"TransformerBlock {self.mha.name} backward dx_norm1 after FFN: {dx_norm1.shape}")

        # Backprop through first Add & Norm
        dx_plus_attn = self.ln1.backward(dx_norm1)
        dx = dx_plus_attn # from residual
        d_attn_out = dx_plus_attn # from main path

        # Backprop through MHA
        dx += self.mha.backward(d_attn_out)
        logger.debug(f"TransformerBlock {self.mha.name} backward output dx shape: {dx.shape}")

        return dx

    def parameters(self):
        return self.mha.parameters() + self.ffn.parameters() + self.ln1.parameters() + self.ln2.parameters()

class PositionalEncoding:
    def __init__(self, d_model, max_len=5000):
        pe = np.zeros((max_len, d_model))
        position = np.arange(0, max_len)[:, np.newaxis]
        div_term = np.exp(np.arange(0, d_model, 2) * (-np.log(10000.0) / d_model))
        pe[:, 0::2] = np.sin(position * div_term)
        pe[:, 1::2] = np.cos(position * div_term)
        self.pe = pe

    @handle_errors
    def __call__(self, x, start_pos=0):
        logger.debug(f"PositionalEncoding input x shape: {x.shape}, start_pos: {start_pos}")
        # Assumes x is (batch_size, seq_len, d_model)
        seq_len = x.shape[1]
        max_len = self.pe.shape[0]
        if start_pos + seq_len > max_len:
            raise InputValidationError(f"Cannot apply positional encoding: sequence of length {seq_len} at start position {start_pos} exceeds max length of {max_len}.")
        
        positions = np.arange(start_pos, start_pos + seq_len)
        x_out = x + self.pe[np.newaxis, positions, :]
        logger.debug(f"PositionalEncoding output x shape: {x_out.shape}")
        return x_out

class QuantaTissu:
    @handle_errors
    def __init__(self, config, db_host='127.0.0.1', db_port=8080, use_db=False):
        self.config = config
        d_model = config["n_embd"]
        vocab_size = config["vocab_size"]
        n_head = config["n_head"]
        d_ff = config["d_ff"]
        n_layer = config["n_layer"]

        logger.debug(f"QuantaTissu __init__: d_model={d_model}, vocab_size={vocab_size}, n_head={n_head}, d_ff={d_ff}, n_layer={n_layer}")

        self.d_model = d_model
        self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
        logger.debug(f"QuantaTissu embeddings shape: {self.embeddings.value.shape}")
        self.pos_encoding = PositionalEncoding(d_model)
        self.transformer_blocks = [TransformerBlock(d_model, n_head, d_ff, name=f"transformer_blocks.{i}") for i in range(n_layer)]
        self.output_proj = Parameter(np.random.randn(d_model, vocab_size) / np.sqrt(d_model), name="output_proj")

        if use_db:
            try:
                logger.info(f"Initializing KnowledgeBase with TissDB connection to {db_host}:{db_port}")
                self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
            except DatabaseConnectionError as e:
                raise TissSystemError(f"Failed to connect to database: {e}") from e
        else:
            self.knowledge_base = None
        self.cache = {}

    def _create_causal_mask(self, seq_len):
        mask = np.triu(np.ones((seq_len, seq_len)), k=1) * -1e9
        return mask

    @handle_errors
    def forward(self, token_ids, kv_cache=None, start_pos=0):
        logger.debug(f"Model forward pass: token_ids shape {token_ids.shape}, start_pos {start_pos}")
        batch_size, seq_len = token_ids.shape

        mask = None
        if start_pos == 0: # Only create causal mask for the initial sequence
            mask = self._create_causal_mask(seq_len)
            if mask is not None:
                mask = mask[np.newaxis, np.newaxis, :, :]
            logger.debug(f"Causal mask created with shape: {mask.shape}")

        self.cache['token_ids'] = token_ids

        x = self.embeddings.value[token_ids]
        logger.debug(f"After embeddings: x shape {x.shape}")

        x = self.pos_encoding(x, start_pos=start_pos)
        logger.debug(f"After positional encoding: x shape {x.shape}")
        self.cache['x_pos_encoded'] = x

        for i, block in enumerate(self.transformer_blocks):
            layer_cache = kv_cache[i] if kv_cache is not None else None
            logger.debug(f"Processing TransformerBlock {i}. KV cache for block: {'present' if layer_cache else 'absent'}")
            x = block(x, mask=mask, kv_cache=layer_cache)
            logger.debug(f"After TransformerBlock {i}: x shape {x.shape}")
            self.cache[f'block_{i}_out'] = x

        logits = x @ self.output_proj.value
        logger.debug(f"Final logits shape: {logits.shape}")
        self.cache['final_x'] = x
        return logits

    @handle_errors
    def backward(self, d_logits):
        logger.debug(f"Model backward pass: d_logits shape {d_logits.shape}")
        # d_logits is (batch, seq, vocab_size)
        # final_x is (batch, seq, d_model)
        batch_size, seq_len, d_model = self.cache['final_x'].shape

        # Reshape for matmul
        final_x_reshaped = self.cache['final_x'].reshape(batch_size * seq_len, d_model)
        d_logits_reshaped = d_logits.reshape(batch_size * seq_len, -1) # vocab_size

        # Backprop through output projection
        self.output_proj.grad += final_x_reshaped.T @ d_logits_reshaped
        dx = d_logits @ self.output_proj.value.T # This one is ok, it broadcasts correctly
        logger.debug(f"Model backward: dx after output_proj backward: {dx.shape}")

        # Backprop through transformer blocks
        for i in reversed(range(len(self.transformer_blocks))):
            dx = self.transformer_blocks[i].backward(dx)
            logger.debug(f"Model backward: dx after TransformerBlock {i} backward: {dx.shape}")
            
        # Backprop through positional encoding (just passes gradient)
        dx_pos = dx

        # Backprop through embeddings
        # This is a bit tricky. We need to add gradients for each token.
        # Create a zero gradient array for the embeddings
        d_embeddings = np.zeros_like(self.embeddings.value)
        logger.debug(f"Model backward: d_embeddings initial shape: {d_embeddings.shape}")

        # Get the original token_ids and embedded input
        token_ids = self.cache['token_ids']
        logger.debug(f"Model backward: token_ids shape for embedding gradient: {token_ids.shape}")

        # Accumulate gradients
        # Using np.add.at for efficient indexed addition
        np.add.at(d_embeddings, token_ids, dx_pos)
        logger.debug(f"Model backward: d_embeddings after accumulation: {d_embeddings.shape}")

        self.embeddings.grad += d_embeddings

        # --- Receptor Field and Hessian Matrix Calculation ---
        # This is a placeholder for the actual calculation.
        
        # 1. Receptor Field (using embedding gradients as a proxy)
        receptor_field = {
            'type': 'token_gradients',
            'gradients': self.embeddings.grad.tolist() # Convert to list for JSON serialization
        }

        # 2. Hessian Matrix (placeholder)
        # In a real scenario, this would involve complex second-order derivative calculations.
        # Here, we'll just create a dummy Hessian matrix of eigenvalues.
        hessian_matrix = {
            'type': 'eigenvalues',
            'eigenvalues': np.random.rand(self.d_model).tolist() # Dummy eigenvalues
        }

        return {
            'receptor_field': receptor_field,
            'hessian_matrix': hessian_matrix
        }

    def parameters(self):
        params = [self.embeddings, self.output_proj]
        for block in self.transformer_blocks:
            params.extend(block.parameters())
        return params

    @handle_errors
    def load_weights(self, path):
        if not os.path.exists(path):
            logger.warning(f"Model weights file not found at {path}. Using random initialization.")
            return

        try:
            data = np.load(path)
            keys = list(data.keys())
            logger.info(f"Loading weights from {path}. Found keys: {keys}")

            # A legacy checkpoint is one that contains 'param_' keys.
            # It might also contain optimizer states, so we check with any().
            is_legacy = any(k.startswith('param_') for k in keys)

            if keys and is_legacy:
                logger.info("Detected legacy checkpoint format. Loading by parameter order.")
                # Filter for model parameter keys only
                param_keys = [k for k in keys if k.startswith('param_')]
                # Sort keys numerically: param_0, param_1, ...
                sorted_keys = sorted(param_keys, key=lambda k: int(k.split('_')[1]))
                model_params = self.parameters()

                # Filter out optimizer state from the loaded keys before comparing lengths
                num_model_params_in_ckpt = len(sorted_keys)

                if num_model_params_in_ckpt != len(model_params):
                    logger.warning(f"Mismatch in parameter count. Checkpoint has {num_model_params_in_ckpt} model params, but model requires {len(model_params)}.")

                for i, key in enumerate(sorted_keys):
                    if i < len(model_params):
                        param = model_params[i]
                        if param.value.shape == data[key].shape:
                            param.value[:] = data[key]
                        else:
                            logger.warning(f"Shape mismatch for {param.name} (from {key}). Expected {param.value.shape}, got {data[key].shape}. Skipping.")
                    else:
                        break  # No more model params to load into
            else:
                # Load by hierarchical name
                logger.info("Loading by hierarchical parameter name.")
                for param in self.parameters():
                    if param.name in data:
                        if param.value.shape == data[param.name].shape:
                            param.value = data[param.name]
                        else:
                            logger.warning(f"Shape mismatch for {param.name}. Expected {param.value.shape}, got {data[param.name].shape}. Skipping.")
                    else:
                        logger.warning(f"Parameter {param.name} not found in weights file. Using random initialization.")

            logger.info(f"Successfully loaded model weights from {path}")
        except (IOError, FileNotFoundError) as e:
            # Re-raise file-related errors as a system error
            raise TissSystemError(f"Failed to read model weights file from {path}: {e}") from e
        except Exception as e:
            # Wrap other potential errors (e.g., parsing, key errors) in a model processing error
            logger.error(f"Error processing model weights from {path}: {e}", exc_info=True)
            raise ModelProcessingError(f"Failed to process model weights from {path}: {e}") from e

    @handle_errors
    def _predict_from_logits(self, logits, method="greedy", temperature=1.0, top_k=None, top_p=None, past_tokens=None, repetition_penalty=1.0):
        logger.debug(f"_predict_from_logits: method={method}, temp={temperature}, top_k={top_k}, top_p={top_p}, logits shape={logits.shape}")
        # Ensure logits are a 1D array for consistent processing
        if logits.ndim > 1:
            logits = np.squeeze(logits)
            logger.debug(f"_predict_from_logits: logits after squeeze: {logits.shape}")

        # Apply repetition penalty
        if past_tokens is not None and repetition_penalty != 1.0:
            logger.debug(f"Applying repetition penalty for past_tokens: {len(past_tokens)} tokens, penalty={repetition_penalty}")
            for token_id in set(past_tokens): # Use set to avoid penalizing the same token multiple times
                if logits[token_id] > 0:
                    logits[token_id] /= repetition_penalty
                else:
                    # For negative logits, multiplying by a penalty > 1 makes them more negative
                    logits[token_id] *= repetition_penalty

        if method == "greedy":
            next_token = np.argmax(logits)
            logger.debug(f"Greedy sampling: next_token={next_token}")
            return int(next_token)

        probs = softmax(logits, temperature=temperature)
        logger.debug(f"Probabilities after softmax (first 5): {probs[:5]}")
        logger.debug(f"Probabilities after softmax: {probs.shape}")


        # Ensure probs is also 1D, in case softmax re-adds a dimension
        if probs.ndim > 1:
            probs = np.squeeze(probs)
            logger.debug(f"Probabilities after squeeze: {probs.shape}")
        
        if method == "top_k":
            if top_k is None:
                raise ConfigurationError("top_k must be specified for top_k sampling")
                
            top_k_indices = np.argsort(probs)[-top_k:]
            logger.debug(f"Top-K sampling: top_k_indices={top_k_indices.tolist()}")
            logger.debug(f"Top-K sampling: top_k_indices shape: {top_k_indices.shape}, top_k_probs shape: {np.zeros_like(probs).shape}") # np.zeros_like(probs) is used to get the shape of top_k_probs before assignment
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            next_token = np.random.choice(len(probs), p=top_k_probs)
            logger.debug(f"Top-K sampling: next_token={next_token}")
        elif method == "nucleus":
            if top_p is None:
                raise ConfigurationError("top_p must be specified for nucleus sampling")
            
            # Sort probabilities in descending order
            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            
            # Calculate cumulative probabilities
            cumulative_probs = np.cumsum(sorted_probs)
            logger.debug(f"Nucleus sampling: sorted_indices shape: {sorted_indices.shape}, sorted_probs shape: {sorted_probs.shape}, cumulative_probs shape: {cumulative_probs.shape}")
            
            # Find the index where the cumulative probability exceeds top_p
            # Add a small epsilon to top_p to handle floating point inaccuracies
            cutoff_idx_where = np.where(cumulative_probs >= top_p - 1e-8)[0]
            if len(cutoff_idx_where) == 0:
                # This can happen if top_p is > 1.0 or due to float inaccuracies
                # In this case, we take all tokens
                cutoff_idx = len(sorted_indices) - 1
                logger.warning(f"Nucleus sampling: top_p {top_p} is too high, no cutoff found. Taking all tokens.")
            else:
                cutoff_idx = cutoff_idx_where[0]
            
            # Create a mask for tokens to keep (up to and including cutoff_idx)
            # All tokens after cutoff_idx should have their probabilities set to 0
            tokens_to_keep_indices = sorted_indices[:cutoff_idx + 1]
            logger.debug(f"Nucleus sampling: cutoff_idx={cutoff_idx}, tokens_to_keep_indices={tokens_to_keep_indices.tolist()}")
            
            nucleus_probs = np.zeros_like(probs)
            nucleus_probs[tokens_to_keep_indices] = probs[tokens_to_keep_indices]
            
            # Normalize the remaining probabilities
            if np.sum(nucleus_probs) > 0: # Avoid division by zero if all probs are zeroed out
                nucleus_probs /= np.sum(nucleus_probs)
                logger.debug(f"Nucleus sampling: nucleus_probs shape: {nucleus_probs.shape}")
            else:
                # Fallback: if all probabilities are zeroed, revert to uniform sampling or greedy
                # For now, we'll raise an error or fallback to greedy if this happens
                # A more robust solution might be to ensure at least one token remains.
                raise ModelProcessingError("Nucleus sampling resulted in all probabilities being zeroed out.")

            next_token = np.random.choice(len(probs), p=nucleus_probs)
            logger.debug(f"Nucleus sampling: next_token={next_token}")
        elif method == "random" or method == "sampling":
             next_token = np.random.choice(len(probs), p=probs.flatten())
             logger.debug(f"Random sampling: next_token={next_token}")
        else:
            raise ConfigurationError(f"Unknown sampling method: {method}")

        return int(next_token)

    @handle_errors
    def generate(self, prompt_tokens, n_new_tokens, method="greedy", temperature=1.0, top_k=None, top_p=None, use_cache=True, repetition_penalty=1.0):
        logger.debug(f"Starting generation: prompt_tokens_len={len(prompt_tokens)}, n_new_tokens={n_new_tokens}, method={method}, temp={temperature}, top_k={top_k}, top_p={top_p}, use_cache={use_cache}")
        if not use_cache:
            # Generate without cache by calling predict in a loop
            generated_ids = []
            current_tokens = list(prompt_tokens)
            for _ in range(n_new_tokens):
                token_ids_np = np.array(current_tokens)
                next_token_id = self.predict(token_ids_np, method=method, temperature=temperature, top_k=top_k, top_p=top_p, repetition_penalty=repetition_penalty)
                generated_ids.append(next_token_id)
                current_tokens.append(next_token_id)
            logger.debug(f"Generated IDs (without cache): {generated_ids}")
            return generated_ids

        # 1. Initialize cache for each layer
        kv_cache = [{} for _ in self.transformer_blocks]
        logger.debug("KV cache initialized.")

        # 2. Process prompt tokens and populate the cache
        prompt_token_ids = np.array(prompt_tokens)[np.newaxis, :]
        logger.debug(f"Generate: prompt_token_ids shape: {prompt_token_ids.shape}")
        logger.debug(f"Processing prompt tokens: {prompt_token_ids.tolist()}")
        prompt_logits = self.forward(prompt_token_ids, kv_cache=kv_cache, start_pos=0)
        logger.debug(f"Generate: prompt_logits shape: {prompt_logits.shape}")
        
        # 3. Predict the first token from the prompt's output
        next_token_id = self._predict_from_logits(prompt_logits[:, -1, :], method, temperature, top_k, top_p, past_tokens=prompt_tokens, repetition_penalty=repetition_penalty)
        logger.debug(f"First generated token ID (from prompt): {next_token_id}")
        
        generated_ids = [next_token_id]
        
        # 4. Sequentially generate the rest of the tokens
        for i in range(n_new_tokens - 1):
            current_token_id = np.array([[next_token_id]])
            start_pos = len(prompt_tokens) + i
            logger.debug(f"Generate loop: current_token_id shape: {current_token_id.shape}, start_pos: {start_pos}")
            logger.debug(f"Generating token {i+1}/{n_new_tokens-1}: current_token_id={current_token_id.tolist()}, start_pos={start_pos}")
            
            logits = self.forward(current_token_id, kv_cache=kv_cache, start_pos=start_pos)
            logger.debug(f"Generate loop: logits shape from forward: {logits.shape}")
            # Pass all previously generated tokens (prompt + generated so far)
            all_past_tokens = list(prompt_tokens) + generated_ids
            logger.debug(f"Generate loop: all_past_tokens length: {len(all_past_tokens)}")
            next_token_id = self._predict_from_logits(logits[:, -1, :], method, temperature, top_k, top_p, past_tokens=all_past_tokens, repetition_penalty=repetition_penalty)
            generated_ids.append(next_token_id)
            logger.debug(f"Generated token ID: {next_token_id}, current generated_ids: {generated_ids}")
            
        logger.debug(f"Final generated IDs: {generated_ids}")
        return generated_ids

    @handle_errors
    def predict(self, token_ids, method="greedy", temperature=1.0, top_k=None, top_p=None):
        """
        DEPRECATED: This method is inefficient for generation as it re-processes the entire sequence for each token.
        Use generate() for efficient autoregressive generation.
        This method predicts the single next token from a given sequence.
        """
        if token_ids.ndim == 1:
            token_ids = token_ids[np.newaxis, :]

        logits = self.forward(token_ids)
        last_logit = logits[0, -1, :]
        return self._predict_from_logits(last_logit, method, temperature, top_k, top_p)

    @handle_errors
    def generate_with_kb(self, prompt, n_new_tokens, generation_method="greedy", k=1, **kwargs):
        context_docs = self.knowledge_base.retrieve(prompt, k=k)
        if context_docs:
            context = " ".join(context_docs)
            augmented_prompt = f"context: {context} question: {prompt}"
            logger.info(f"Augmented prompt with context: '{augmented_prompt}'")
        else:
            augmented_prompt = prompt
        token_ids = tokenize(augmented_prompt)
        if len(token_ids) == 0:
            logger.warning("Prompt resulted in empty token sequence. Cannot generate.")
            raise InputValidationError("Prompt resulted in empty token sequence after tokenization. Cannot generate.")
        
        generated_token_ids = self.generate(token_ids, n_new_tokens, method=generation_method, **kwargs)
