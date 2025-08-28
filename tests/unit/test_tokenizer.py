import sys
import os
import unittest
from unittest.mock import patch
import numpy as np

# This is a common pattern to make sure the test can find the source code
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.core.tokenizer import tokenize, detokenize
from helpers.test_utils import assert_equal, assert_allclose

class TestTokenizer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Load the vocabulary from the trained tokenizer."""
        # Note: The path to the vocabulary file might need adjustment if tests are run from a different directory.
        # This path assumes the test is run from the project root.
        vocab_path = os.path.join(os.path.dirname(__file__), '..', '..', 'models', 'trained_tokenizer_vocab.json')
        try:
            with open(vocab_path, 'r') as f:
                # The vocab is a mapping from string numbers to lists of numbers, convert to int -> int mapping
                cls.vocab = {int(k): v[0] for k, v in json.load(f).items()}
        except FileNotFoundError:
            # If the model file is not found, create a dummy vocab for the tests to run without error.
            # This allows testing the logic without depending on the pre-trained model file.
            cls.vocab = {i: i for i in range(256)} # Dummy vocab for ASCII\
            
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer, tokenize, detokenize
from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer
from tests.unit.test_utils import assert_equal, assert_allclose

class TestTokenizer(unittest.TestCase):

    @patch('quanta_tissu.tisslm.core.bpe_trainer.BPETokenizer.load')
    def setUp(self, mock_load):
        """Set up a tokenizer for each test."""
        # Create and train a simple BPE tokenizer for testing
        self.bpe_trainer = BPETokenizer()
        self.bpe_trainer.train("abc", vocab_size=257)

        # Create a Tokenizer instance that uses our trained BPE tokenizer
        # The mock_load patch prevents it from trying to load from a file
        self.tokenizer = Tokenizer()
        self.tokenizer.bpe_tokenizer = self.bpe_trainer

        # Patch the global tokenizer instance used by the legacy functions
        self.patcher = patch('quanta_tissu.tisslm.core.tokenizer._global_tokenizer_instance', self.tokenizer)
        self.patcher.start()

    def tearDown(self):
        self.patcher.stop()

    def test_tokenize_simple(self):
        """Tests tokenization of a simple, known sentence."""
        text = "abc"
        expected_ids = self.bpe_trainer.encode(text)
        token_ids = tokenize(text)
        assert_allclose(np.array(token_ids), np.array(expected_ids), msg="test_tokenize_simple")

    def test_detokenize_simple(self):
        """Tests detokenization of a simple sequence of IDs."""
        token_ids = self.bpe_trainer.encode("abc")
        expected_text = "abc"
        text = detokenize(np.array(token_ids))
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
