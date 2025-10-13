import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
sys.path.insert(0, project_root)

import numpy as np
from .bpe_trainer import BPETokenizer
from ..config import system_config # Import system_config to get model_save_path

class Tokenizer:
    """
    A tokenizer class that encapsulates tokenization logic using a trained BPETokenizer.
    Handles conversion between text and token IDs.
    """
    
    def __init__(self, tokenizer_prefix=None):
        self.bpe_tokenizer = BPETokenizer()

        if tokenizer_prefix is None:
            try:
                model_path = system_config["model_save_path"]
                model_dir = os.path.dirname(model_path)
                tokenizer_prefix = os.path.join(model_dir, "trained_tokenizer")
            except KeyError:
                print("Warning: 'model_save_path' not in system_config. Tokenizer will be initialized empty.")
                tokenizer_prefix = None
            except Exception as e:
                print(f"Warning: Could not determine tokenizer path from config. Error: {e}")
                tokenizer_prefix = None

        self.load_successful = False
        if tokenizer_prefix:
            try:
                self.bpe_tokenizer.load(tokenizer_prefix)
                self.load_successful = True
            except FileNotFoundError:
                print(f"Warning: BPE tokenizer files not found at {tokenizer_prefix}. Please train the tokenizer first using tisslm/train_bpe.py.")
                self.bpe_tokenizer.vocab = None
            except Exception as e:
                print(f"Error loading BPE tokenizer from {tokenizer_prefix}: {e}")
                self.bpe_tokenizer.vocab = None
        else:
            print("Warning: No tokenizer prefix provided. Tokenizer will be initialized empty.")

        self.unk_token = "<unk>"
        self.pad_token = "<pad>"
        if self.load_successful and self.bpe_tokenizer.vocab:
            self.unk_token_id = self.bpe_tokenizer.encode(self.unk_token)[0] if self.bpe_tokenizer.encode(self.unk_token) else 0
            self.pad_token_id = self.bpe_tokenizer.encode(self.pad_token)[0] if self.bpe_tokenizer.encode(self.pad_token) else 1
        else:
            self.unk_token_id = 0
            self.pad_token_id = 1

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
        Handles space re-insertion based on common BPE practices.
        
        Args:
            token_ids: NumPy array of token IDs
            
        Returns:
            Reconstructed text string
        """
        if not isinstance(token_ids, np.ndarray):
            raise TypeError("Input must be a NumPy array")

        # Decode tokens to a list of strings
        decoded_tokens = [self.bpe_tokenizer.decode([token_id]) for token_id in token_ids.tolist()]
        
        text = "".join(decoded_tokens)
        
        # The BPE tokenizer should ideally handle spaces correctly.
        # If there are still issues with leading spaces, it might be due to how
        # the BPE tokenizer encodes/decodes initial spaces.
        # For now, we assume the BPE tokenizer's decode method is robust.
        if text.startswith(' '):
            text = text[1:]
            
        return text
    
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

import json # Added import

# Maintain backward compatibility with existing function-based interface
_global_tokenizer_instance = None
def _get_global_tokenizer():
    global _global_tokenizer_instance
    if _global_tokenizer_instance is None:
        # Load paths from configuration file
        project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        config_path = os.path.join(project_root, 'quanta_tissu', 'configurations', 'paths.json')
        try:
            with open(config_path, 'r') as f:
                paths_config = json.load(f)
        except FileNotFoundError:
            print(f"Error: Configuration file not found at {config_path}. Cannot initialize global tokenizer.")
            raise

        tokenizer_dir = os.path.join(project_root, paths_config.get("tokenizer_dir"))
        tokenizer_filename_prefix = paths_config.get("tokenizer_filename_prefix")
        full_tokenizer_prefix = os.path.join(tokenizer_dir, tokenizer_filename_prefix)

        _global_tokenizer_instance = Tokenizer(tokenizer_prefix=full_tokenizer_prefix)
    return _global_tokenizer_instance


def tokenize(text):
    """Legacy function for backward compatibility, uses the BPE tokenizer."""
    return _get_global_tokenizer().tokenize(text)


def detokenize(token_ids):
    """Legacy function for backward compatibility, uses the BPE tokenizer."""
    return _get_global_tokenizer().detokenize(token_ids)