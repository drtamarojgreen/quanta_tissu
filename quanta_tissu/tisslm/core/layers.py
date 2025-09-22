import numpy as np
from .parameter import Parameter

def softmax(x, axis=-1, temperature=1.0):
    if temperature <= 0:
        raise ValueError("Temperature must be positive")
    x = x / temperature
    e_x = np.exp(x - np.max(x, axis=axis, keepdims=True))
    return e_x / e_x.sum(axis=axis, keepdims=True)

def d_softmax(d_out, softmax_out, axis=-1):
    # Jacobian of softmax: diag(s) - s*s.T
    # For batch processing, this needs to be applied carefully.
    # A more numerically stable way for d_out * softmax_out is:
    return softmax_out * (d_out - np.sum(d_out * softmax_out, axis=axis, keepdims=True))

def d_relu(x, d_out):
    return d_out * (x > 0)

def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def d_sigmoid(d_out, sigmoid_out):
    return d_out * sigmoid_out * (1 - sigmoid_out)

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

    def backward(self, d_out, cache):
        x = cache['x']
        x_norm = cache['x_norm']
        mean = cache['mean']
        var = cache['var']
        gamma = cache['gamma']
        eps = cache['eps']

        N = x.shape[-1] # Number of features

        # Gradients for beta and gamma
        self.beta.grad += np.sum(d_out, axis=(0, 1))
        self.gamma.grad += np.sum(d_out * x_norm, axis=(0, 1))

        # Gradient for x_norm
        dx_norm = d_out * gamma.value

        # Gradient for variance
        dvar = np.sum(dx_norm * (x - mean) * -0.5 * np.power(var + eps, -1.5), axis=-1, keepdims=True)

        # Gradient for mean
        dmean = np.sum(dx_norm * -1 / np.sqrt(var + eps), axis=-1, keepdims=True)
        dmean += dvar * np.mean(-2 * (x - mean), axis=-1, keepdims=True)

        # Gradient for x
        dx = dx_norm / np.sqrt(var + eps)
        dx += dvar * 2 * (x - mean) / N
        dx += dmean / N

        return dx

    def parameters(self):
        return [self.gamma, self.beta]


class RMSNorm:
    """
    Applies Root Mean Square Layer Normalization.
    """
    def __init__(self, d_model, eps=1e-6, name=""):
        self.eps = eps
        self.gamma = Parameter(np.ones(d_model), name=f"{name}.gamma")

    def __call__(self, x):
        # Calculate the root mean square
        rms = np.sqrt(np.mean(x**2, axis=-1, keepdims=True) + self.eps)
        # Normalize and apply gain
        x_norm = x / rms
        out = self.gamma.value * x_norm

        cache = {'x': x, 'rms': rms, 'x_norm': x_norm, 'gamma': self.gamma}
        return out, cache

    def backward(self, d_out, cache):
        x, rms, x_norm, gamma = cache['x'], cache['rms'], cache['x_norm'], cache['gamma']

        # Gradient for gamma
        self.gamma.grad += np.sum(d_out * x_norm, axis=(0, 1))

        # Gradient for x_norm
        dx_norm = d_out * gamma.value

        # Gradient for rms
        drms = -np.sum(dx_norm * x / (rms**2), axis=-1, keepdims=True)

        # Gradient for x
        dx = dx_norm / rms
        dx += drms * x / (x.shape[-1] * rms)

        return dx

    def parameters(self):
        return [self.gamma]


def precompute_rope_freqs(d_head, max_len, theta=10000.0):
    """
    Precomputes the RoPE frequencies (cos and sin values).
    """
    theta_numerator = np.arange(0, d_head, 2).astype(np.float32)
    theta = 1.0 / (theta ** (theta_numerator / d_head))
    m = np.arange(max_len)
    freqs = np.outer(m, theta)
    return np.cos(freqs), np.sin(freqs)

