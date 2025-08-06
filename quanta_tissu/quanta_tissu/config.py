# --- Tokenizer Configuration ---
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
vocab_size = len(vocab)

# --- Model Configuration ---
model_config = {
    "d_model": 32,
    "n_layers": 2,
    "num_heads": 4,
    "d_ff": 128,
    "vocab_size": vocab_size,
}
