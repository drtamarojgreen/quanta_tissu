import os

# --- System Configuration ---
# Defines system-level parameters like file paths.
# Using os.path.join for platform-independent path construction.
_project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
system_config = {
    "_project_root": _project_root, # Added for easier access in run_training.py
    "model_save_path": os.path.join(_project_root, "models", "quanta_tissu.npz"),
    "logs_dir": os.path.join(_project_root, "logs"),
    "bpe_tokenizer_prefix": os.path.join(_project_root, "models", "trained_tokenizer"),
}

# --- Tokenizer Configuration ---
tokenizer_config = {
    "max_len": 100,  # Max sequence length for padding/truncation.
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
   "n_embd": 128,      # The dimensionality of the model's embeddings and hidden states.
    "n_layer": 4,       # The number of Transformer blocks.
    "n_head": 8,      # The number of attention heads in the Multi-Head Attention layers.
    "d_ff": 512,         # The dimensionality of the inner layer of the Feed-Forward Networks (4 * d_model).
    "vocab_size": 8000,  # Will be set dynamically in run_training.py
    "block_size": tokenizer_config["max_len"], # Max sequence length for the model's internal processing.
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
    "batch_size": 1,
    "num_epochs": 5,
    "weight_decay": 0.01,
}