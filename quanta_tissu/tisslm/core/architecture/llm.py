import numpy as np
import logging

from ..layers import MultiHeadAttention, FeedForward, LayerNorm
from ..parameter import Parameter
from ..model_error_handler import InputValidationError

logger = logging.getLogger(__name__)

class TransformerBlock:
    """
    A single block of the Transformer model.
    This version's forward pass returns a cache for backpropagation.
    """
    def __init__(self, d_model, num_heads, d_ff, name=""):
        self.mha = MultiHeadAttention(d_model, num_heads, name=f"{name}.mha")
        self.ffn = FeedForward(d_model, d_ff, name=f"{name}.ffn")
        self.ln1 = LayerNorm(d_model, name=f"{name}.ln1")
        self.ln2 = LayerNorm(d_model, name=f"{name}.ln2")

    def __call__(self, x, mask=None, kv_cache=None):
        """
        Forward pass for the Transformer block, returning output and cache.
        """
        attn_out, mha_cache = self.mha(x, mask=mask, kv_cache=kv_cache)

        x_plus_attn = x + attn_out
        x_norm1, ln1_cache = self.ln1(x_plus_attn)

        ffn_out, ffn_cache = self.ffn(x_norm1)

        x_plus_ffn = x_norm1 + ffn_out
        x_norm2, ln2_cache = self.ln2(x_plus_ffn)

        # Aggregate caches for the backward pass
        cache = {
            'mha_cache': mha_cache,
            'ln1_cache': ln1_cache,
            'ffn_cache': ffn_cache,
            'ln2_cache': ln2_cache,
            'x_plus_attn': x_plus_attn,
            'x_norm1': x_norm1
        }
        return x_norm2, cache

    def backward(self, d_out, cache):
        """
        Backward pass for the Transformer block.
        """
        # Unpack cache
        mha_cache = cache['mha_cache']
        ln1_cache = cache['ln1_cache']
        ffn_cache = cache['ffn_cache']
        ln2_cache = cache['ln2_cache']
        x_plus_attn = cache['x_plus_attn']
        x_norm1 = cache['x_norm1']

        # 1. Backpropagate through ln2 (LayerNorm)
        dx_norm2 = self.ln2.backward(d_out, ln2_cache)

        # 2. Backpropagate through addition (x_norm1 + ffn_out)
        d_ffn_out = dx_norm2
        dx_norm1_from_ffn = dx_norm2

        # 3. Backpropagate through ffn (FeedForward)
        dx_norm1_from_ffn += self.ffn.backward(d_ffn_out, ffn_cache)

        # 4. Backpropagate through ln1 (LayerNorm)
        dx_plus_attn = self.ln1.backward(dx_norm1_from_ffn, ln1_cache)

        # 5. Backpropagate through addition (x + attn_out)
        d_attn_out = dx_plus_attn
        dx_from_attn = dx_plus_attn

        # 6. Backpropagate through mha (MultiHeadAttention)
        dx_from_attn += self.mha.backward(d_attn_out, mha_cache)

        return dx_from_attn

    def parameters(self):
        return self.mha.parameters() + self.ffn.parameters() + self.ln1.parameters() + self.ln2.parameters()

class PositionalEncoding:
    """
    Injects positional information into the input embeddings.
    """
    def __init__(self, d_model, max_len=5000):
        pe = np.zeros((max_len, d_model))
        position = np.arange(0, max_len)[:, np.newaxis]
        div_term = np.exp(np.arange(0, d_model, 2) * (-np.log(10000.0) / d_model))
        pe[:, 0::2] = np.sin(position * div_term)
        pe[:, 1::2] = np.cos(position * div_term)
        self.pe = pe

    def __call__(self, x, start_pos=0):
        seq_len = x.shape[1]
        max_len = self.pe.shape[0]
        if start_pos + seq_len > max_len:
            raise InputValidationError(f"Sequence of length {seq_len} at start position {start_pos} exceeds max positional encoding length of {max_len}.")

        positions = np.arange(start_pos, start_pos + seq_len)
        return x + self.pe[np.newaxis, positions, :]

class Model:
    """
    The core Transformer model architecture.
    This version's forward pass returns logits and a cache for backpropagation.
    """
    def __init__(self, config):
        d_model = config["n_embd"]
        vocab_size = config["vocab_size"]
        n_head = config["n_head"]
        d_ff = config["d_ff"]
        n_layer = config["n_layer"]

        self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
        self.pos_encoding = PositionalEncoding(d_model)
        self.transformer_blocks = [TransformerBlock(d_model, n_head, d_ff, name=f"transformer_blocks.{i}") for i in range(n_layer)]
        self.output_proj = Parameter(np.random.randn(d_model, vocab_size) / np.sqrt(d_model), name="output_proj")

    def _create_causal_mask(self, seq_len):
        return np.triu(np.ones((seq_len, seq_len)), k=1) * -1e9

    def forward(self, token_ids, kv_cache=None, start_pos=0):
        """
        Performs the forward pass of the model, returning logits and a cache.
        """
        _, seq_len = token_ids.shape
        block_caches = []

        mask = self._create_causal_mask(seq_len)
        if mask is not None:
            mask = mask[np.newaxis, np.newaxis, :, :]

        x = self.embeddings.value[token_ids]
        x = self.pos_encoding(x, start_pos=start_pos)

        for i, block in enumerate(self.transformer_blocks):
            layer_kv_cache = kv_cache[i] if kv_cache is not None else None
            x, block_cache = block(x, mask=mask, kv_cache=layer_kv_cache)
            block_caches.append(block_cache)

        logits = x @ self.output_proj.value

        model_cache = {
            'token_ids': token_ids,
            'final_x': x,
            'block_caches': block_caches,
            'embeddings': self.embeddings,
            'output_proj': self.output_proj
        }

        return logits, model_cache

    def backward(self, d_logits, model_cache):
        """
        Performs the backward pass of the model, propagating gradients.
        """
        token_ids = model_cache['token_ids']
        final_x = model_cache['final_x']
        block_caches = model_cache['block_caches']
        embeddings = model_cache['embeddings']
        output_proj = model_cache['output_proj']

        # 1. Backpropagate through output projection
        d_final_x = d_logits @ output_proj.value.T
        output_proj.grad += final_x.reshape(-1, final_x.shape[-1]).T @ d_logits.reshape(-1, d_logits.shape[-1])

        # 2. Backpropagate through transformer blocks in reverse order
        dx = d_final_x
        for i in reversed(range(len(self.transformer_blocks))):
            block = self.transformer_blocks[i]
            block_cache = block_caches[i]
            dx = block.backward(dx, block_cache)

        # 3. Backpropagate through positional encoding (no gradients for PE)
        # 4. Backpropagate through embeddings
        # Gradients for embeddings are accumulated based on token_ids
        embeddings.grad = np.zeros_like(embeddings.value)
        np.add.at(embeddings.grad, token_ids, dx)

    def parameters(self):
        params = [self.embeddings, self.output_proj]
        for block in self.transformer_blocks:
            params.extend(block.parameters())
        return params