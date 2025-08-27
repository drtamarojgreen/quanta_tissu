import sys
import os
import json
import numpy as np
import unittest

# This is a common pattern to make sure the test can find the source code
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.core.tokenizer import tokenize, detokenize
from tests.test_utils import assert_equal, assert_allclose

class TestTokenizer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Load the vocabulary from the trained tokenizer."""
        vocab_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'models', 'trained_tokenizer_vocab.json'))
        with open(vocab_path, 'r') as f:
            # The vocab is a mapping from string numbers to lists of numbers, convert to int -> int mapping
            cls.vocab = {int(k): v[0] for k, v in json.load(f).items()}

    def test_tokenize_simple(self):
        """Tests tokenization of a simple, known sentence."""
        text = "abc"
        expected_ids = np.array([self.vocab[ord('a')], self.vocab[ord('b')], self.vocab[ord('c')]])
        token_ids = tokenize(text)
        assert_allclose(token_ids, expected_ids, msg="test_tokenize_simple")

    def test_detokenize_simple(self):
        """Tests detokenization of a simple sequence of IDs."""
        token_ids = np.array([self.vocab[ord('a')], self.vocab[ord('b')], self.vocab[ord('c')]])
        expected_text = "abc"
        text = detokenize(token_ids)
        assert_equal(text, expected_text, msg="test_detokenize_simple")

    def test_tokenize_empty_string(self):
        """Tests tokenization of an empty string."""
        text = ""
        expected_ids = np.array([])
        token_ids = tokenize(text)
        assert_allclose(token_ids, expected_ids, msg="test_tokenize_empty_string")

    def test_detokenize_empty_sequence(self):
        """Tests detokenization of an empty sequence."""
        token_ids = np.array([])
        expected_text = ""
        text = detokenize(token_ids)
        assert_equal(text, expected_text, msg="test_detokenize_empty_sequence")

if __name__ == '__main__':
    unittest.main()
