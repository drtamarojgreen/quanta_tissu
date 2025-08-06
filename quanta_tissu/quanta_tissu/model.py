import numpy as np
from .layers import MultiHeadAttention, FeedForward, LayerNorm, softmax
from .knowledge_base import KnowledgeBase
from .tokenizer import tokenize

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
    def __init__(self, config):
        self.config = config
        d_model = config["d_model"]
        vocab_size = config["vocab_size"]
        num_heads = config["num_heads"]
        d_ff = config["d_ff"]

        self.d_model = d_model
        self.embeddings = np.random.randn(vocab_size, d_model) / np.sqrt(d_model)
        self.pos_encoding = PositionalEncoding(d_model)
        self.transformer_blocks = [
            TransformerBlock(d_model, num_heads, d_ff) for _ in range(config["n_layers"])
        ]
        self.output_proj = np.random.randn(d_model, vocab_size) / np.sqrt(d_model)

        # Initialize the knowledge base, providing it with the model's embeddings and tokenizer
        self.knowledge_base = KnowledgeBase(self.embeddings, tokenize)

    def generate_with_kb(self, prompt, generation_method="greedy", **kwargs):
        """
        Generates a token by first retrieving context from the knowledge base.
        """
        # 1. Retrieve context from the knowledge base
        context_docs = self.knowledge_base.retrieve(prompt, k=1)

        # 2. Formulate a new prompt with the retrieved context
        if context_docs:
            context = " ".join(context_docs)
            # Use a simple template for context + question
            augmented_prompt = f"context: {context} question: {prompt}"
            print(f"INFO: Augmented prompt with context: '{augmented_prompt}'")
        else:
            augmented_prompt = prompt

        # 3. Tokenize the (potentially augmented) prompt and predict the next token
        token_ids = tokenize(augmented_prompt)
        if len(token_ids) == 0:
            print("Warning: Prompt resulted in empty token sequence. Cannot predict.")
            return None

        return self.predict(token_ids, method=generation_method, **kwargs)

    def forward(self, token_ids):
        x = self.embeddings[token_ids]
        x = self.pos_encoding(x)
        for block in self.transformer_blocks:
            x = block(x)
        logits = x @ self.output_proj
        return logits

    def predict(self, token_ids, method="greedy", temperature=1.0, top_k=None, top_p=None):
        logits = self.forward(token_ids)
        last_logit = logits[-1]

        if method == "greedy":
            next_token = np.argmax(last_logit)
            return next_token

        probs = softmax(last_logit, temperature=temperature)

        if method == "top_k":
            if top_k is None:
                raise ValueError("top_k must be specified for top_k sampling")

            top_k_indices = np.argsort(probs)[-top_k:]
            top_k_probs = np.zeros_like(probs)
            top_k_probs[top_k_indices] = probs[top_k_indices]
            top_k_probs /= np.sum(top_k_probs)
            next_token = np.random.choice(len(probs), p=top_k_probs)

        elif method == "nucleus":
            if top_p is None:
                raise ValueError("top_p must be specified for nucleus sampling")

            sorted_indices = np.argsort(probs)[::-1]
            sorted_probs = probs[sorted_indices]
            cumulative_probs = np.cumsum(sorted_probs)

            indices_to_remove = cumulative_probs > top_p
            indices_to_remove[1:] = indices_to_remove[:-1]
            indices_to_remove[0] = False

            indices_to_remove_orig = sorted_indices[indices_to_remove]

            nucleus_probs = np.copy(probs)
            nucleus_probs[indices_to_remove_orig] = 0
            nucleus_probs /= np.sum(nucleus_probs)
            next_token = np.random.choice(len(probs), p=nucleus_probs)

        elif method == "random":
             next_token = np.random.choice(len(probs), p=probs)

        else:
            raise ValueError(f"Unknown sampling method: {method}")

        return next_token
