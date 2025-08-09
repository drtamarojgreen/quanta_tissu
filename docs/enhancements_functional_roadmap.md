**Planning API Design and Task Breakdown**

I’m thinking through the task and API design for the modules: Attention, MLP, LayerNorm, and ResidualAdd. I’ll define interfaces with shapes and a config dataclass for flexibility. I want to provide code stubs and a checklist for each milestone with acceptance criteria and tests. For risks, I’ll include complexity considerations and time estimates. The project will move from basic config creation to implementing different components like LayerNorm and MultiBlock. I'll also touch on decoding strategies and set up benchmarks for testing and performance. Eco factor’s not a must, but could be included. Let’s proceed!
# Roadmap for modularizing blocks, stacking multiple blocks, and adding decoding strategies

This roadmap is structured into focused milestones with clear deliverables, acceptance criteria, tests, and example APIs. It assumes a NumPy-only codebase.

---

## Goals and guiding principles

- Clean, testable modules with stable interfaces.
- Extensible stack of decoder blocks.
- Pluggable decoding strategies with consistent APIs.
- Maintain small, readable code.

---

## Milestone 1: Introduce a config and modularize the transformer block

### Tasks
- Add a Config dataclass for model hyperparameters.
- Split the decoder into modules: LayerNorm, MultiHeadSelfAttention, FeedForward, TransformerBlock.
- Centralize masking and positional encoding utilities.
- Add a Model class shell with embedding, positional encoding, stack hook, and lm_head.

### Proposed structure
- quanta_tissu/
  - config.py
  - layers/
    - layer_norm.py
    - attention.py
    - feedforward.py
    - transformer_block.py
  - model/
    - model.py
  - utils/
    - masking.py
    - posenc.py
    - sampling.py (placeholder for Milestone 3)
  - tests/
    - test_layers.py
    - test_model.py
  - examples/
    - quickstart.ipynb

### Key APIs (skeletons)

```python
# config.py
from dataclasses import dataclass

@dataclass
class Config:
    vocab_size: int
    d_model: int
    n_heads: int
    d_ff: int
    n_layers: int
    max_seq_len: int
    dropout: float = 0.0
    use_pre_norm: bool = True
    tie_weights: bool = True
```

```python
# layers/layer_norm.py
import numpy as np

class LayerNorm:
    def __init__(self, d_model, eps=1e-5):
        self.gamma = np.ones((d_model,), dtype=np.float32)
        self.beta = np.zeros((d_model,), dtype=np.float32)
        self.eps = eps

    def __call__(self, x):
        mu = x.mean(axis=-1, keepdims=True)
        var = ((x - mu) ** 2).mean(axis=-1, keepdims=True)
        x_hat = (x - mu) / np.sqrt(var + self.eps)
        return self.gamma * x_hat + self.beta
```

```python
# layers/attention.py
import numpy as np

def causal_mask(T):
    return np.triu(np.ones((T, T), dtype=bool), k=1)

class MultiHeadSelfAttention:
    def __init__(self, d_model, n_heads):
        assert d_model % n_heads == 0
        self.d_model = d_model
        self.n_heads = n_heads
        self.d_head = d_model // n_heads
        # parameter shapes: (d_model, d_model)
        self.Wq = np.random.randn(d_model, d_model).astype(np.float32) / np.sqrt(d_model)
        self.Wk = np.random.randn(d_model, d_model).astype(np.float32) / np.sqrt(d_model)
        self.Wv = np.random.randn(d_model, d_model).astype(np.float32) / np.sqrt(d_model)
        self.Wo = np.random.randn(d_model, d_model).astype(np.float32) / np.sqrt(d_model)

    def __call__(self, x, mask=None, kv_cache=None):
        B, T, D = x.shape
        Q = x @ self.Wq
        K = x @ self.Wk
        V = x @ self.Wv

        # reshape to heads: (B, n_heads, T, d_head)
        def split_heads(t):
            return t.reshape(B, T, self.n_heads, self.d_head).transpose(0, 2, 1, 3)

        Qh, Kh, Vh = split_heads(Q), split_heads(K), split_heads(V)

        scores = (Qh @ Kh.transpose(0,1,3,2)) / np.sqrt(self.d_head)  # (B, H, T, T)
        if mask is not None:
            scores = np.where(mask[None, None, :, :], -1e9, scores)
        attn = np.exp(scores - scores.max(axis=-1, keepdims=True))
        attn = attn / attn.sum(axis=-1, keepdims=True)
        context = attn @ Vh  # (B, H, T, d_head)
        context = context.transpose(0, 2, 1, 3).reshape(B, T, D)
        return context @ self.Wo
```

