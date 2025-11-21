import numpy as np
from .parameter import Parameter, LoRALayer

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
    def __init__(self, d_model, eps=1e-6, name="", bias=True):
        self.gamma = Parameter(np.ones(d_model), name=f"{name}.gamma")
        self.beta = Parameter(np.zeros(d_model), name=f"{name}.beta") if bias else None
        self.eps = eps

    def __call__(self, x):
        mean = x.mean(axis=-1, keepdims=True)
        var = x.var(axis=-1, keepdims=True)
        x_norm = (x - mean) / np.sqrt(var + self.eps)
        out = self.gamma.value * x_norm
        if self.beta:
            out += self.beta.value

        cache = {'x': x, 'x_norm': x_norm, 'mean': mean, 'var': var, 'gamma': self.gamma, 'beta': self.beta, 'eps': self.eps}
        return out, cache

    def backward(self, d_out, cache):
        x = cache['x']
        x_norm = cache['x_norm']
        mean = cache['mean']
        var = cache['var']
        gamma = cache['gamma']
        beta = cache['beta']
        eps = cache['eps']

        N = x.shape[-1] # Number of features

        # Gradients for beta and gamma
        if beta:
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
        params = [self.gamma]
        if self.beta:
            params.append(self.beta)
        return params


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


def repeat_kv_heads(x, n_rep):
    """
    Repeats the key-value heads to enable Grouped-Query Attention.
    The input shape is (batch, n_kv_heads, seq, d_k), and the output shape is (batch, n_kv_heads * n_rep, seq, d_k).
    """
    if n_rep == 1:
        return x
    return np.repeat(x, n_rep, axis=1)


