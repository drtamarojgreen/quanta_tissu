import unittest
import os
import sys
from unittest.mock import patch, MagicMock, mock_open
import logging
import numpy as np

# Add project root to path to allow importing quanta_tissu
# Assuming the test file is at /project_root/tests/model/behavior/features/
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

# Import common steps
from tests.model.behavior.steps.common_steps import cleanup_test_dir

# Mock system_config before importing train_bpe to avoid FileNotFoundError
with patch('quanta_tissu.tisslm.core.train_bpe.system_config') as mock_train_bpe_system_config:
    mock_train_bpe_system_config._project_root = project_root
    mock_train_bpe_system_config.bpe_tokenizer_prefix = os.path.join(project_root, "models", "test_trained_tokenizer")
    from quanta_tissu.tisslm.core.train_bpe import main as train_bpe_main
    from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer

class TestModelTrainingBehavior(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_bdd_training")
        self.corpus_dir = os.path.join(self.test_dir, "corpus")
        self.output_dir = os.path.join(self.test_dir, "output")
        os.makedirs(self.corpus_dir, exist_ok=True)
        os.makedirs(self.output_dir, exist_ok=True)

        self.corpus_file_path = os.path.join(self.corpus_dir, "sample_corpus.txt")
        with open(self.corpus_file_path, "w", encoding="utf-8") as f:
            f.write("This is a sample text for training the BPE tokenizer. It contains some repeated words.")
            f.write("Another line of text to make the corpus a bit larger.")

        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        cleanup_test_dir(self.test_dir)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    def test_given_corpus_when_trained_then_artifacts_saved(self, mock_print, MockArgumentParser):
        """
        Scenario: Successful training and artifact generation
        Given: A clean environment and a valid text corpus.
        When: The tisslm model training pipeline is executed with specified vocabulary size and save path.
        Then: A trained model's vocabulary and merge files are created at the specified save path.
        And: The created vocabulary and merge files are not empty.
        """
        mock_args = MagicMock()
        mock_args.corpus_path = self.corpus_dir
        mock_args.vocab_size = 500
        mock_args.save_prefix = os.path.join(self.output_dir, "bdd_tokenizer")
        mock_args.verbose = False
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        train_bpe_main()

        vocab_file = f"{mock_args.save_prefix}_vocab.json"
        merges_file = f"{mock_args.save_prefix}_merges.txt"

        self.assertTrue(os.path.exists(vocab_file), f"Vocabulary file not found: {vocab_file}")
        self.assertTrue(os.path.exists(merges_file), f"Merges file not found: {merges_file}")
        self.assertGreater(os.path.getsize(vocab_file), 0, f"Vocabulary file is empty: {vocab_file}")
        self.assertGreater(os.path.getsize(merges_file), 0, f"Merges file is empty: {merges_file}")

class TestModelPersistenceBehavior(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_bdd_persistence")
        self.output_dir = os.path.join(self.test_dir, "output")
        os.makedirs(self.output_dir, exist_ok=True)
        self.save_prefix = os.path.join(self.output_dir, "bdd_persistent_tokenizer")

        self.original_tokenizer = BPETokenizer()
        self.original_tokenizer.vocab = {0: b'a', 1: b'b', 256: b'ab'}
        self.original_tokenizer.merges = {(0, 1): 256}
        self.original_tokenizer.reverse_vocab = {v: k for k, v in self.original_tokenizer.vocab.items()}

        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        cleanup_test_dir(self.test_dir)

    def test_given_trained_model_when_saved_and_loaded_then_identical(self):
        """
        Scenario: Saving and loading a trained model
        Given: A trained tisslm model instance.
        When: The model is saved to a specified location.
        And: The model is then loaded from the same location into a new instance.
        Then: The loaded model's vocabulary and merges are identical to the original trained model's.
        """
        self.original_tokenizer.save(self.save_prefix)

        loaded_tokenizer = BPETokenizer()
        loaded_tokenizer.load(self.save_prefix)

        self.assertEqual(self.original_tokenizer.vocab, loaded_tokenizer.vocab)
        self.assertEqual(self.original_tokenizer.merges, loaded_tokenizer.merges)

class TestTokenizerIntegrationBehavior(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_bdd_tokenizer_integration")
        self.output_dir = os.path.join(self.test_dir, "output")
        os.makedirs(self.output_dir, exist_ok=True)
        self.save_prefix = os.path.join(self.output_dir, "bdd_integrated_tokenizer")

        self.tokenizer = BPETokenizer()
        text_for_training = "hello world this is a test string for tokenizer integration"
        self.tokenizer.train(text_for_training, vocab_size=300)

        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        cleanup_test_dir(self.test_dir)

    def test_given_text_when_encoded_and_decoded_then_identical(self):
        """
        Scenario: Encoding and decoding text using the integrated tokenizer
        Given: A trained tisslm model with its integrated tokenizer. (Implicitly, the tokenizer is part of the model's ecosystem)
        And: A sample text string.
        When: The sample text is encoded into token IDs.
        And: The token IDs are then decoded back into text.
        Then: The decoded text is identical to the original sample text.
        """
        sample_text = "This is a sample text to encode and decode."
        encoded_ids = self.tokenizer.encode(sample_text)
        decoded_text = self.tokenizer.decode(encoded_ids)
        self.assertEqual(sample_text, decoded_text)
