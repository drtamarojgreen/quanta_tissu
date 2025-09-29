import unittest
import numpy as np
import os
import shutil
from unittest.mock import MagicMock, patch

# Adjust path to import modules from the project root
import sys
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.architecture.llm import Model
from quanta_tissu.tisslm.core.parameter import Parameter
from quanta_tissu.tisslm.core.tokenizer import Tokenizer

class TestQuantaTissuModel(unittest.TestCase):

    def setUp(self):
        self.config = {
            "vocab_size": 256, # ASCII range for simplicity
            "n_layer": 1,
            "n_head": 2,
            "n_embd": 32,
            "d_ff": 64,
            "block_size": 64 # Max sequence length
        }
        self.model = QuantaTissu(self.config, use_db=False) # Don't use DB for model tests

        # Create a dummy tokenizer for the model
        self.mock_tokenizer = MagicMock(spec=Tokenizer)
        self.mock_tokenizer.tokenize.side_effect = lambda text: np.array([ord(c) for c in text])
        self.mock_tokenizer.detokenize.side_effect = lambda token_ids: "".join([chr(i) for i in token_ids])
        self.mock_tokenizer.get_vocab_size.return_value = self.config["vocab_size"]

    def test_forward_pass(self):
        # Create dummy input token IDs
        input_tokens = np.array([[10, 20, 30, 40]]) # Batch=1, Seq_len=4
        
        # Perform forward pass
        logits, model_cache, _ = self.model.forward(input_tokens)

        # Assert output shape
        self.assertEqual(logits.shape, (1, 4, self.config["vocab_size"])) # Batch, Seq_len, Vocab_size
        self.assertIsNotNone(model_cache)

    def test_backward_pass(self):
        # Create dummy input token IDs
        input_tokens = np.array([[10, 20, 30, 40]]) # Batch=1, Seq_len=4

        # Perform forward pass to get cache
        logits, model_cache, _ = self.model.forward(input_tokens)

        # Create dummy gradients for logits
        d_logits = np.random.randn(*logits.shape)

        # Perform backward pass
        self.model.backward(d_logits, model_cache)

        # Assert that gradients are accumulated for parameters
        for param in self.model.parameters():
            self.assertIsNotNone(param.grad)
            self.assertFalse(np.all(param.grad == 0)) # Gradients should not be all zeros

    def test_sample_greedy(self):
        prompt = "hello"
        n_new_tokens = 5
        np.random.seed(42) # For deterministic sampling

        # Mock the underlying Model's forward pass to return predictable logits
        with patch.object(self.model.model, 'forward') as mock_model_forward:
            # Create a logit array where token '2' is always the most likely
            logits = np.full((1, 1, self.config["vocab_size"]), -np.inf)
            logits[0, 0, 2] = 0.9
            logits[0, 0, 1] = 0.2
            logits[0, 0, 0] = 0.1

            # The forward pass will be called multiple times, always return the same logits
            mock_model_forward.return_value = (logits, MagicMock(), MagicMock())

            generated_tokens = self.model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="greedy")

            self.assertEqual(len(generated_tokens), n_new_tokens)
            # The token with highest logit (0.9) is index 2
            self.assertEqual(generated_tokens, [2, 2, 2, 2, 2])

    def test_sample_top_k(self):
        prompt = "test"
        n_new_tokens = 3
        top_k = 2
        np.random.seed(42) # For deterministic sampling

        with patch.object(self.model, 'forward') as mock_quanta_tissu_forward:
            # Simulate logits where top-k sampling can be tested
            mock_quanta_tissu_forward.side_effect = [
                (np.array([[[0.1, 0.8, 0.2, 0.7, 0.0]]]), MagicMock(), MagicMock()), # Logits for last prompt token
                (np.array([[[0.9, 0.1, 0.0, 0.0, 0.0]]]), MagicMock(), MagicMock()), # Logits for first generated token
                (np.array([[[0.0, 0.0, 0.0, 0.5, 0.5]]]), MagicMock(), MagicMock()), # Logits for second generated token
                (np.array([[[0.0, 0.0, 0.0, 0.0, 0.0]]]), MagicMock(), MagicMock()), # Dummy for third generated token
            ]

            # Patch np.random.choice to return the expected sequence of tokens
            with patch('numpy.random.choice', side_effect=[1, 0, 3]):
                generated_tokens = self.model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="top_k", top_k=top_k)

            self.assertEqual(len(generated_tokens), n_new_tokens)
            self.assertEqual(generated_tokens, [1, 0, 3])

    def test_sample_nucleus(self):
        prompt = "prompt"
        n_new_tokens = 4
        top_p = 0.9
        np.random.seed(42) # For deterministic sampling

        with patch.object(self.model, 'forward') as mock_quanta_tissu_forward:
            # Simulate logits for nucleus sampling
            mock_quanta_tissu_forward.side_effect = [
                (np.array([[[0.05, 0.1, 0.7, 0.1, 0.05]]]), MagicMock(), MagicMock()), # Logits for last prompt token
                (np.array([[[0.8, 0.1, 0.05, 0.05, 0.0]]]), MagicMock(), MagicMock()), # Logits for first generated token
                (np.array([[[0.01, 0.01, 0.9, 0.04, 0.04]]]), MagicMock(), MagicMock()), # Logits for second generated token
                (np.array([[[0.0, 0.0, 0.0, 0.95, 0.05]]]), MagicMock(), MagicMock()), # Logits for third generated token
                (np.array([[[0.0, 0.0, 0.0, 0.0, 0.0]]]), MagicMock(), MagicMock()), # Dummy for fourth generated token
            ]

            # Patch np.random.choice to return the expected sequence of tokens
            with patch('numpy.random.choice', side_effect=[2, 0, 2, 3]):
                generated_tokens = self.model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="nucleus", top_p=top_p)

            self.assertEqual(len(generated_tokens), n_new_tokens)
            self.assertEqual(generated_tokens, [2, 0, 2, 3])
if __name__ == '__main__':
    unittest.main()