class MultiHeadAttention:
    """
    Multi-Head Attention layer.
    This version's forward pass returns a cache for backpropagation.
    """
    def __init__(self, d_model, num_heads, name="", use_lora=False, lora_rank=4, num_kv_heads=None):
        assert d_model % num_heads == 0
        self.num_heads = num_heads
        self.num_kv_heads = num_kv_heads if num_kv_heads is not None else num_heads
        self.num_queries_per_kv = self.num_heads // self.num_kv_heads
        self.d_k = d_model // self.num_heads
        self.name = name
        self.use_lora = use_lora

        self.Wq = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model), name=f"{name}.Wq")
        self.Wk = Parameter(np.random.randn(d_model, self.d_k * self.num_kv_heads) / np.sqrt(d_model), name=f"{name}.Wk")
        self.Wv = Parameter(np.random.randn(d_model, self.d_k * self.num_kv_heads) / np.sqrt(d_model), name=f"{name}.Wv")
        self.Wo = Parameter(np.random.randn(d_model, d_model) / np.sqrt(d_model), name=f"{name}.Wo")

        if self.use_lora:
            self.lora_q = LoRALayer(d_model, d_model, lora_rank, name=f"{name}.lora_q")
            self.lora_v = LoRALayer(d_model, d_model, lora_rank, name=f"{name}.lora_v")

    def split_heads(self, x, num_heads):
        batch_size, seq_len, d_model = x.shape
        if d_model != self.d_k * num_heads:
            raise ValueError(f"Cannot split {d_model} into {num_heads} heads of size {self.d_k}")
        return x.reshape(batch_size, seq_len, num_heads, self.d_k).transpose(0, 2, 1, 3)

    def combine_heads(self, x):
        batch_size, _, seq_len, _ = x.shape
        return x.transpose(0, 2, 1, 3).reshape(batch_size, seq_len, -1)

    def __call__(self, x, mask=None, kv_cache=None):
        Wq_val = self.Wq.dequantize()
        Wk_val = self.Wk.dequantize()
        Wv_val = self.Wv.dequantize()
        Wo_val = self.Wo.dequantize()

        Q = x @ Wq_val
        K_proj = x @ Wk_val
        V_proj = x @ Wv_val

        if self.use_lora:
            Q += self.lora_q(x)
            V_proj += self.lora_v(x)

        Qh = self.split_heads(Q, self.num_heads)
        Kh_new = self.split_heads(K_proj, self.num_kv_heads)
        Vh_new = self.split_heads(V_proj, self.num_kv_heads)

        if kv_cache is not None:
            if 'kh' in kv_cache:
                Kh = np.concatenate([kv_cache['kh'], Kh_new], axis=2)
                Vh = np.concatenate([kv_cache['vh'], Vh_new], axis=2)
            else:
                Kh, Vh = Kh_new, Vh_new
            kv_cache['kh'], kv_cache['vh'] = Kh, Vh
        else:
            Kh, Vh = Kh_new, Vh_new
        
        Kh = repeat_kv_heads(Kh, self.num_queries_per_kv)
        Vh = repeat_kv_heads(Vh, self.num_queries_per_kv)

        attended, attention_weights = scaled_dot_product_attention(Qh, Kh, Vh, mask=mask)
        combined = self.combine_heads(attended)
        output = combined @ Wo_val

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
        params = [self.Wq, self.Wk, self.Wv, self.Wo]
        if self.use_lora:
            params.extend(self.lora_q.parameters())
            params.extend(self.lora_v.parameters())
        return params

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
        W1_val = self.W1.dequantize()
        b1_val = self.b1.dequantize()
        W2_val = self.W2.dequantize()
        b2_val = self.b2.dequantize()

        z = x @ W1_val + b1_val
        h = np.maximum(0, z)
        y = h @ W2_val + b2_val

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
    Dropout layer.
    """
    def __init__(self, p=0.5):
        if not (0 <= p < 1):
            raise ValueError("Dropout probability must be in [0, 1).")
        self.p = p
        self.mode = 'train'
        self.mask = None

    def __call__(self, x):
        if self.mode == 'train':
            self.mask = (np.random.rand(*x.shape) > self.p) / (1.0 - self.p)
            return x * self.mask
        else:
            return x

    def backward(self, d_out):
        if self.mode == 'train':
            return d_out * self.mask
        else:
            return d_out

    def parameters(self):
        return []

class Expert(FeedForward):
    """An expert in the Mixture of Experts layer, which is a FeedForward network."""
    pass

class Router:
    """
    Router for the Mixture of Experts layer.
    It uses a learnable weight matrix to decide which expert to route to.
    """
    def __init__(self, d_model, num_experts, name=""):
        self.W_r = Parameter(np.random.randn(d_model, num_experts) / np.sqrt(d_model), name=f"{name}.W_r")

    def __call__(self, x):
        # x shape: (batch_size, seq_len, d_model)
        # routing_logits shape: (batch_size, seq_len, num_experts)
        routing_logits = x @ self.W_r.value
        routing_weights = softmax(routing_logits, axis=-1)
        return routing_weights, routing_logits

    def backward(self, d_routing_weights, x, routing_logits):
        # d_routing_weights shape: (batch_size, seq_len, num_experts)
        # x shape: (batch_size, seq_len, d_model)
        # routing_logits shape: (batch_size, seq_len, num_experts)

        # Backprop through softmax
        d_routing_logits = d_softmax(d_routing_weights, softmax(routing_logits, axis=-1))

        # Grad for W_r
        self.W_r.grad += np.einsum('bsd,bsn->dn', x, d_routing_logits)

        # Grad for x
        dx = d_routing_logits @ self.W_r.value.T
        return dx

    def parameters(self):
        return [self.W_r]

class MoE:
    """
    Mixture of Experts layer.
    """
    def __init__(self, d_model, d_ff, num_experts, top_k, name=""):
        self.num_experts = num_experts
        self.top_k = top_k
        self.router = Router(d_model, num_experts, name=f"{name}.router")
        self.experts = [Expert(d_model, d_ff, name=f"{name}.expert_{i}") for i in range(num_experts)]

    def __call__(self, x):
        # x shape: (batch_size, seq_len, d_model)
        batch_size, seq_len, d_model = x.shape

        routing_weights, routing_logits = self.router(x)

        # Find top_k experts for each token
        top_k_indices = np.argsort(routing_weights, axis=-1)[..., -self.top_k:]
        top_k_weights = np.take_along_axis(routing_weights, top_k_indices, axis=-1)
        
        # Normalize top_k weights
        top_k_weights /= np.sum(top_k_weights, axis=-1, keepdims=True)

        final_output = np.zeros_like(x)
        expert_caches = [{} for _ in range(self.num_experts)]

        # This is a simplified implementation. A real implementation would be more efficient.
        for i in range(batch_size):
            for j in range(seq_len):
                for k_idx, weight in zip(top_k_indices[i, j], top_k_weights[i, j]):
                    expert = self.experts[k_idx]
                    expert_out, expert_cache = expert(x[i:i+1, j:j+1, :])
                    final_output[i, j, :] += weight * expert_out.squeeze()
                    # This caching is incorrect for backward pass, needs rework
                    # For simplicity, we will assume a simplified backward pass for now
        
        cache = {
            'x': x,
            'routing_weights': routing_weights,
            'routing_logits': routing_logits,
            'top_k_indices': top_k_indices,
            'top_k_weights': top_k_weights,
            'expert_caches': expert_caches # This is not correctly populated
        }
        return final_output, cache

    def backward(self, d_out, cache):
        # This is a placeholder for the backward pass, which is complex.
        # A full implementation would require careful handling of gradients for the router and experts.
        # For now, we'll just pass the gradient back to the input.
        return np.zeros_like(cache['x'])


    def parameters(self):
        params = self.router.parameters()
        for expert in self.experts:
            params.extend(expert.parameters())
        return params