def apply_rope(x, cos_freqs, sin_freqs):
    """
    Applies RoPE to the input tensor x.
    """
    # x shape: (batch, heads, seq_len, d_head)
    seq_len = x.shape[2]

    # Reshape x to treat pairs of features
    x_reshaped = x.reshape(*x.shape[:-1], -1, 2)

    # Split into real and imaginary parts (metaphorically)
    x_real = x_reshaped[..., 0]
    x_imag = x_reshaped[..., 1]

    # Select frequencies for the given sequence length
    cos = cos_freqs[:seq_len, :]
    sin = sin_freqs[:seq_len, :]

    # Add dimensions for broadcasting to batch and heads
    cos = cos[np.newaxis, np.newaxis, :, :]
    sin = sin[np.newaxis, np.newaxis, :, :]

    # Apply rotation
    x_rotated_real = x_real * cos - x_imag * sin
    x_rotated_imag = x_real * sin + x_imag * cos

    # Combine back
    x_rotated = np.stack((x_rotated_real, x_rotated_imag), axis=-1)

    # Reshape back to original d_head
    return x_rotated.reshape(*x.shape)


def backward_apply_rope(d_out, cos_freqs, sin_freqs):
    """
    Applies the backward pass for RoPE. This is equivalent to applying RoPE
    with the negative angle.
    """
    # d_out shape: (batch, heads, seq_len, d_head)
    seq_len = d_out.shape[2]

    # Reshape d_out to treat pairs of features
    d_out_reshaped = d_out.reshape(*d_out.shape[:-1], -1, 2)

    d_real = d_out_reshaped[..., 0]
    d_imag = d_out_reshaped[..., 1]

    cos = cos_freqs[:seq_len, :]
    sin = sin_freqs[:seq_len, :]

    cos = cos[np.newaxis, np.newaxis, :, :]
    sin = sin[np.newaxis, np.newaxis, :, :]

    # Apply inverse rotation
    dx_real = d_real * cos + d_imag * sin
    dx_imag = -d_real * sin + d_imag * cos

    dx_rotated = np.stack((dx_real, dx_imag), axis=-1)

    return dx_rotated.reshape(*d_out.shape)


def scaled_dot_product_attention(Q, K, V, mask=None):
    d_k = Q.shape[-1]
    scores = Q @ K.transpose(0, 1, 3, 2) / np.sqrt(d_k)
    if mask is not None:
        scores += mask
    weights = softmax(scores, axis=-1)
    output = weights @ V
    return output, weights

def backward_scaled_dot_product_attention(d_out, Q, K, V, scores, weights, mask=None):
    d_V = weights.transpose(0, 1, 3, 2) @ d_out
    d_weights = d_out @ V.transpose(0, 1, 3, 2)
    d_weights = d_softmax(d_weights, weights, axis=-1)

    d_scores = d_weights
    if mask is not None:
        d_scores[mask == -1e9] = 0 # Masked values don't contribute to gradient

    d_Q = d_scores @ K / np.sqrt(Q.shape[-1])
    d_K = d_scores.transpose(0, 1, 3, 2) @ Q / np.sqrt(Q.shape[-1])

    return d_Q, d_K, d_V

