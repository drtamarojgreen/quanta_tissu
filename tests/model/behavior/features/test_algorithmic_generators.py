import unittest
import numpy as np
import os
import shutil
from unittest.mock import MagicMock, patch, mock_open

# Adjust path to import modules from the project root
import sys
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.generation.alg_generator import AlgorithmicGenerator
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.retrieval.semantic import BayesianSimilarityStrategy
from quanta_tissu.tisslm.core.model_error_handler import ConfigurationError

class TestAlgorithmicGenerators(unittest.TestCase):

    def setUp(self):
        self.config = {
            "vocab_size": 256, # ASCII range for simplicity
            "n_layer": 1,
            "n_head": 2,
            "n_embd": 32,
            "d_ff": 64,
            "block_size": 64, # Max sequence length
            "_project_root": project_root # For file paths
        }
        self.mock_model = MagicMock(spec=QuantaTissu) # Mock the QuantaTissu model
        self.mock_model.model = MagicMock()
        self.mock_model.model.forward.side_effect = lambda *args, **kwargs: (
            np.zeros((args[0].shape[0], args[0].shape[1], self.config["vocab_size"])), MagicMock()
        )
        # Mock the QuantaTissu.forward method directly
        self.mock_model.forward.side_effect = lambda *args, **kwargs: (
            self.mock_model.model.forward(args[0], *args[1:], **kwargs) # Delegate to the mocked llm.Model.forward
        )
        self.mock_model.embeddings = MagicMock()
        self.mock_model.embeddings.value = np.random.randn(self.config["vocab_size"], self.config["n_embd"])

        self.generator = AlgorithmicGenerator(self.mock_model, self.config)

        self.mock_tokenizer = MagicMock(spec=Tokenizer)
        self.mock_tokenizer.tokenize.side_effect = lambda text: np.array([ord(c) for c in text])
        self.mock_tokenizer.detokenize.side_effect = lambda token_ids: "".join([chr(i) for i in token_ids])

        self.test_dir = os.path.join(project_root, "temp_alg_gen_test")
        os.makedirs(self.test_dir, exist_ok=True)
        self.generator.wordlist_path = os.path.join(self.test_dir, "wordlist.txt")
        self.generator.temp_list_path = os.path.join(self.test_dir, "temp_list.txt")

    def tearDown(self):
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)

    def test_dynamic_token_revision_sampling(self):
        prompt_tokens = np.array([ord('a'), ord('b')])
        n_new_tokens = 5
        
        # Mock model forward to return predictable logits
        self.mock_model.model.forward.side_effect = [
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Logits for first generated token (token 2)
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
        ]

        with patch('builtins.open', mock_open()) as mocked_file_open:
            generated_tokens = self.generator.dynamic_token_revision_sampling(
                prompt_tokens, n_new_tokens, underlying_method="greedy", save_interval=2
            )
            self.assertEqual(len(generated_tokens), n_new_tokens)
            self.assertEqual(self.mock_tokenizer.detokenize(generated_tokens), "\x02\x02\x02\x02\x02")

            # Verify file write operations
            self.assertTrue(mocked_file_open.called)
            # Check content written to temp_list.txt (simplified check)
            handle = mocked_file_open()
            handle.write.assert_called()

    def test_bayesian_word_expansion_sampling(self):
        prompt_tokens = np.array([ord('t'), ord('h'), ord('e')])
        n_new_tokens = 3
        
        # Mock model forward to return predictable logits
        self.mock_model.model.forward.side_effect = [
            (np.zeros((1, len(prompt_tokens), self.config["vocab_size"])), MagicMock()), # Prompt processing
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
        ]

        # Mock _predict_from_logits to return predictable tokens and probabilities
        with patch.object(self.generator, '_predict_from_logits') as mock_predict_from_logits:
            mock_predict_from_logits.side_effect = [
                (ord('a'), np.full(self.config["vocab_size"], 0.1)), # Token 'a', low prob initially
                (ord('b'), np.full(self.config["vocab_size"], 0.9)), # Token 'b', high prob to trigger expansion
                (ord('c'), np.full(self.config["vocab_size"], 0.9)), # Token 'c', high prob to trigger expansion
            ]

            with patch('builtins.open', mock_open()) as mocked_file_open:
                generated_tokens = self.generator.bayesian_word_expansion_sampling(
                    prompt_tokens, n_new_tokens, tokenizer=self.mock_tokenizer, expansion_threshold=0.7
                )
                self.assertEqual(len(generated_tokens), n_new_tokens)
                self.assertEqual(self.mock_tokenizer.detokenize(generated_tokens), "abc")

                # Verify wordlist.txt write operations
                self.assertTrue(mocked_file_open.called)
                handle = mocked_file_open()
                # Check that write was called with expected content (simplified check)
                written_content = "".join([call.args[0] for call in handle.write.call_args_list])
                self.assertIn("the", written_content) # From prompt
                self.assertIn("a", written_content) # From generated token
                self.assertIn("b", written_content) # From generated token
                self.assertIn("c", written_content) # From generated token

    def test_adaptive_sentiment_sampling(self):
        prompt_tokens = np.array([ord('h'), ord('i')])
        n_new_tokens = 4

        mock_sentiment_analyzer = MagicMock()
        mock_sentiment_analyzer.get_sentiment_bias.side_effect = [
            {ord('p'): 0.5}, # Initial bias
            {ord('p'): 0.6}, # Adjusted bias
            {ord('p'): 0.7}, # Adjusted bias
            {ord('p'): 0.8}, # Adjusted bias
        ]
        mock_sentiment_analyzer.sentiment_scores = {
            ord('h'): 0.1, ord('i'): 0.2, ord('p'): 0.5, ord('o'): -0.1 # Example scores
        }

        # Mock _predict_from_logits to return predictable tokens
        with patch.object(self.generator, '_predict_from_logits') as mock_predict_from_logits:
            mock_predict_from_logits.side_effect = [
                (ord('a'), np.zeros(self.config["vocab_size"])), # Token 'a'
                (ord('b'), np.zeros(self.config["vocab_size"])), # Token 'b'
                (ord('c'), np.zeros(self.config["vocab_size"])), # Token 'c'
                (ord('d'), np.zeros(self.config["vocab_size"])), # Token 'd'
            ]

            generated_tokens = self.generator.adaptive_sentiment_sampling(
                prompt_tokens, n_new_tokens, 
                tokenizer=self.mock_tokenizer, 
                sentiment_analyzer=mock_sentiment_analyzer, 
                target_sentiment="positive", 
                target_strength=0.5, 
                evaluation_interval=2
            )

            self.assertEqual(len(generated_tokens), n_new_tokens)
            self.assertEqual(self.mock_tokenizer.detokenize(generated_tokens), "abcd")
            mock_sentiment_analyzer.get_sentiment_bias.assert_called()

    def test_iterative_sampling_dispatch(self):
        prompt_tokens = np.array([ord('x')])
        n_new_tokens = 2

        # Mock model forward to return predictable logits
        self.mock_model.model.forward.side_effect = [
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Logits for first generated token (token 2)
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
            (np.array([[[0.1, 0.2, 0.7, 0.0, 0.0]]]), MagicMock()), # Token 2
        ]

        generated_tokens = self.generator.iterative_sampling(
            prompt_tokens, n_new_tokens, method="greedy"
        )
        self.assertEqual(len(generated_tokens), n_new_tokens)
        self.assertEqual(self.mock_tokenizer.detokenize(generated_tokens), "\x02\x02")

    def test_load_known_words(self):
        # Create a dummy wordlist.txt
        wordlist_content = "apple\nbanana\norange\n"
        with open(self.generator.wordlist_path, "w") as f:
            f.write(wordlist_content)
        
        self.generator._load_known_words()
        self.assertIn("apple", self.generator.known_words)
        self.assertIn("banana", self.generator.known_words)
        self.assertIn("orange", self.generator.known_words)
        self.assertEqual(len(self.generator.known_words), 3)

    def test_save_session_tokens_to_temp_file(self):
        self.generator.session_token_counts[ord('a')] = 5
        self.generator.session_token_counts[ord('b')] = 2

        with patch('builtins.open', mock_open()) as mocked_file_open:
            self.generator._save_session_tokens_to_temp_file()
            mocked_file_open.assert_called_once_with(self.generator.temp_list_path, 'w', encoding='utf-8')
            handle = mocked_file_open()
            # Check that write was called with expected content (order might vary for most_common)
            # We'll check for substrings
            written_content = "".join([call.args[0] for call in handle.write.call_args_list])
            self.assertIn("97: 5", written_content) # ord('a') = 97
            self.assertIn("98: 2", written_content) # ord('b') = 98
            self.assertEqual(len(self.generator.session_token_counts), 0) # Should be cleared

if __name__ == '__main__':
    unittest.main()
