import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
sys.path.insert(0, project_root)

import numpy as np
from .bpe_trainer import BPETokenizer
from .config import system_config # Import system_config to get model_save_path

class Tokenizer:
    """
    A tokenizer class that encapsulates tokenization logic using a trained BPETokenizer.
    Handles conversion between text and token IDs.
    """
    
    def __init__(self):
        self.bpe_tokenizer = BPETokenizer()
        # Construct the path to the trained tokenizer files
        tokenizer_prefix = os.path.join(os.path.dirname(system_config["model_save_path"]), "trained_tokenizer")
        if not os.path.exists(os.path.dirname(tokenizer_prefix)):
            os.makedirs(os.path.dirname(tokenizer_prefix))
        try:
            self.bpe_tokenizer.load(tokenizer_prefix)
        except FileNotFoundError:
            print(f"Warning: BPE tokenizer files not found at {tokenizer_prefix}. Please train the tokenizer first using tisslm/train_bpe.py.")
            # Fallback to a minimal tokenizer or raise an error, depending on desired behavior
            # For now, we'll proceed with an empty tokenizer, which will likely cause errors later.
            # A more robust solution would be to train a default one or exit.

        # Special tokens, assuming they are part of the BPE vocabulary or handled externally
        # For BPE, <unk> and <pad> might be handled implicitly or added during training.
        # We'll assume they are present in the BPE vocab for now.
        self.unk_token = "<unk>"
        self.pad_token = "<pad>"
        # These might need to be mapped to actual BPE token IDs if they are not directly bytes
        self.unk_token_id = self.bpe_tokenizer.encode(self.unk_token)[0] if self.bpe_tokenizer.encode(self.unk_token) else 0 # Fallback
        self.pad_token_id = self.bpe_tokenizer.encode(self.pad_token)[0] if self.bpe_tokenizer.encode(self.pad_token) else 1 # Fallback

    def tokenize(self, text: str) -> np.ndarray:
        """
        Convert text to a NumPy array of token IDs using the BPE tokenizer.
        
        Args:
            text: Input text string
            
        Returns:
            NumPy array of token IDs
        """
        if not isinstance(text, str):
            raise ValueError("Input must be a string")
            
        token_ids = self.bpe_tokenizer.encode(text)
        return np.array(token_ids, dtype=np.int32)
    
    def detokenize(self, token_ids: np.ndarray) -> str:
        """
        Convert an array of token IDs back to text using the BPE tokenizer.
        
        Args:
            token_ids: NumPy array of token IDs
            
        Returns:
            Reconstructed text string
        """
        if not isinstance(token_ids, np.ndarray):
            token_ids = np.array(token_ids)
            
        return self.bpe_tokenizer.decode(token_ids.tolist())
    
    def get_vocab_size(self) -> int:
        """Return the size of the vocabulary of the BPE tokenizer."""
        return len(self.bpe_tokenizer.vocab)
    
    def get_token_id(self, token: str) -> int:
        """Get the ID for a specific token using the BPE tokenizer."""
        # BPE tokenizer encodes segments, so this might not be a direct 1:1 mapping for all 'tokens'
        # For single tokens, it should work.
        encoded = self.bpe_tokenizer.encode(token)
        return encoded[0] if encoded else self.unk_token_id
    
    def get_token(self, token_id: int) -> str:
        """Get the token for a specific ID using the BPE tokenizer."""
        # BPE tokenizer decodes IDs to bytes, then to string.
        return self.bpe_tokenizer.decode([token_id])

# Maintain backward compatibility with existing function-based interface
_global_tokenizer_instance = None
def _get_global_tokenizer():
    global _global_tokenizer_instance
    if _global_tokenizer_instance is None:
        _global_tokenizer_instance = Tokenizer()
    return _global_tokenizer_instance

def tokenize(text):
    """Legacy function for backward compatibility, uses the BPE tokenizer."""
    return _get_global_tokenizer().tokenize(text)


def detokenize(token_ids):
    """Legacy function for backward compatibility, uses the BPE tokenizer."""
    return _get_global_tokenizer().detokenize(token_ids)
