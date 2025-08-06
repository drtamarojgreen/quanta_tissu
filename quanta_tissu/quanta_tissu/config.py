import os

# --- Vocabulary Definition ---
# A more comprehensive toy vocabulary for the tokenizer.
# The simple tokenizer in `tokenizer.py` splits by space, so "context:" is a single token.
_vocab_list = [
    # --- Special Tokens (0-1) ---
    "<unk>", "<pad>",

    # --- Punctuation & Symbols (2-6) ---
    ".", "?", ",", "!", "'s",

    # --- Prompt/Template Words (7-8) ---
    "context:", "question:",

    # --- TissLang Keywords (9-16) ---
    # These are lowercased as the tokenizer uses .lower()
    "task", "run", "assert", "read", "as", "write", "setup", "step",

    # --- Common English Words & Numbers (17-68) ---
    "a", "an", "and", "are", "at", "be", "by", "come", "for", "from",
    "get", "go", "has", "have", "he", "hello", "how", "i", "in", "is",
    "it", "know", "look", "make", "of", "on", "see", "take", "that",
    "the", "think", "this", "to", "was", "what", "when", "where", "who",
    "why", "with", "world", "you",
    "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
    "test", # Original test word
]
# Generate vocab dictionaries from the list to ensure correct and contiguous indexing.
vocab = {word: i for i, word in enumerate(_vocab_list)}
inv_vocab = {i: word for i, word in enumerate(_vocab_list)}

# --- System Configuration ---
# Defines system-level parameters like file paths.
# Using os.path.join for platform-independent path construction.
_project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
system_config = {
    "model_save_path": os.path.join(_project_root, "models", "quanta_tissu.npz"),
    "logs_dir": os.path.join(_project_root, "logs"),
}

# --- Tokenizer Configuration ---
tokenizer_config = {
    "vocab": vocab,
    "inv_vocab": inv_vocab,
    "max_len": 512,  # Max sequence length for padding/truncation.
}

# --- TissLang Parser Configuration ---
parser_config = {
    # List of greetings recognized by the TissLang parser.
    "greetings": ["hi", "hello", "gm", "good morning", "hey", "greetings"],
}

# --- Knowledge Base Configuration ---
knowledge_base_config = {
    # Number of documents to retrieve from the KB for a given query.
    "retrieval_k": 3,
    # Learning rate for updating document relevance scores based on feedback.
    "feedback_alpha": 0.1,
    # Default confidence score for knowledge added via self-generation.
    "self_gen_confidence": 0.7,
}

# --- Prompting Configuration ---
prompt_config = {
    # Template for augmenting a prompt with retrieved context.
    "context_template": "context: {context} question: {prompt}"
}

# --- Model Configuration ---
model_config = {
    "d_model": 32,       # The dimensionality of the model's embeddings and hidden states.
    "n_layers": 2,       # The number of Transformer blocks.
    "num_heads": 4,      # The number of attention heads in the Multi-Head Attention layers.
    "d_ff": 128,         # The dimensionality of the inner layer of the Feed-Forward Networks.
    "vocab_size": len(vocab), # The size of the vocabulary.
    "layer_norm_eps": 1e-6, # Epsilon for Layer Normalization to prevent division by zero.
    # Max length for positional encodings, tied to tokenizer's max length.
    "positional_encoding_max_len": tokenizer_config["max_len"],
}

# --- Generation Configuration ---
# Default parameters for text generation.
generation_config = {
    "default_method": "greedy", # Can be "greedy", "top_k", "nucleus", "random"
    "temperature": 1.0,         # Controls randomness. Higher is more random.
    "top_k": 10,                # The number of highest probability vocabulary tokens to keep for top-k-filtering.
    "top_p": 0.9,               # The cumulative probability for nucleus sampling.
}

# --- Training Configuration ---
# Parameters for the (future) training loop.
training_config = {
    "learning_rate": 1e-4,
    "batch_size": 32,
    "num_epochs": 5,
    "weight_decay": 0.01,
}
