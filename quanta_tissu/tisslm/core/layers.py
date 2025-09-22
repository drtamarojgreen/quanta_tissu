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
            'layer_instance': self, # To access split_heads/combine_heads
            'K_full': Kh, 'V_full': Vh, 'scores': Qh @ Kh.transpose(0, 1, 3, 2) / np.sqrt(Qh.shape[-1]) # Store full K, V for backward
        }
        return output, cache

    def backward(self, d_out, cache):
        x = cache['x']
        Qh = cache['Qh']
        Kh_full = cache['K_full'] # Use full K from cache
        Vh_full = cache['V_full'] # Use full V from cache
        attention_weights = cache['attention_weights']
        combined = cache['combined']
        Wq = cache['Wq']
        Wk = cache['Wk']
        Wv = cache['Wv']
        Wo = cache['Wo']
        scores = cache['scores']
        
        # 1. Gradient for Wo
        self.Wo.grad += np.einsum('bsi,bsj->ij', combined, d_out)

        # 2. Gradient for combined (output of combine_heads)
        d_combined = d_out @ Wo.value.T

        # 3. Gradient for attended (input to combine_heads)
        d_attended = self.split_heads(d_combined) # This is not correct, need to inverse combine_heads
        # Correct inverse of combine_heads:
        d_attended = d_combined.reshape(d_combined.shape[0], d_combined.shape[1], self.num_heads, self.d_k).transpose(0, 2, 1, 3)

        # 4. Gradients for Q, K, V from scaled_dot_product_attention
        d_Qh, d_Kh, d_Vh = backward_scaled_dot_product_attention(d_attended, Qh, Kh_full, Vh_full, scores, attention_weights)

        # 5. Gradients for Wq, Wk, Wv
        self.Wq.grad += np.einsum('bsi,bsj->ij', x, self.combine_heads(d_Qh))
        self.Wk.grad += np.einsum('bsi,bsj->ij', x, self.combine_heads(d_Kh))
        self.Wv.grad += np.einsum('bsi,bsj->ij', x, self.combine_heads(d_Vh))

        # 6. Gradient for x (input to MHA)
        dx = (self.combine_heads(d_Qh) @ Wq.value.T +
              self.combine_heads(d_Kh) @ Wk.value.T +
              self.combine_heads(d_Vh) @ Wv.value.T)

        return dx

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

    def backward(self, d_out, cache):
        x = cache['x']
        z = cache['z']
        h = cache['h']
        W1 = cache['W1']
        b1 = cache['b1']
        W2 = cache['W2']
        b2 = cache['b2']

        # 1. Gradient for W2 and b2
        self.W2.grad += np.einsum('bsf,bsd->fd', h, d_out)
        self.b2.grad += np.sum(d_out, axis=(0, 1))

        # 2. Gradient for h
        dh = d_out @ W2.value.T

        # 3. Gradient for z (through ReLU)
        dz = d_relu(z, dh)

        # 4. Gradient for W1 and b1
        self.W1.grad += np.einsum('bsd,bsf->df', x, dz)
        self.b1.grad += np.sum(dz, axis=(0, 1))

        # 5. Gradient for x (input to FFN)
        dx = dz @ W1.value.T

        return dx

    def parameters(self):
        return [self.W1, self.b1, self.W2, self.b2]


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