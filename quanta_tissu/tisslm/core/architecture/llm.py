import numpy as np
import logging

from ..layers import MultiHeadAttention, FeedForward, LayerNorm, Dropout, MoE
from ..convolution import DepthwiseSeparableConv
from ..parameter import Parameter
from ..model_error_handler import InputValidationError

logger = logging.getLogger(__name__)

class TransformerBlock:
    """
    A single block of the Transformer model.
    This version's forward pass returns a cache for backpropagation.
    """
    def __init__(self, d_model, num_heads, d_ff, dropout_p, name="", moe_config=None, bias=True):
        self.mha = MultiHeadAttention(d_model, num_heads, name=f"{name}.mha")
        if moe_config:
            self.ffn = MoE(d_model, d_ff, moe_config['num_experts'], moe_config['top_k'], name=f"{name}.moe")
        else:
            self.ffn = FeedForward(d_model, d_ff, name=f"{name}.ffn")
        self.ln1 = LayerNorm(d_model, name=f"{name}.ln1", bias=bias)
        self.ln2 = LayerNorm(d_model, name=f"{name}.ln2", bias=bias)
        self.dropout1 = Dropout(dropout_p)
        self.dropout2 = Dropout(dropout_p)

    def __call__(self, x, mask=None, kv_cache=None):
        """
        Forward pass for the Transformer block, returning output, cache, and attention weights.
        """
        attn_out, mha_cache = self.mha(x, mask=mask, kv_cache=kv_cache)
        attn_out = self.dropout1(attn_out)

        x_plus_attn = x + attn_out
        x_norm1, ln1_cache = self.ln1(x_plus_attn)

        ffn_out, ffn_cache = self.ffn(x_norm1)
        ffn_out = self.dropout2(ffn_out)

        x_plus_ffn = x_norm1 + ffn_out
        x_norm2, ln2_cache = self.ln2(x_plus_ffn)

        # Aggregate caches for the backward pass
        cache = {
            'mha_cache': mha_cache,
            'ln1_cache': ln1_cache,
            'ffn_cache': ffn_cache,
            'ln2_cache': ln2_cache,
            'x_plus_attn': x_plus_attn,
            'x_norm1': x_norm1,
            'dropout1_mask': self.dropout1.mask,
            'dropout2_mask': self.dropout2.mask
        }
        return x_norm2, cache, mha_cache['attention_weights']

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
        dropout1_mask = cache['dropout1_mask']
        dropout2_mask = cache['dropout2_mask']

        # 1. Backpropagate through ln2 (LayerNorm)
        dx_norm2 = self.ln2.backward(d_out, ln2_cache)

        # 2. Backpropagate through addition (x_norm1 + ffn_out)
        d_ffn_out = dx_norm2
        dx_norm1_from_ffn = dx_norm2

        # 3. Backpropagate through dropout2
        if dropout2_mask is not None:
            d_ffn_out = self.dropout2.backward(d_ffn_out)

        # 4. Backpropagate through ffn (FeedForward)
        dx_norm1_from_ffn += self.ffn.backward(d_ffn_out, ffn_cache)

        # 5. Backpropagate through ln1 (LayerNorm)
        dx_plus_attn = self.ln1.backward(dx_norm1_from_ffn, ln1_cache)

        # 6. Backpropagate through addition (x + attn_out)
        d_attn_out = dx_plus_attn
        dx_from_attn = dx_plus_attn

        # 7. Backpropagate through dropout1
        if dropout1_mask is not None:
            d_attn_out = self.dropout1.backward(d_attn_out)

        # 8. Backpropagate through mha (MultiHeadAttention)
        dx_from_attn += self.mha.backward(d_attn_out, mha_cache)

        return dx_from_attn

    def parameters(self):
        return self.mha.parameters() + self.ffn.parameters() + self.ln1.parameters() + self.ln2.parameters()

