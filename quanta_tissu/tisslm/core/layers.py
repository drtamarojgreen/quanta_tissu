import numpy as np
from .parameter import Parameter

def softmax(x, axis=-1, temperature=1.0):
    if temperature <= 0:
        raise ValueError("Temperature must be positive")
    x = x / temperature
    e_x = np.exp(x - np.max(x, axis=axis, keepdims=True))
    return e_x / e_x.sum(axis=axis, keepdims=True)

class LayerNorm:
    """
    Applies Layer Normalization.
    This version's forward pass returns a cache for backpropagation.
    """
    def __init__(self, d_model, eps=1e-6, name=""):
        self.gamma = Parameter(np.ones(d_model), name=f"{name}.gamma")
        self.beta = Parameter(np.zeros(d_model), name=f"{name}.beta")
        self.eps = eps

    def __call__(self, x):
        mean = x.mean(axis=-1, keepdims=True)
        var = x.var(axis=-1, keepdims=True)
        x_norm = (x - mean) / np.sqrt(var + self.eps)
        out = self.gamma.value * x_norm + self.beta.value

        cache = {'x': x, 'x_norm': x_norm, 'mean': mean, 'var': var, 'gamma': self.gamma, 'beta': self.beta, 'eps': self.eps}
        return out, cache

    def parameters(self):
        return [self.gamma, self.beta]


def scaled_dot_product_attention(Q, K, V, mask=None):
    d_k = Q.shape[-1]
    scores = Q @ K.transpose(0, 1, 3, 2) / np.sqrt(d_k)
    if mask is not None:
        scores += mask
    weights = softmax(scores, axis=-1)
    output = weights @ V
    return output, weights

class MultiHeadAttention:
    """
    Multi-Head Attention layer.
    This version's forward pass returns a cache for backpropagation.
    """
    def __init__(self, d_model, num_heads, name=""):
        assert d_model % num_heads == 0
        self.num_heads = num_heads
        self.d_k = d_model // self.num_heads
        self.name = name
        self.Wq = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model), name=f"{name}.Wq")
        self.Wk = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model), name=f"{name}.Wk")
        self.Wv = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model), name=f"{name}.Wv")
        self.Wo = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model), name=f"{name}.Wo")

    def split_heads(self, x):
        batch_size, seq_len, d_model = x.shape
        return x.reshape(batch_size, seq_len, self.num_heads, self.d_k).transpose(0, 2, 1, 3)

    def combine_heads(self, x):
        batch_size, _, seq_len, _ = x.shape
        return x.transpose(0, 2, 1, 3).reshape(batch_size, seq_len, -1)

    def __call__(self, x, mask=None, kv_cache=None):
        Q = x @ self.Wq.value
        K_proj = x @ self.Wk.value
        V_proj = x @ self.Wv.value
        Qh, Kh_new, Vh_new = self.split_heads(Q), self.split_heads(K_proj), self.split_heads(V_proj)

        if kv_cache is not None:
            if 'kh' in kv_cache:
                Kh = np.concatenate([kv_cache['kh'], Kh_new], axis=2)
                Vh = np.concatenate([kv_cache['vh'], Vh_new], axis=2)
            else:
                Kh, Vh = Kh_new, Vh_new
            kv_cache['kh'], kv_cache['vh'] = Kh, Vh
        else:
            Kh, Vh = Kh_new, Vh_new
        
        attended, attention_weights = scaled_dot_product_attention(Qh, Kh, Vh, mask=mask)
        combined = self.combine_heads(attended)
        output = combined @ self.Wo.value

        cache = {
            'x': x, 'Qh': Qh, 'Kh': Kh_new, 'Vh': Vh_new,
            'attention_weights': attention_weights, 'combined': combined,
            'Wq': self.Wq, 'Wk': self.Wk, 'Wv': self.Wv, 'Wo': self.Wo,
            'layer_instance': self  # To access split_heads/combine_heads
        }
        return output, cache

    def parameters(self):
        return [self.Wq, self.Wk, self.Wv, self.Wo]

class FeedForward:
    """
    A simple feed-forward network (FFN).
    This version's forward pass returns a cache for backpropagation.
    """
    def __init__(self, d_model, d_ff, name=""):
        self.W1 = Parameter(np.random.randn(d_model, d_ff) / np.sqrt(d_model), name=f"{name}.W1")
        self.b1 = Parameter(np.zeros(d_ff), name=f"{name}.b1")
        self.W2 = Parameter(np.random.randn(d_ff, d_model) / np.sqrt(d_ff), name=f"{name}.W2")
        self.b2 = Parameter(np.zeros(d_model), name=f"{name}.b2")

    def __call__(self, x):
        z = x @ self.W1.value + self.b1.value
        h = np.maximum(0, z)
        y = h @ self.W2.value + self.b2.value

        cache = {'x': x, 'z': z, 'h': h, 'W1': self.W1, 'b1': self.b1, 'W2': self.W2, 'b2': self.b2}
        return y, cache

    def parameters(self):
        return [self.W1, self.b1, self.W2, self.b2]