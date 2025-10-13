import unittest
import numpy as np
import os
import shutil
from unittest.mock import MagicMock, patch

# Adjust path to import modules from the project root
import sys
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer
from quanta_tissu.tisslm.core.embedding.embedder import Embedder
from quanta_tissu.tisslm.core.model_error_handler import ModelProcessingError

class TestTokenizerWrapper(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_tokenizer_wrapper_test")
        os.makedirs(self.test_dir, exist_ok=True)
        self.tokenizer_prefix = os.path.join(self.test_dir, "trained_tokenizer")

        # Create a dummy trained BPE tokenizer for testing
        self.dummy_bpe_tokenizer = BPETokenizer()
        self.dummy_bpe_tokenizer.vocab = {0: b'a', 1: b'b', 2: b'c', 3: b'ab', 4: b'abc'}
        self.dummy_bpe_tokenizer.merges = {(0, 1): 3, (3, 2): 4}
        self.dummy_bpe_tokenizer.save(self.tokenizer_prefix)

        # Mock system_config to point to our dummy tokenizer
        self.mock_system_config = MagicMock()
        self.mock_system_config.model_save_path = os.path.join(self.test_dir, "dummy_model.npz")

    def tearDown(self):
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)

    @patch('quanta_tissu.tisslm.core.tokenizer.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.tokenizer.system_config', new_callable=MagicMock)
    def test_tokenizer_loading(self, mock_system_config, MockBPETokenizer):
        mock_system_config.__getitem__.side_effect = lambda key: {'model_save_path': self.mock_system_config.model_save_path}[key]
        
        # Configure the mock BPETokenizer instance
        mock_bpe_instance = MockBPETokenizer.return_value
        mock_bpe_instance.vocab = self.dummy_bpe_tokenizer.vocab
        mock_bpe_instance.merges = self.dummy_bpe_tokenizer.merges
        mock_bpe_instance.load.return_value = None # Simulate successful load

        tokenizer = Tokenizer()
        self.assertIsNotNone(tokenizer.bpe_tokenizer)
        self.assertEqual(tokenizer.bpe_tokenizer.vocab, self.dummy_bpe_tokenizer.vocab)
        self.assertEqual(tokenizer.bpe_tokenizer.merges, self.dummy_bpe_tokenizer.merges)
        MockBPETokenizer.return_value.load.assert_called_once_with(os.path.join(self.test_dir, "trained_tokenizer"))

    @patch('quanta_tissu.tisslm.core.tokenizer.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.tokenizer.system_config', new_callable=MagicMock)
    def test_tokenize_and_detokenize(self, mock_system_config, MockBPETokenizer):
        mock_system_config.__getitem__.side_effect = lambda key: {'model_save_path': self.mock_system_config.model_save_path}[key]

        mock_bpe_instance = MockBPETokenizer.return_value
        mock_bpe_instance.vocab = self.dummy_bpe_tokenizer.vocab
        mock_bpe_instance.merges = self.dummy_bpe_tokenizer.merges
        mock_bpe_instance.load.return_value = None
        mock_bpe_instance.encode.side_effect = lambda x: [ord(c) for c in x] # Simple char to int
        mock_bpe_instance.decode.side_effect = lambda x: "".join([chr(i) for i in x])

        tokenizer = Tokenizer()
        
        text = "abc"
        token_ids = tokenizer.tokenize(text)
        self.assertIsInstance(token_ids, np.ndarray)
        self.assertTrue(len(token_ids) > 0)

        detokenized_text = tokenizer.detokenize(token_ids)
        self.assertEqual(detokenized_text, text)

        text_with_space = "a b c"
        token_ids_space = tokenizer.tokenize(text_with_space)
        detokenized_text_space = tokenizer.detokenize(token_ids_space)
        self.assertEqual(detokenized_text_space, text_with_space)

    @patch('quanta_tissu.tisslm.core.tokenizer.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.tokenizer.system_config', new_callable=MagicMock)
    def test_get_vocab_size(self, mock_system_config, MockBPETokenizer):
        mock_system_config.__getitem__.side_effect = lambda key: {'model_save_path': self.mock_system_config.model_save_path}[key]

        mock_bpe_instance = MockBPETokenizer.return_value
        mock_bpe_instance.vocab = self.dummy_bpe_tokenizer.vocab
        mock_bpe_instance.merges = self.dummy_bpe_tokenizer.merges
        mock_bpe_instance.load.return_value = None

        tokenizer = Tokenizer()
        self.assertEqual(tokenizer.get_vocab_size(), len(self.dummy_bpe_tokenizer.vocab))

    @patch('quanta_tissu.tisslm.core.tokenizer.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.tokenizer.system_config', new_callable=MagicMock)
    def test_get_token_id_and_get_token(self, mock_system_config, MockBPETokenizer):
        mock_system_config.__getitem__.side_effect = lambda key: {'model_save_path': self.mock_system_config.model_save_path}[key]

        mock_bpe_instance = MockBPETokenizer.return_value
        mock_bpe_instance.vocab = self.dummy_bpe_tokenizer.vocab
        mock_bpe_instance.merges = self.dummy_bpe_tokenizer.merges
        mock_bpe_instance.load.return_value = None
        mock_bpe_instance.encode.side_effect = lambda x: [0] if x == "a" else ([1] if x == "b" else [])
        mock_bpe_instance.decode.side_effect = lambda x: "a" if x == [0] else ("b" if x == [1] else "")

        tokenizer = Tokenizer()

        self.assertEqual(tokenizer.get_token_id("a"), 0)
        self.assertEqual(tokenizer.get_token_id("b"), 1)
        self.assertEqual(tokenizer.get_token(0), "a")
        self.assertEqual(tokenizer.get_token(1), "b")

    @patch('quanta_tissu.tisslm.core.tokenizer.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.tokenizer.system_config', new_callable=MagicMock)
    def test_tokenizer_loading_file_not_found(self, mock_system_config, MockBPETokenizer):
        # Point to a non-existent tokenizer prefix
        mock_system_config.__getitem__.side_effect = lambda key: {'model_save_path': os.path.join(self.test_dir, "non_existent_model.npz")}[key]
        
        # Configure the mock BPETokenizer instance to raise FileNotFoundError on load
        MockBPETokenizer.return_value.load.side_effect = FileNotFoundError

        # Capture print output to check warning message
        with patch('builtins.print') as mock_print:
            tokenizer = Tokenizer()
            mock_print.assert_called_with(unittest.mock.ANY) # Check if print was called
            self.assertEqual(len(tokenizer.bpe_tokenizer.vocab), 0) # Vocab should be empty if loading failed

