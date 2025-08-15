import numpy as np
from .layers import MultiHeadAttention, FeedForward, LayerNorm, softmax
from .knowledge_base import KnowledgeBase
from .tokenizer import tokenize
from .parameter import Parameter

class TransformerBlock:
    def __init__(self, d_model, num_heads, d_ff):
        self.mha = MultiHeadAttention(d_model, num_heads)
        self.ffn = FeedForward(d_model, d_ff)
        self.ln1 = LayerNorm(d_model)
        self.ln2 = LayerNorm(d_model)
        self.cache = {}

    def __call__(self, x, mask=None):
        # Store input for backward pass
        self.cache['x'] = x

        # Attention sub-layer
        attn_out = self.mha(x, mask=mask)
        self.cache['attn_out'] = attn_out

        # Add & Norm
        x_plus_attn = x + attn_out
        self.cache['x_plus_attn'] = x_plus_attn
        x_norm1 = self.ln1(x_plus_attn)
        self.cache['x_norm1'] = x_norm1

        # FFN sub-layer
        ffn_out = self.ffn(x_norm1)
        self.cache['ffn_out'] = ffn_out

        # Add & Norm
        x_plus_ffn = x_norm1 + ffn_out
        self.cache['x_plus_ffn'] = x_plus_ffn
        x_norm2 = self.ln2(x_plus_ffn)

        return x_norm2

    def backward(self, d_out):
        # Backprop through second Add & Norm
        dx_plus_ffn = self.ln2.backward(d_out)
        dx_norm1 = dx_plus_ffn # from residual
        d_ffn_out = dx_plus_ffn # from main path

        # Backprop through FFN
        dx_norm1 += self.ffn.backward(d_ffn_out)

        # Backprop through first Add & Norm
        dx_plus_attn = self.ln1.backward(dx_norm1)
        dx = dx_plus_attn # from residual
        d_attn_out = dx_plus_attn # from main path

        # Backprop through MHA
        dx += self.mha.backward(d_attn_out)

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

    def __call__(self, x):
        # Assumes x is (batch_size, seq_len, d_model)
        seq_len = x.shape[1]
        return x + self.pe[np.newaxis, :seq_len, :]

class QuantaTissu:
    def __init__(self, config):
        self.config = config
        d_model = config["d_model"]
        vocab_size = config["vocab_size"]
        num_heads = config["num_heads"]
        d_ff = config["d_ff"]
        n_layers = config["n_layers"]

        self.d_model = d_model
        self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model))
        self.pos_encoding = PositionalEncoding(d_model)
        self.transformer_blocks = [TransformerBlock(d_model, num_heads, d_ff) for _ in range(n_layers)]
        self.output_proj = Parameter(np.random.randn(d_model, vocab_size) / np.sqrt(d_model))

        # KnowledgeBase is not part of the trainable model graph
        self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize)
        self.cache = {}

    def _create_causal_mask(self, seq_len):
        mask = np.triu(np.ones((seq_len, seq_len)), k=1) * -1e9
        return mask

    def forward(self, token_ids):
        # token_ids: (batch_size, seq_len)
        batch_size, seq_len = token_ids.shape
        mask = self._create_causal_mask(seq_len)
        # Add batch and head dimensions for broadcasting
        mask = mask[np.newaxis, np.newaxis, :, :]
        
        self.cache['token_ids'] = token_ids
        
        x = self.embeddings.value[token_ids]
        self.cache['x_embedded'] = x

        x = self.pos_encoding(x)
        self.cache['x_pos_encoded'] = x

        for i, block in enumerate(self.transformer_blocks):
            x = block(x, mask=mask)
            self.cache[f'block_{i}_out'] = x
            
        logits = x @ self.output_proj.value
        self.cache['final_x'] = x
        return logits

    def backward(self, d_logits):
        # d_logits is (batch, seq, vocab_size)
        # final_x is (batch, seq, d_model)
        batch_size, seq_len, d_model = self.cache['final_x'].shape

        # Reshape for matmul
        final_x_reshaped = self.cache['final_x'].reshape(batch_size * seq_len, d_model)
        d_logits_reshaped = d_logits.reshape(batch_size * seq_len, -1) # vocab_size

        # Backprop through output projection
        self.output_proj.grad += final_x_reshaped.T @ d_logits_reshaped
        dx = d_logits @ self.output_proj.value.T # This one is ok, it broadcasts correctly

        # Backprop through transformer blocks
        for i in reversed(range(len(self.transformer_blocks))):
            dx = self.transformer_blocks[i].backward(dx)
            
        # Backprop through positional encoding (just passes gradient)
        dx_pos = dx

        # Backprop through embeddings
        # This is a bit tricky. We need to add gradients for each token.
        # Create a zero gradient array for the embeddings
        d_embeddings = np.zeros_like(self.embeddings.value)

        # Get the original token_ids and embedded input
        token_ids = self.cache['token_ids']

        # Accumulate gradients
        # Using np.add.at for efficient indexed addition
        np.add.at(d_embeddings, token_ids, dx_pos)

        self.embeddings.grad += d_embeddings

        # No gradient returned, as embeddings are the input layer
        return

    def parameters(self):
        params = [self.embeddings, self.output_proj]
        for block in self.transformer_blocks:
            params.extend(block.parameters())
        return params

    def predict(self, token_ids, method="greedy", temperature=1.0, top_k=None, top_p=None):
        # Predict needs to handle a single sequence, but forward expects a batch.
        # So, we add a batch dimension.
        if token_ids.ndim == 1:
            token_ids = token_ids[np.newaxis, :] # (1, seq_len)

        logits = self.forward(token_ids) # (1, seq_len, vocab_size)
        last_logit = logits[0, -1, :] # (vocab_size,)

        if method == "greedy":
            next_token = np.argmax(last_logit)
            return int(next_token)

        probs = softmax(last_logit, temperature=temperature)
        # ... (rest of the prediction logic is the same)
        if method == "top_k":
            if top_k is None:
                raise ValueError("top_k must be specified for top_k sampling")

            top_k_indices = np.argsort(probs)[-top_k:]
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            next_token = np.random.choice(len(probs), p=top_k_probs)

        elif method == "nucleus":
            if top_p is None:
                raise ValueError("top_p must be specified for nucleus sampling")

            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)

            indices_to_remove = cumulative_probs > top_p
            indices_to_remove[1:] = indices_to_remove[:-1]
            indices_to_remove[0] = False

            indices_to_remove_orig = sorted_indices[indices_to_remove]

            nucleus_probs = np.copy(probs)
            nucleus_probs[indices_to_remove_orig] = 0
            nucleus_probs /= np.sum(nucleus_probs)
            next_token = np.random.choice(len(probs), p=nucleus_probs)

        elif method == "random":
             next_token = np.random.choice(len(probs), p=probs)

        else:
            raise ValueError(f"Unknown sampling method: {method}")

        return int(next_token)

    def generate_with_kb(self, prompt, generation_method="greedy", **kwargs):
        context_docs = self.knowledge_base.retrieve(prompt, k=1)
        if context_docs:
            context = " ".join(context_docs)
            augmented_prompt = f"context: {context} question: {prompt}"
            print(f"INFO: Augmented prompt with context: '{augmented_prompt}'")
        else:
            augmented_prompt = prompt
        token_ids = tokenize(augmented_prompt)
        if len(token_ids) == 0:
            print("Warning: Prompt resulted in empty token sequence. Cannot predict.")
            return None
        return self.predict(np.array(token_ids), method=generation_method, **kwargs)
