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
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.data import Dataset, load_corpus
from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
from quanta_tissu.tisslm.core.optimizer import AdamW

class TestFullTrainingIntegration(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_full_training_test")
        os.makedirs(self.test_dir, exist_ok=True)
        self.corpus_dir = os.path.join(self.test_dir, "corpus")
        os.makedirs(self.corpus_dir, exist_ok=True)
        self.model_save_path = os.path.join(self.test_dir, "trained_model.npz")
        self.tokenizer_save_path = os.path.join(self.test_dir, "trained_tokenizer")

        # Create a dummy corpus file
        self.corpus_file_path = os.path.join(self.corpus_dir, "sample.txt")
        with open(self.corpus_file_path, "w", encoding="utf-8") as f:
            f.write("This is a sample text for training. It contains multiple words.")
            f.write("Another line to make the corpus a bit larger.")

        # Create and save a dummy BPE tokenizer
        from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer
        dummy_bpe_tokenizer = BPETokenizer()
        dummy_bpe_tokenizer.vocab = {ord(c): c.encode() for c in "abcdefghijklmnopqrstuvwxyz "}
        dummy_bpe_tokenizer.merges = {}
        dummy_bpe_tokenizer.save(self.tokenizer_save_path)

        self.config = {
            "vocab_size": 256, # ASCII range for simplicity
            "n_layer": 1,
            "n_head": 2,
            "n_embd": 32,
            "d_ff": 64,
            "block_size": 10, # Small block size for quick test
            "max_len": 10, # Added for tokenizer_config
            "learning_rate": 0.01,
            "weight_decay": 0.0,
            "num_epochs": 2, # Small number of epochs for quick test
            "batch_size": 2,
            "model_save_path": self.model_save_path,
            "dropout_p": 0.1 # Changed from 'dropout' to 'dropout_p'
        }

        # Mock Tokenizer to return predictable token IDs
        self.mock_tokenizer = MagicMock(spec=Tokenizer)
        self.mock_tokenizer.tokenize.side_effect = lambda text: np.array([ord(c) for c in text])
        self.mock_tokenizer.detokenize.side_effect = lambda token_ids: "".join([chr(i) for i in token_ids])
        self.mock_tokenizer.get_vocab_size.return_value = self.config["vocab_size"]

    def tearDown(self):
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)

    @patch('quanta_tissu.tisslm.core.data.Tokenizer') # Patch Tokenizer in data.py
    @patch('quanta_tissu.tisslm.config.system_config') # Patch system_config in model.py
    @patch('quanta_tissu.tisslm.config.model_config') # Patch model_config in model.py
    @patch('quanta_tissu.tisslm.config.training_config') # Patch training_config in train.py
    @patch('quanta_tissu.tisslm.config.tokenizer_config') # Patch tokenizer_config in train.py
    @patch('quanta_tissu.tisslm.core.tokenizer.system_config') # Patch system_config in tokenizer.py
    @patch('quanta_tissu.tisslm.core.data.load_corpus') # Patch load_corpus
    def test_full_training_pipeline(self, MockLoadCorpus, mock_tokenizer_system_config, mock_tokenizer_config, mock_training_config, mock_model_config, mock_system_config, MockDataTokenizer):
        # Configure MockLoadCorpus
        MockLoadCorpus.return_value = self.mock_tokenizer.tokenize("This is a sample text for training. It contains multiple words. Another line to make the corpus a bit larger.")
        # Configure mocks
        mock_system_config.__getitem__.side_effect = lambda key: {'model_save_path': self.config["model_save_path"]}[key]
        mock_tokenizer_system_config.__getitem__.side_effect = lambda key: {'model_save_path': self.config["model_save_path"]}[key]

        # Configure model_config to handle both __getitem__ and get
        def model_config_side_effect(key, default=None):
            return self.config.get(key, default)

        mock_model_config.__getitem__.side_effect = lambda key: self.config[key]
        mock_model_config.get.side_effect = lambda key, default: self.config.get(key, default) # Added for get method
        mock_training_config.__getitem__.side_effect = lambda key: self.config[key]
        mock_tokenizer_config.__getitem__.side_effect = lambda key: self.config[key]

        # Mock the Tokenizer used by load_corpus
        MockDataTokenizer.return_value = self.mock_tokenizer

        # Import train after patching to ensure mocks are in place
        from quanta_tissu.tisslm.legacylm.train import train as train_main

        # Run the training pipeline
        train_main()

        # Verify model checkpoint is saved
        self.assertTrue(os.path.exists(self.model_save_path))
        self.assertGreater(os.path.getsize(self.model_save_path), 0)

        # Load the trained model and perform a simple generation test
        loaded_model = QuantaTissu(self.config, use_db=False)
        loaded_model.load_weights(self.model_save_path)

        prompt = "This is a"
        n_new_tokens = 5
        generated_tokens = loaded_model.sample(self.mock_tokenizer.tokenize(prompt), n_new_tokens, method="greedy")
        generated_text = self.mock_tokenizer.detokenize(generated_tokens)

        self.assertEqual(len(generated_tokens), n_new_tokens)
        self.assertIsInstance(generated_text, str)
        self.assertGreater(len(generated_text), 0)

if __name__ == '__main__':
    unittest.main()