class TestEmbedder(unittest.TestCase):

    def setUp(self):
        self.mock_tokenizer = MagicMock()
        self.mock_tokenizer.tokenize.side_effect = lambda text: np.array([ord(c) for c in text])
        self.embedding_dim = 10
        self.model_embeddings_value = np.random.randn(256, self.embedding_dim) # Vocab size 256

    def test_embed_single_word(self):
        embedder = Embedder(self.mock_tokenizer, self.model_embeddings_value)
        text = "hello"
        embedding = embedder.embed(text)

        self.assertEqual(embedding.shape, (self.embedding_dim,))
        # Manually calculate expected embedding
        token_ids = np.array([ord(c) for c in text])
        expected_embedding = np.mean(self.model_embeddings_value[token_ids], axis=0)
        np.testing.assert_allclose(embedding, expected_embedding, rtol=1e-5, atol=1e-5)

    def test_embed_multiple_words(self):
        embedder = Embedder(self.mock_tokenizer, self.model_embeddings_value)
        text = "hello world"
        embedding = embedder.embed(text)

        self.assertEqual(embedding.shape, (self.embedding_dim,))
        token_ids = np.array([ord(c) for c in text])
        expected_embedding = np.mean(self.model_embeddings_value[token_ids], axis=0)
        np.testing.assert_allclose(embedding, expected_embedding, rtol=1e-5, atol=1e-5)

    def test_embed_empty_text(self):
        embedder = Embedder(self.mock_tokenizer, self.model_embeddings_value)
        text = ""
        embedding = embedder.embed(text)

        self.assertEqual(embedding.shape, (self.embedding_dim,))
        np.testing.assert_allclose(embedding, np.zeros(self.embedding_dim))

    def test_embed_tokenizer_returns_empty(self):
        self.mock_tokenizer.tokenize.side_effect = lambda text: np.array([])
        embedder = Embedder(self.mock_tokenizer, self.model_embeddings_value)
        text = "some text"
        embedding = embedder.embed(text)

        self.assertEqual(embedding.shape, (self.embedding_dim,))
        np.testing.assert_allclose(embedding, np.zeros(self.embedding_dim))

    def test_embed_index_error_handling(self):
        # Make tokenizer return an out-of-bounds token ID
        self.mock_tokenizer.tokenize.side_effect = lambda text: np.array([9999])
        embedder = Embedder(self.mock_tokenizer, self.model_embeddings_value)
        text = "error text"

        with self.assertRaises(ModelProcessingError):
            embedder.embed(text)

if __name__ == '__main__':
    unittest.main()