class ConvTransformerBlock:
    """
    A Transformer block that uses Depthwise Separable Convolutions instead of Multi-Head Attention.
    """
    def __init__(self, d_model, d_ff, kernel_size, dropout_p, name="", bias=True):
        self.conv = DepthwiseSeparableConv(d_model, d_model, kernel_size, padding=kernel_size//2, name=f"{name}.conv")
        self.ffn = FeedForward(d_model, d_ff, name=f"{name}.ffn")
        self.ln1 = LayerNorm(d_model, name=f"{name}.ln1", bias=bias)
        self.ln2 = LayerNorm(d_model, name=f"{name}.ln2", bias=bias)
        self.dropout1 = Dropout(dropout_p)
        self.dropout2 = Dropout(dropout_p)

    def __call__(self, x, mask=None, kv_cache=None):
        # x shape: (batch_size, seq_len, d_model)
        # Reshape for convolution: (batch_size, d_model, seq_len, 1)
        batch_size, seq_len, d_model = x.shape
        x_reshaped = x.transpose(0, 2, 1)[:, :, :, np.newaxis]

        conv_out, conv_cache = self.conv(x_reshaped)
        conv_out = conv_out.squeeze(3).transpose(0, 2, 1)
        
        conv_out = self.dropout1(conv_out)

        x_plus_conv = x + conv_out
        x_norm1, ln1_cache = self.ln1(x_plus_conv)

        ffn_out, ffn_cache = self.ffn(x_norm1)
        ffn_out = self.dropout2(ffn_out)

        x_plus_ffn = x_norm1 + ffn_out
        x_norm2, ln2_cache = self.ln2(x_plus_ffn)

        cache = {
            'conv_cache': conv_cache,
            'ln1_cache': ln1_cache,
            'ffn_cache': ffn_cache,
            'ln2_cache': ln2_cache,
            'x_plus_conv': x_plus_conv,
            'x_norm1': x_norm1,
            'dropout1_mask': self.dropout1.mask,
            'dropout2_mask': self.dropout2.mask,
            'x_shape': x.shape
        }
        return x_norm2, cache

    def backward(self, d_out, cache):
        conv_cache = cache['conv_cache']
        ln1_cache = cache['ln1_cache']
        ffn_cache = cache['ffn_cache']
        ln2_cache = cache['ln2_cache']
        x_plus_conv = cache['x_plus_conv']
        x_norm1 = cache['x_norm1']
        dropout1_mask = cache['dropout1_mask']
        dropout2_mask = cache['dropout2_mask']
        x_shape = cache['x_shape']

        dx_norm2 = self.ln2.backward(d_out, ln2_cache)

        d_ffn_out = dx_norm2
        dx_norm1_from_ffn = dx_norm2

        if dropout2_mask is not None:
            d_ffn_out = self.dropout2.backward(d_ffn_out)

        dx_norm1_from_ffn += self.ffn.backward(d_ffn_out, ffn_cache)

        dx_plus_conv = self.ln1.backward(dx_norm1_from_ffn, ln1_cache)

        d_conv_out = dx_plus_conv
        dx_from_conv = dx_plus_conv

        if dropout1_mask is not None:
            d_conv_out = self.dropout1.backward(d_conv_out)
            
        d_conv_out = d_conv_out.transpose(0, 2, 1)[:, :, :, np.newaxis]
        dx_from_conv += self.conv.backward(d_conv_out).squeeze(3).transpose(0, 2, 1)

        return dx_from_conv

    def parameters(self):
        return self.conv.parameters() + self.ffn.parameters() + self.ln1.parameters() + self.ln2.parameters()

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
        dropout_p = config.get("dropout_p", 0.0) # Get dropout probability from config, default to 0
        moe_config = config.get("moe")
        self.tie_weights = config.get("tie_weights", False)
        bias = config.get("bias", True)
        use_conv_attention = config.get("use_conv_attention", False)
        kernel_size = config.get("kernel_size", 3)

        self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
        self.pos_encoding = PositionalEncoding(d_model)
        self.dropout = Dropout(dropout_p)
        if use_conv_attention:
            self.transformer_blocks = [ConvTransformerBlock(d_model, d_ff, kernel_size, dropout_p, name=f"transformer_blocks.{i}", bias=bias) for i in range(n_layer)]
        else:
            self.transformer_blocks = [TransformerBlock(d_model, n_head, d_ff, dropout_p, name=f"transformer_blocks.{i}", moe_config=moe_config, bias=bias) for i in range(n_layer)]
        
        if self.tie_weights:
            self.output_proj = self.embeddings
        else:
            self.output_proj = Parameter(np.random.randn(d_model, vocab_size) / np.sqrt(d_model), name="output_proj")

    def _create_causal_mask(self, seq_len):
        return np.triu(np.ones((seq_len, seq_len)), k=1) * -1e9

    def forward(self, token_ids, kv_cache=None, start_pos=0):
        """
        Performs the forward pass of the model, returning logits, a cache, and attention weights.
        """
        _, seq_len = token_ids.shape
        block_caches = []
        attention_weights_list = []

        mask = self._create_causal_mask(seq_len)
        if mask is not None:
            mask = mask[np.newaxis, np.newaxis, :, :]

        x = self.embeddings.value[token_ids]
        x = self.pos_encoding(x, start_pos=start_pos)
        x = self.dropout(x)

        for i, block in enumerate(self.transformer_blocks):
            layer_kv_cache = kv_cache[i] if kv_cache is not None else None
            x, block_cache, attention_weights = block(x, mask=mask, kv_cache=layer_kv_cache)
            block_caches.append(block_cache)
            attention_weights_list.append(attention_weights)

        if self.tie_weights:
            logits = x @ self.output_proj.value.T
        else:
            logits = x @ self.output_proj.value

        model_cache = {
            'token_ids': token_ids,
            'final_x': x,
            'block_caches': block_caches,
            'embeddings': self.embeddings,
            'output_proj': self.output_proj,
            'dropout_mask': self.dropout.mask
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
        dropout_mask = model_cache['dropout_mask']

        # 1. Backpropagate through output projection
        if self.tie_weights:
            d_final_x = d_logits @ output_proj.value
            output_proj.grad += (final_x.reshape(-1, final_x.shape[-1]).T @ d_logits.reshape(-1, d_logits.shape[-1])).T
        else:
            d_final_x = d_logits @ output_proj.value.T
            output_proj.grad += final_x.reshape(-1, final_x.shape[-1]).T @ d_logits.reshape(-1, d_logits.shape[-1])

        # 2. Backpropagate through transformer blocks in reverse order
        dx = d_final_x
        for i in reversed(range(len(self.transformer_blocks))):
            block = self.transformer_blocks[i]
            block_cache = block_caches[i]
            dx = block.backward(dx, block_cache)

        # 3. Backpropagate through dropout
        if dropout_mask is not None:
            dx = self.dropout.backward(dx)

        # 4. Backpropagate through positional encoding (no gradients for PE)
        # 5. Backpropagate through embeddings
        # Gradients for embeddings are accumulated based on token_ids
        if self.tie_weights:
            # The gradient from the output projection is already added to the embeddings
            np.add.at(embeddings.grad, token_ids, dx)
        else:
            embeddings.grad = np.zeros_like(embeddings.value)
            np.add.at(embeddings.grad, token_ids, dx)

    def parameters(self):
        params = [self.embeddings]
        if not self.tie_weights:
            params.append(self.output_proj)
        for block in self.transformer_blocks:
            params.extend(block.parameters())
        return params