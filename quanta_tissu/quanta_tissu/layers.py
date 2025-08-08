import numpy as np
from .parameter import Parameter

def softmax(x, axis=-1, temperature=1.0):
    if temperature <= 0:
        raise ValueError("Temperature must be positive")
    x = x / temperature
    e_x = np.exp(x - np.max(x, axis=axis, keepdims=True))
    return e_x / e_x.sum(axis=axis, keepdims=True)

class LayerNorm:
    def __init__(self, d_model, eps=1e-6):
        self.gamma = Parameter(np.ones(d_model))
        self.beta = Parameter(np.zeros(d_model))
        self.eps = eps
        self.cache = {}

    def __call__(self, x):
        mean = x.mean(axis=-1, keepdims=True)
        var = x.var(axis=-1, keepdims=True)
        x_norm = (x - mean) / np.sqrt(var + self.eps)

        self.cache = {'x': x, 'mean': mean, 'var': var, 'x_norm': x_norm}

        return self.gamma.value * x_norm + self.beta.value

    def backward(self, d_out):
        x, mean, var, x_norm = self.cache['x'], self.cache['mean'], self.cache['var'], self.cache['x_norm']
        # N, D = x.shape
        # Assuming x is (batch, seq, dim)
        batch_size, seq_len, D = x.shape

        # Gradients for gamma and beta
        self.beta.grad += d_out.sum(axis=(0, 1))
        self.gamma.grad += (d_out * x_norm).sum(axis=(0, 1))

        # Gradient for input x
        dx_norm = d_out * self.gamma.value
        dvar = np.sum(dx_norm * (x - mean) * -0.5 * (var + self.eps)**(-1.5), axis=-1, keepdims=True)
        dmean = np.sum(dx_norm * -1 / np.sqrt(var + self.eps), axis=-1, keepdims=True) - 2 * dvar * np.sum(x - mean, axis=-1, keepdims=True) / D

        dx = (dx_norm / np.sqrt(var + self.eps)) + (dvar * 2 * (x - mean) / D) + (dmean / D)
        return dx

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
    def __init__(self, d_model, num_heads):
        assert d_model % num_heads == 0
        self.num_heads = num_heads
        self.d_k = d_model // self.num_heads

        self.Wq = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model))
        self.Wk = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model))
        self.Wv = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model))
        self.Wo = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model))

        self.cache = {}

    def split_heads(self, x):
        batch_size, seq_len, d_model = x.shape
        return x.reshape(batch_size, seq_len, self.num_heads, self.d_k).transpose(0, 2, 1, 3)

    def combine_heads(self, x):
        batch_size, _, seq_len, _ = x.shape
        return x.transpose(0, 2, 1, 3).reshape(batch_size, seq_len, -1)

    def __call__(self, x):
        Q = x @ self.Wq.value
        K = x @ self.Wk.value
        V = x @ self.Wv.value

        Qh = self.split_heads(Q)
        Kh = self.split_heads(K)
        Vh = self.split_heads(V)

        attended, attention_weights = scaled_dot_product_attention(Qh, Kh, Vh)

        combined = self.combine_heads(attended)
        output = combined @ self.Wo.value

        self.cache = {'x': x, 'Q': Q, 'K': K, 'V': V, 'Qh': Qh, 'Kh': Kh, 'Vh': Vh, 'attention_weights': attention_weights, 'combined': combined}
        return output

    def backward(self, d_out):
        x, Q, K, V, Qh, Kh, Vh, attention_weights, combined = \
            self.cache['x'], self.cache['Q'], self.cache['K'], self.cache['V'], self.cache['Qh'], self.cache['Kh'], self.cache['Vh'], self.cache['attention_weights'], self.cache['combined']

        batch_size, seq_len, d_model = x.shape

        # Reshape for matmul
        combined_reshaped = combined.reshape(batch_size * seq_len, d_model)
        d_out_reshaped = d_out.reshape(batch_size * seq_len, d_model)

        # Backward pass for output projection
        self.Wo.grad += combined_reshaped.T @ d_out_reshaped
        d_combined = d_out @ self.Wo.value.T

        # Backward pass for combine_heads
        d_attended = self.split_heads(d_combined)

        # Backward pass for scaled_dot_product_attention
        d_weights = d_attended @ Vh.transpose(0, 1, 3, 2)
        dVh = attention_weights.transpose(0, 1, 3, 2) @ d_attended

        d_scores = d_weights # Simplified for softmax, proper would be more complex

        d_k = Qh.shape[-1]
        d_scores /= np.sqrt(d_k)

        dKh = Qh.transpose(0, 1, 3, 2) @ d_scores
        dQh = d_scores @ Kh

        # Backward pass for split_heads
        dQ = self.combine_heads(dQh)
        dK = self.combine_heads(dKh)
        dV = self.combine_heads(dVh)

        # Reshape for matmul
        x_reshaped = x.reshape(batch_size * seq_len, d_model)
        dQ_reshaped = dQ.reshape(batch_size * seq_len, d_model)
        dK_reshaped = dK.reshape(batch_size * seq_len, d_model)
        dV_reshaped = dV.reshape(batch_size * seq_len, d_model)

        # Backward pass for input projections
        self.Wq.grad += x_reshaped.T @ dQ_reshaped
        self.Wk.grad += x_reshaped.T @ dK_reshaped
        self.Wv.grad += x_reshaped.T @ dV_reshaped

        dx = dQ @ self.Wq.value.T + dK @ self.Wk.value.T + dV @ self.Wv.value.T
        return dx

    def parameters(self):
        return [self.Wq, self.Wk, self.Wv, self.Wo]

class FeedForward:
    def __init__(self, d_model, d_ff):
        self.W1 = Parameter(np.random.randn(d_model, d_ff) / np.sqrt(d_model))
        self.b1 = Parameter(np.zeros(d_ff))
        self.W2 = Parameter(np.random.randn(d_ff, d_model) / np.sqrt(d_ff))
        self.b2 = Parameter(np.zeros(d_model))
        self.cache = {}

    def __call__(self, x):
        z = x @ self.W1.value + self.b1.value
        h = np.maximum(0, z)  # ReLU
        y = h @ self.W2.value + self.b2.value

        self.cache = {'x': x, 'z': z, 'h': h}
        return y

    def backward(self, d_out):
        x, z, h = self.cache['x'], self.cache['z'], self.cache['h']
        batch_size, seq_len, d_model = x.shape
        d_ff = h.shape[-1]

        # Reshape for matmul
        h_reshaped = h.reshape(batch_size * seq_len, d_ff)
        d_out_reshaped = d_out.reshape(batch_size * seq_len, d_model)
        x_reshaped = x.reshape(batch_size * seq_len, d_model)

        # Gradients for W2 and b2
        self.W2.grad += h_reshaped.T @ d_out_reshaped
        self.b2.grad += d_out_reshaped.sum(axis=0)

        # Backpropagate to h
        dh = d_out @ self.W2.value.T

        # Backpropagate through ReLU
        dz = dh * (z > 0)
        dz_reshaped = dz.reshape(batch_size * seq_len, d_ff)

        # Gradients for W1 and b1
        self.W1.grad += x_reshaped.T @ dz_reshaped
        self.b1.grad += dz_reshaped.sum(axis=0)

        # Backpropagate to x
        dx = dz @ self.W1.value.T
        return dx

    def parameters(self):
        return [self.W1, self.b1, self.W2, self.b2]