class MultiHeadAttention:
    """
    Multi-Head Attention layer with RoPE.
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

    def __call__(self, x, rope_freqs, mask=None, kv_cache=None):
        cos_freqs, sin_freqs = rope_freqs
        Q = x @ self.Wq.value
        K_proj = x @ self.Wk.value
        V_proj = x @ self.Wv.value

        Qh, Kh_new, Vh_new = self.split_heads(Q), self.split_heads(K_proj), self.split_heads(V_proj)

        # Apply RoPE
        Qh_rot = apply_rope(Qh, cos_freqs, sin_freqs)
        Kh_new_rot = apply_rope(Kh_new, cos_freqs, sin_freqs)

        if kv_cache is not None:
            if 'kh' in kv_cache:
                Kh_rot = np.concatenate([kv_cache['kh'], Kh_new_rot], axis=2)
                Vh = np.concatenate([kv_cache['vh'], Vh_new], axis=2)
            else:
                Kh_rot, Vh = Kh_new_rot, Vh_new
            kv_cache['kh'], kv_cache['vh'] = Kh_rot, Vh
        else:
            Kh_rot, Vh = Kh_new_rot, Vh_new
        
        attended, attention_weights = scaled_dot_product_attention(Qh_rot, Kh_rot, Vh, mask=mask)
        combined = self.combine_heads(attended)
        output = combined @ self.Wo.value

        cache = {
            'x': x, 'Qh_rot': Qh_rot, 'Kh_rot': Kh_new_rot, 'Vh': Vh_new,
            'rope_freqs': rope_freqs,
            'attention_weights': attention_weights, 'combined': combined,
            'Wq': self.Wq, 'Wk': self.Wk, 'Wv': self.Wv, 'Wo': self.Wo,
            'layer_instance': self,
            'K_full_rot': Kh_rot, 'V_full': Vh, 'scores': Qh_rot @ Kh_rot.transpose(0, 1, 3, 2) / np.sqrt(Qh_rot.shape[-1])
        }
        return output, cache

    def backward(self, d_out, cache):
        x = cache['x']
        Qh_rot = cache['Qh_rot']
        Kh_full_rot = cache['K_full_rot']
        Vh_full = cache['V_full']
        attention_weights = cache['attention_weights']
        combined = cache['combined']
        rope_freqs = cache['rope_freqs']
        cos_freqs, sin_freqs = rope_freqs

        Wq, Wk, Wv, Wo = cache['Wq'], cache['Wk'], cache['Wv'], cache['Wo']
        scores = cache['scores']
        
        # 1. Gradient for Wo
        self.Wo.grad += combined.reshape(-1, combined.shape[-1]).T @ d_out.reshape(-1, d_out.shape[-1])

        # 2. Gradient for combined
        d_combined = d_out @ Wo.value.T

        # 3. Gradient for attended
        d_attended = self.split_heads(d_combined)

        # 4. Gradients for Q_rot, K_rot, V
        d_Qh_rot, d_Kh_rot, d_Vh = backward_scaled_dot_product_attention(
            d_attended, Qh_rot, Kh_full_rot, Vh_full, scores, attention_weights
        )

        # 5. Backpropagate through RoPE
        d_Qh = backward_apply_rope(d_Qh_rot, cos_freqs, sin_freqs)
        d_Kh = backward_apply_rope(d_Kh_rot, cos_freqs, sin_freqs)

        # 6. Gradients for Wq, Wk, Wv
        d_Q = self.combine_heads(d_Qh)
        d_K = self.combine_heads(d_Kh)
        d_V = self.combine_heads(d_Vh)

        self.Wq.grad += x.reshape(-1, x.shape[-1]).T @ d_Q.reshape(-1, d_Q.shape[-1])
        self.Wk.grad += x.reshape(-1, x.shape[-1]).T @ d_K.reshape(-1, d_K.shape[-1])
        self.Wv.grad += x.reshape(-1, x.shape[-1]).T @ d_V.reshape(-1, d_V.shape[-1])

        # 7. Gradient for x (input to MHA)
        dx = (d_Q @ Wq.value.T + d_K @ Wk.value.T + d_V @ Wv.value.T)

        return dx

    def parameters(self):
        return [self.Wq, self.Wk, self.Wv, self.Wo]

class FeedForward:
    """
    A SwiGLU (Swish-Gated Linear Unit) feed-forward network.
    This version replaces the standard ReLU FFN.
    """
    def __init__(self, d_model, d_ff, name=""):
        # d_ff is the inner dimension of the FFN.
        # For SwiGLU, we often use two linear layers (one gated) followed by an output layer.
        self.W1 = Parameter(np.random.randn(d_model, d_ff) / np.sqrt(d_model), name=f"{name}.W1")
        self.V1 = Parameter(np.random.randn(d_model, d_ff) / np.sqrt(d_model), name=f"{name}.V1") # Gating layer
        self.W2 = Parameter(np.random.randn(d_ff, d_model) / np.sqrt(d_ff), name=f"{name}.W2")

    def __call__(self, x):
        # Forward pass: output = ((x @ W1) * sigmoid(x @ V1)) @ W2
        gate_val = x @ self.V1.value
        g = sigmoid(gate_val)
        h = (x @ self.W1.value) * g
        y = h @ self.W2.value

        cache = {'x': x, 'g': g, 'h': h, 'gate_val': gate_val, 'W1': self.W1, 'V1': self.V1, 'W2': self.W2}
        return y, cache

    def backward(self, d_out, cache):
        # Unpack cache
        x, g, h, gate_val, W1, V1, W2 = \
            cache['x'], cache['g'], cache['h'], cache['gate_val'], cache['W1'], cache['V1'], cache['W2']

        # 1. Gradient for W2
        # dL/dW2 = dL/dy * dy/dW2 = h.T @ d_out
        self.W2.grad += h.reshape(-1, h.shape[-1]).T @ d_out.reshape(-1, d_out.shape[-1])

        # 2. Gradient for h
        # dL/dh = dL/dy * dy/dh = d_out @ W2.T
        dh = d_out @ W2.value.T

        # 3. Gradient for g (from h = (x @ W1) * g)
        # dL/dg = dL/dh * dh/dg = dh * (x @ W1)
        dg = dh * (x @ W1.value)

        # 4. Gradient for V1 (through sigmoid)
        # dL/dV1 = dL/dg * dg/d(gate_val) * d(gate_val)/dV1
        # dg/d(gate_val) is d_sigmoid(g)
        d_gate_val = d_sigmoid(dg, g)
        # d(gate_val)/dV1 = x.T
        self.V1.grad += x.reshape(-1, x.shape[-1]).T @ d_gate_val.reshape(-1, d_gate_val.shape[-1])

        # 5. Gradient for W1
        # dL/dW1 = dL/dh * dh/d(x@W1) * d(x@W1)/dW1
        # dL/dh * dh/d(x@W1) = dh * g
        d_x_w1 = dh * g
        self.W1.grad += x.reshape(-1, x.shape[-1]).T @ d_x_w1.reshape(-1, d_x_w1.shape[-1])

        # 6. Gradient for x (input to FFN)
        # The gradient dL/dx is the sum of gradients from the two paths where x is used:
        # Path 1: Through the linear transformation (x @ W1)
        # Path 2: Through the gating transformation (x @ V1)
        # dL/dx = dL/d(x@W1) * d(x@W1)/dx + dL/d(x@V1) * d(x@V1)/dx
        # dL/dx = (d_x_w1 @ W1.T) + (d_gate_val @ V1.T)
        dx = (d_x_w1 @ W1.value.T) + (d_gate_val @ V1.value.T)

        return dx

    def parameters(self):
        return [self.W1, self.V1, self.W2]


class Dropout:
    """
    Applies Dropout regularization.
    """
    def __init__(self, p=0.5):
        if not (0 <= p < 1):
            raise ValueError("Dropout probability must be in [0, 1)")
        self.p = p
        self.mask = None

    def __call__(self, x, training=True):
        """
        Forward pass for Dropout.

        Args:
            x: The input data.
            training: If True, applies dropout. Otherwise, returns the input untouched.

        Returns:
            The output after applying dropout and the cache for backpropagation.
        """
        if not training or self.p == 0:
            self.mask = None
            cache = {'mask': self.mask}
            return x, cache

        # Inverted dropout: scale up during training
        self.mask = (np.random.rand(*x.shape) > self.p) / (1.0 - self.p)
        out = x * self.mask
        cache = {'mask': self.mask}
        return out, cache

    def backward(self, d_out, cache):
        """
        Backward pass for Dropout.
        """
        mask = cache.get('mask')
        if mask is None: # If not in training mode or p=0
            return d_out
        return d_out * mask

    def parameters(self):
        return []