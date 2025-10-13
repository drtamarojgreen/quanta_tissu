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
        logits, model_cache = self.model.forward(input_tokens)

        # Assert output shape
        self.assertEqual(logits.shape, (1, 4, self.config["vocab_size"])) # Batch, Seq_len, Vocab_size
        self.assertIsNotNone(model_cache)

    def test_backward_pass(self):
        # Create dummy input token IDs
        input_tokens = np.array([[10, 20, 30, 40]]) # Batch=1, Seq_len=4

        # Perform forward pass to get cache
        logits, model_cache = self.model.forward(input_tokens)

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

        # Mock the underlying LLM model's forward pass to return predictable logits
        with patch.object(self.model.model, 'forward') as mock_llm_forward:
            # Simulate logits that always lead to the same next token for greedy
            # Logits shape: (batch_size, seq_len, vocab_size)
            # We only care about the last token's logits for generation
            mock_llm_forward.side_effect = [
                (np.array([[[0.1, 0.2, 0.9, 0.0, 0.0]]]), MagicMock()), # Logits for last prompt token (to predict first new token)
                (np.array([[[0.1, 0.2, 0.9, 0.0, 0.0]]]), MagicMock()), # Logits for first generated token
                (np.array([[[0.1, 0.2, 0.9, 0.0, 0.0]]]), MagicMock()), # Logits for second generated token
                (np.array([[[0.1, 0.2, 0.9, 0.0, 0.0]]]), MagicMock()), # Logits for third generated token
                (np.array([[[0.1, 0.2, 0.9, 0.0, 0.0]]]), MagicMock()), # Logits for fourth generated token
                (np.array([[[0.1, 0.2, 0.9, 0.0, 0.0]]]), MagicMock()), # Logits for fifth generated token (dummy)
            ]

            generated_tokens = self.model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="greedy")
            generated_text = self.mock_tokenizer.detokenize(generated_tokens)

            self.assertEqual(len(generated_tokens), n_new_tokens)
            # The token with highest logit (0.9) is index 2
            self.assertEqual(generated_text, "\x02\x02\x02\x02\x02") # chr(2) = ''

    def test_sample_top_k(self):
        prompt = "test"
        n_new_tokens = 3
        top_k = 2
        np.random.seed(42) # For deterministic sampling

        with patch.object(self.model.model, 'forward') as mock_llm_forward:
            # Simulate logits where top-k sampling can be tested
            mock_llm_forward.side_effect = [
                (np.array([[[0.1, 0.8, 0.2, 0.7, 0.0]]]), MagicMock()), # Logits for last prompt token
                (np.array([[[0.9, 0.1, 0.0, 0.0, 0.0]]]), MagicMock()), # Logits for first generated token
                (np.array([[[0.0, 0.0, 0.0, 0.5, 0.5]]]), MagicMock()), # Logits for second generated token
                (np.array([[[0.0, 0.0, 0.0, 0.0, 0.0]]]), MagicMock()), # Dummy for third generated token
            ]

            generated_tokens = self.model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="top_k", top_k=top_k)
            generated_text = self.mock_tokenizer.detokenize(generated_tokens)

            self.assertEqual(len(generated_tokens), n_new_tokens)
            # With seed 42, and top_k=2 for [0.1, 0.8, 0.2, 0.7, 0.0] (indices 1, 3 are top 2)
            # Probs: [0.1, 0.8, 0.2, 0.7, 0.0] -> sorted top_k: [0.8, 0.7] (indices 1, 3)
            # Normalized: [0.8/1.5, 0.7/1.5] -> [0.533, 0.466]
            # np.random.choice with seed 42 on [1, 3] with these probs will pick 1 (0.8)
            # For second token: [0.9, 0.1, 0.0, 0.0, 0.0] -> top_k: [0.9, 0.1] (indices 0, 1)
            # np.random.choice with seed 42 on [0, 1] with normalized probs will pick 0 (0.9)
            # For third token: [0.0, 0.0, 0.0, 0.5, 0.5] -> top_k: [0.5, 0.5] (indices 3, 4)
            # np.random.choice with seed 42 on [3, 4] with normalized probs will pick 3 (0.5)
            self.assertEqual(generated_text, "\x01\x01\x04") # chr(1), chr(1), chr(4)

    def test_sample_nucleus(self):
        prompt = "prompt"
        n_new_tokens = 4
        top_p = 0.9
        np.random.seed(42) # For deterministic sampling

        with patch.object(self.model.model, 'forward') as mock_llm_forward:
            # Simulate logits for nucleus sampling
            mock_llm_forward.side_effect = [
                (np.array([[[0.05, 0.1, 0.7, 0.1, 0.05]]]), MagicMock()), # Logits for last prompt token
                (np.array([[[0.8, 0.1, 0.05, 0.05, 0.0]]]), MagicMock()), # Logits for first generated token
                (np.array([[[0.01, 0.01, 0.9, 0.04, 0.04]]]), MagicMock()), # Logits for second generated token
                (np.array([[[0.0, 0.0, 0.0, 0.95, 0.05]]]), MagicMock()), # Logits for third generated token
                (np.array([[[0.0, 0.0, 0.0, 0.0, 0.0]]]), MagicMock()), # Dummy for fourth generated token
            ]

            generated_tokens = self.model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="nucleus", top_p=top_p)
            generated_text = self.mock_tokenizer.detokenize(generated_tokens)

            self.assertEqual(len(generated_tokens), n_new_tokens)
            # For first token: [0.05, 0.1, 0.7, 0.1, 0.05] -> sorted: [0.7, 0.1, 0.1, 0.05, 0.05] (indices 2, 1, 3, 0, 4)
            # Cumulative: [0.7, 0.8, 0.9, 0.95, 1.0]
            # top_p=0.9, cutoff_idx is 2 (tokens with 0.7, 0.1, 0.1 -> indices 2, 1, 3)
            # np.random.choice with seed 42 on [2, 1, 3] with normalized probs will pick 2 (0.7)
            # For second token: [0.8, 0.1, 0.05, 0.05, 0.0] -> sorted: [0.8, 0.1, 0.05, 0.05, 0.0] (indices 0, 1, 2, 3, 4)
            # Cumulative: [0.8, 0.9, 0.95, 1.0, 1.0]
            # top_p=0.9, cutoff_idx is 1 (tokens with 0.8, 0.1 -> indices 0, 1)
            # np.random.choice with seed 42 on [0, 1] with normalized probs will pick 0 (0.8)
            # For third token: [0.01, 0.01, 0.9, 0.04, 0.04] -> sorted: [0.9, 0.04, 0.04, 0.01, 0.01] (indices 2, 3, 4, 0, 1)
            # Cumulative: [0.9, 0.94, 0.98, 0.99, 1.0]
            # top_p=0.9, cutoff_idx is 0 (token with 0.9 -> index 2)
            # np.random.choice with seed 42 on [2] will pick 2
            # For fourth token: [0.0, 0.0, 0.0, 0.95, 0.05] -> sorted: [0.95, 0.05, 0.0, 0.0, 0.0] (indices 3, 4, 0, 1, 2)
            # Cumulative: [0.95, 1.0, 1.0, 1.0, 1.0]
            # top_p=0.9, cutoff_idx is 0 (token with 0.95 -> index 3)
            # np.random.choice with seed 42 on [3] will pick 3
            self.assertEqual(generated_text, "\x02\x04\x03\x03") # chr(2), chr(4), chr(3), chr(3)

if __name__ == '__main__':
    unittest.main()
