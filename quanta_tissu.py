import numpy as np

# Toy vocabulary
vocab = {
    "hello": 0,
    "world": 1,
    "this": 2,
    "is": 3,
    "a": 4,
    "test": 5,
    ".": 6,
    "<unk>": 7,
}
inv_vocab = {v: k for k, v in vocab.items()}

def tokenize(text):
    tokens = []
    for word in text.lower().split():
        tokens.append(vocab.get(word, vocab["<unk>"]))
    return np.array(tokens)

def detokenize(token_ids):
    return " ".join(inv_vocab.get(t, "<unk>") for t in token_ids)

def softmax(x, axis=-1):
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
    scores = Q @ K.transpose(0, 2, 1) / np.sqrt(d_k)
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

class TransformerBlock:
    def __init__(self, d_model, num_heads, d_ff):
        self.mha = MultiHeadAttention(d_model, num_heads)
        self.ffn = FeedForward(d_model, d_ff)
        self.ln1 = LayerNorm(d_model)
        self.ln2 = LayerNorm(d_model)

    def __call__(self, x):
        attn_out = self.mha(x)
        x = self.ln1(x + attn_out)
        ffn_out = self.ffn(x)
        x = self.ln2(x + ffn_out)
        return x

class PositionalEncoding:
    def __init__(self, d_model, max_len=5000):
        pe = np.zeros((max_len, d_model))
        position = np.arange(0, max_len)[:, np.newaxis]
        div_term = np.exp(np.arange(0, d_model, 2) * (-np.log(10000.0) / d_model))
        pe[:, 0::2] = np.sin(position * div_term)
        pe[:, 1::2] = np.cos(position * div_term)
        self.pe = pe

    def __call__(self, x):
        seq_len = x.shape[0]
        return x + self.pe[:seq_len]

class QuantaTissu:
    def __init__(self, vocab_size, d_model=32, num_heads=4, d_ff=128):
        self.d_model = d_model
        self.embeddings = np.random.randn(vocab_size, d_model) / np.sqrt(d_model)
        self.pos_encoding = PositionalEncoding(d_model)
        self.transformer = TransformerBlock(d_model, num_heads, d_ff)
        self.output_proj = np.random.randn(d_model, vocab_size) / np.sqrt(d_model)

    def forward(self, token_ids):
        x = self.embeddings[token_ids]  # (seq_len, d_model)
        x = self.pos_encoding(x)
        x = self.transformer(x)
        logits = x @ self.output_proj  # (seq_len, vocab_size)
        return logits

    def predict(self, token_ids):
        logits = self.forward(token_ids)
        # Greedy: pick token with highest logit for last position
        next_token = np.argmax(logits[-1])
        return next_token

# Example usage
if __name__ == "__main__":
    model = QuantaTissu(vocab_size=len(vocab))
    prompt = "hello world"
    token_ids = tokenize(prompt)
    print("Input tokens:", token_ids)
    logits = model.forward(token_ids)
    print("Logits shape:", logits.shape)
    next_token = model.predict(token_ids)
    print("Predicted next token:", next_token, inv_vocab.get(next_token, "<unk>"))
