# --- Tokenizer Configuration ---
# A more comprehensive toy vocabulary.
# The simple tokenizer in `tokenizer.py` splits by space, so "context:" is a single token.
vocab = {
    # Special tokens
    "<unk>": 0,  # Unknown word
    "<pad>": 1,  # Padding token

    # Punctuation
    ".": 2,
    "?": 3,

    # Words from prompt template used in model.py
    "context:": 4,
    "question:": 5,

    # Common English words
    "a": 6, "an": 7, "and": 8, "are": 9, "as": 10, "at": 11,
    "be": 12, "by": 13, "for": 14, "from": 15, "has": 16, "have": 17,
    "he": 18, "hello": 19, "how": 20, "i": 21, "in": 22, "is": 23,
    "it": 24, "of": 25, "on": 26, "that": 27, "the": 28, "this": 29,
    "to": 30, "was": 31, "what": 32, "when": 33, "where": 34,
    "who": 35, "why": 36, "with": 37, "world": 38, "you": 39,

    # Original test word
    "test": 40,
}
inv_vocab = {v: k for k, v in vocab.items()}
vocab_size = len(vocab)

# --- Model Configuration ---
model_config = {
    "d_model": 32,       # The dimensionality of the model's embeddings and hidden states.
    "n_layers": 2,       # The number of Transformer blocks.
    "num_heads": 4,      # The number of attention heads in the Multi-Head Attention layers.
    "d_ff": 128,         # The dimensionality of the inner layer of the Feed-Forward Networks.
    "vocab_size": vocab_size, # The size of the vocabulary.
}
