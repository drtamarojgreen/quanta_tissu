import numpy as np

def softmax(x, axis=-1, temperature=1.0):
    if temperature <= 0:
        raise ValueError("Temperature must be positive")
    x = x / temperature
    e_x = np.exp(x - np.max(x, axis=axis, keepdims=True))
    return e_x / e_x.sum(axis=axis, keepdims=True)

class LayerNorm:
    def __init__(self, d_model, eps=1e-6):
        self.gamma = np.ones(d_model)
        self.beta = np.zeros(d_model)
        self.eps = eps

    def __call__(self, x):
        mean = x.mean(-1, keepdims=True)
        var = ((x - mean) ** 2).mean(-1, keepdims=True)
        x_norm = (x - mean) / np.sqrt(var + self.eps)
        return self.gamma * x_norm + self.beta

def scaled_dot_product_attention(Q, K, V):
    d_k = Q.shape[-1]
    scores = Q @ K.transpose(0, 1, 3, 2) / np.sqrt(d_k)
    weights = softmax(scores, axis=-1)
    return weights @ V

class MultiHeadAttention:
    def __init__(self, d_model, num_heads):
        assert d_model % num_heads == 0
        self.num_heads = num_heads
        self.d_k = d_model // num_heads
        self.Wq = np.random.randn(d_model, d_model) / np.sqrt(d_model)
        self.Wk = np.random.randn(d_model, d_model) / np.sqrt(d_model)
        self.Wv = np.random.randn(d_model, d_model) / np.sqrt(d_model)
        self.Wo = np.random.randn(d_model, d_model) / np.sqrt(d_model)

    def split_heads(self, x):
        batch_size = 1
        x = x.reshape(batch_size, -1, self.num_heads, self.d_k)
        return x.transpose(0, 2, 1, 3)

    def combine_heads(self, x):
        batch_size = x.shape[0]
        x = x.transpose(0, 2, 1, 3)
        return x.reshape(batch_size, -1, self.num_heads * self.d_k)[0]

    def __call__(self, x):
        Q = x @ self.Wq
        K = x @ self.Wk
        V = x @ self.Wv
        Qh = self.split_heads(Q)
        Kh = self.split_heads(K)
        Vh = self.split_heads(V)
        attended = scaled_dot_product_attention(Qh, Kh, Vh)
        combined = self.combine_heads(attended)
        output = combined @ self.Wo
        return output

class FeedForward:
    def __init__(self, d_model, d_ff):
        self.W1 = np.random.randn(d_model, d_ff) / np.sqrt(d_model)
        self.b1 = np.zeros(d_ff)
        self.W2 = np.random.randn(d_ff, d_model) / np.sqrt(d_ff)
        self.b2 = np.zeros(d_model)

    def __call__(self, x):
        x = np.maximum(0, x @ self.W1 + self.b1)  # ReLU
        x = x @ self.W2 + self.b2
        return x