```python
# layers/feedforward.py
import numpy as np

def gelu(x):
    return 0.5 * x * (1.0 + np.tanh(np.sqrt(2/np.pi) * (x + 0.044715 * x**3)))

class FeedForward:
    def __init__(self, d_model, d_ff):
        self.W1 = np.random.randn(d_model, d_ff).astype(np.float32) / np.sqrt(d_model)
        self.b1 = np.zeros((d_ff,), dtype=np.float32)
        self.W2 = np.random.randn(d_ff, d_model).astype(np.float32) / np.sqrt(d_ff)
        self.b2 = np.zeros((d_model,), dtype=np.float32)

    def __call__(self, x):
        return gelu(x @ self.W1 + self.b1) @ self.W2 + self.b2
```

```python
# layers/transformer_block.py
from .layer_norm import LayerNorm
from .attention import MultiHeadSelfAttention, causal_mask
from .feedforward import FeedForward

class TransformerBlock:
    def __init__(self, cfg):
        self.ln1 = LayerNorm(cfg.d_model)
        self.attn = MultiHeadSelfAttention(cfg.d_model, cfg.n_heads)
        self.ln2 = LayerNorm(cfg.d_model)
        self.ff = FeedForward(cfg.d_model, cfg.d_ff)
        self.use_pre_norm = cfg.use_pre_norm

    def __call__(self, x, apply_mask=True):
        T = x.shape[1]
        mask = causal_mask(T) if apply_mask else None
        if self.use_pre_norm:
            x = x + self.attn(self.ln1(x), mask=mask)
            x = x + self.ff(self.ln2(x))
        else:
            x = self.ln1(x + self.attn(x, mask=mask))
            x = self.ln2(x + self.ff(x))
        return x
```

```python
# model/model.py
import numpy as np
from ..config import Config
from ..layers.transformer_block import TransformerBlock
from ..utils.posenc import sinusoidal_positional_encoding

class Model:
    def __init__(self, cfg: Config):
        self.cfg = cfg
        self.tok_emb = np.random.randn(cfg.vocab_size, cfg.d_model).astype(np.float32) / np.sqrt(cfg.vocab_size)
        self.pos_emb = sinusoidal_positional_encoding(cfg.max_seq_len, cfg.d_model).astype(np.float32)
        self.blocks = [TransformerBlock(cfg) for _ in range(cfg.n_layers)]
        self.final_ln = None  # optional final norm
        self.lm_head = self.tok_emb.T if cfg.tie_weights else np.random.randn(cfg.d_model, cfg.vocab_size).astype(np.float32)

    def forward(self, token_ids):
        B, T = token_ids.shape
        x = self.tok_emb[token_ids] + self.pos_emb[:T]
        for blk in self.blocks:
            x = blk(x, apply_mask=True)
        if self.final_ln is not None:
            x = self.final_ln(x)
        logits = x @ self.lm_head
        return logits
```

### Acceptance criteria
- Single forward pass works with identical outputs to pre-refactor model (within tolerance).
- Unit tests for shapes and mask correctness.
- Example notebook runs greedy decode end-to-end.

### Tests
- Assert attention probabilities sum to 1 on the last axis.
- Check causal mask blocks future tokens.
- Deterministic output with fixed RNG seed.

---

## Milestone 2: Support multiple transformer blocks and KV caching

### Tasks
- Parameterize n_layers in Config; ensure stack composes cleanly.
- Add optional final LayerNorm.
- Implement lightweight KV cache for incremental decoding to cut latency at generation time.

### KV cache API (optional now, ready for Milestone 3)
```python
class KVCache:
    def __init__(self, n_layers, n_heads, max_len, d_head, batch_size):
        self.K = [np.zeros((batch_size, n_heads, 0, d_head), dtype=np.float32) for _ in range(n_layers)]
        self.V = [np.zeros((batch_size, n_heads, 0, d_head), dtype=np.float32) for _ in range(n_layers)]

# Extend attention.__call__(x, kv_cache, layer_idx, new_only=True)
```

### Acceptance criteria
- Changing n_layers changes depth without code edits.
- Optional final norm improves stability but is easy to toggle.
- KV cache passes shape tests; incremental decode matches full decode logits on the last step.

### Tests
- Compare 1-layer vs 2-layer outputs for shape and basic sanity.
- Incremental vs full decoding equivalence at each step for a short sequence.

---

## Milestone 3: Decoding strategies (pluggable and consistent)

### Goals
- Provide a uniform generate API with options: greedy, temperature, top-k, nucleus (top-p), beam search (optional), repetition penalty, EOS handling.

### Sampling utilities

