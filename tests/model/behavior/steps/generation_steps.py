import os
import numpy as np
from unittest.mock import MagicMock

from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer
from quanta_tissu.tisslm.core.tokenizer import Tokenizer # The actual Tokenizer class

def setup_mock_tokenizer_and_model(test_instance, MockTokenizer, MockQuantaTissu, bpe_tokenizer_instance, checkpoint_path):
    """
    Helper to set up mocked Tokenizer and QuantaTissu model for generation tests.
    """
    # Mock Tokenizer to load our pre-trained BPE tokenizer
    mock_tokenizer_instance = MockTokenizer.return_value
    mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(bpe_tokenizer_instance.encode(x))
    mock_tokenizer_instance.detokenize.side_effect = bpe_tokenizer_instance.decode
    mock_tokenizer_instance.get_vocab_size.return_value = len(bpe_tokenizer_instance.vocab)

    # Mock QuantaTissu model
    mock_model_instance = MockQuantaTissu.return_value
    mock_model_instance.load_weights.return_value = None # load_weights doesn't return anything

    return mock_tokenizer_instance, mock_model_instance
