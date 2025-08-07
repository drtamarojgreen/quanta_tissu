from .config import vocab, inv_vocab

class Tokenizer:
    """
    A simple tokenizer class that encapsulates tokenization logic.
    Handles conversion between text and token IDs using a fixed vocabulary.
    """
    
    def __init__(self, vocab=None, inv_vocab=None):
        """
        Initialize the tokenizer with vocabulary mappings.
        
        Args:
            vocab: Dictionary mapping words to token IDs
            inv_vocab: Dictionary mapping token IDs to words
        """
        self.vocab = vocab if vocab is not None else globals()['vocab']
        self.inv_vocab = inv_vocab if inv_vocab is not None else globals()['inv_vocab']
        self.unk_token = "<unk>"
        self.pad_token = "<pad>"
    
    def tokenize(self, text: str) -> np.ndarray:
        """
        Convert text to a NumPy array of token IDs.
        
        Args:
            text: Input text string
            
        Returns:
            NumPy array of token IDs
        """
        if not isinstance(text, str):
            raise ValueError("Input must be a string")
            
        tokens = []
        for word in text.lower().split():
            tokens.append(self.vocab.get(word, self.vocab[self.unk_token]))
        return np.array(tokens, dtype=np.int32)
    
    def detokenize(self, token_ids: np.ndarray) -> str:
        """
        Convert an array of token IDs back to text.
        
        Args:
            token_ids: NumPy array of token IDs
            
        Returns:
            Reconstructed text string
        """
        if not isinstance(token_ids, np.ndarray):
            token_ids = np.array(token_ids)
            
        return " ".join(self.inv_vocab.get(int(t), self.unk_token) for t in token_ids)
    
    def get_vocab_size(self) -> int:
        """Return the size of the vocabulary."""
        return len(self.vocab)
    
    def get_token_id(self, token: str) -> int:
        """Get the ID for a specific token."""
        return self.vocab.get(token, self.vocab[self.unk_token])
    
    def get_token(self, token_id: int) -> str:
        """Get the token for a specific ID."""
        return self.inv_vocab.get(token_id, self.unk_token)

# Maintain backward compatibility with existing function-based interface
def tokenize(text):
    """Legacy function for backward compatibility."""
    tokenizer = Tokenizer()
    return tokenizer.tokenize(text)


def detokenize(token_ids):
    """Legacy function for backward compatibility."""
    tokenizer = Tokenizer()
    return tokenizer.detokenize(token_ids)