```python
# utils/sampling.py
import numpy as np

def softmax(logits, temperature=1.0, axis=-1):
    z = logits / max(temperature, 1e-6)
    z = z - z.max(axis=axis, keepdims=True)
    p = np.exp(z)
    return p / p.sum(axis=axis, keepdims=True)

def top_k_filter(logits, k):
    if k <= 0 or k >= logits.shape[-1]:
        return logits
    thresh = np.partition(logits, -k, axis=-1)[..., -k][..., None]
    mask = logits < thresh
    return np.where(mask, -1e9, logits)

def top_p_filter(logits, p):
    # Sort and keep minimal set summing to >= p
    sorted_idx = np.argsort(-logits, axis=-1)
    sorted_logits = np.take_along_axis(logits, sorted_idx, axis=-1)
    probs = softmax(sorted_logits, temperature=1.0)
    cum = np.cumsum(probs, axis=-1)
    mask = cum > p
    # Always keep at least 1
    mask[..., 0] = False
    filtered = np.where(mask, -1e9, sorted_logits)
    return np.take_along_axis(filtered, np.argsort(sorted_idx, axis=-1), axis=-1)

def apply_repetition_penalty(logits, generated_ids, penalty=1.2):
    # Decrease scores for tokens already generated
    if penalty <= 1.0 or generated_ids.size == 0:
        return logits
    unique, counts = np.unique(generated_ids, return_counts=True)
    penalized = logits.copy()
    penalized[..., unique] = penalized[..., unique] / (penalty ** counts)
    return penalized
```

### Generate API

```python
# model/model.py (add)
def generate(self, input_ids, max_new_tokens=32, eos_id=None,
             temperature=1.0, top_k=0, top_p=0.0, repetition_penalty=1.0,
             use_kv_cache=False):
    ids = input_ids.copy()
    for _ in range(max_new_tokens):
        logits = self.forward(ids[:, -self.cfg.max_seq_len:])
        next_logits = logits[:, -1, :]  # (B, V)
        next_logits = apply_repetition_penalty(next_logits, ids, repetition_penalty)
        if top_k > 0:
            next_logits = top_k_filter(next_logits, top_k)
        if top_p > 0.0:
            next_logits = top_p_filter(next_logits, top_p)
        probs = softmax(next_logits, temperature=temperature)
        next_id = np.array([np.random.choice(probs.shape[-1], p=probs[i]) for i in range(probs.shape[0])], dtype=ids.dtype)
        ids = np.concatenate([ids, next_id[:, None]], axis=1)
        if eos_id is not None and np.all(next_id == eos_id):
            break
    return ids
```

### Acceptance criteria
- Greedy decode reproduces previous behavior when temperature=0 (handled by k/p filters + argmax variant if desired).
- top-k and top-p produce diverse outputs; APIs can be combined with temperature.
- Repetition penalty reduces immediate loops on small vocab.
- EOS and max token controls work as expected.

### Tests
- With fixed RNG seed, sampling is deterministic.
- top-k=1 matches argmax.
- top-p=1.0 equals unfiltered sampling.
- Repetition penalty reduces probability of repeated token in a crafted logits vector.

---

## Milestone 4: Documentation, examples, and tests

### Tasks
- Write docstrings for each module and function.
- Add an examples/quickstart.ipynb with:
  - Build a small model.
  - Run greedy, top-k, top-p decoding side-by-side.
  - Visualize attention weights on a toy prompt.
- Expand tests to cover decoding branches.

### Acceptance criteria
- README shows a minimal code snippet for initialization and generation.
- Notebook runs from a clean environment in < 2 minutes on CPU.

---

## Milestone 5: Performance and stability polish

### Tasks
- Optional: switch some ops to float32 consistently; avoid implicit float64.
- Micro-optimize attention softmax for numerical stability.
- Add simple timing utilities to report tokens/sec for greedy vs sampling.

### Acceptance criteria
- No major regressions in latency after modularization.
- Documented tokens/sec for 1-layer and N-layer models at a fixed seq length.

---

## Suggested timeline

- Week 1: Milestone 1 (config + modular blocks) with unit tests.
- Week 2: Milestone 2 (multi-block + optional final norm + KV cache stub).
- Week 3: Milestone 3 (sampling strategies + generate API) and tests.
- Week 4: Milestone 4–5 (docs, notebook, perf polish).

---

## Definition of done checklist

- Model can be configured via Config and stacked to N layers.
- Decoding strategies selectable via generate arguments.
- Unit tests cover layers, masks, forward, and sampling utilities.
- Example notebook demonstrates differences between greedy, top-k, and top-p.
- Basic performance numbers recorded in README.

---

## Nice-to-haves after this roadmap

- Beam search for controlled decoding.
- Stop-sequences support and token bias API.
- Tokenizer upgrades (e.g., BPE) with compatible EOS/PAD IDs.
- Simple perplexity evaluation on a tiny corpus to sanity-check improvements.

If you want, we can start by opening a PR that introduces Milestone 1’s structure and a minimal set of tests.
