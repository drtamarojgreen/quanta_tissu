import unittest
import os
import sys
from unittest.mock import patch, MagicMock, mock_open
import logging

# Add project root to path to allow importing quanta_tissu
# Assuming the test file is at /project_root/tests/model/behavior/
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
sys.path.insert(0, project_root)

# Mock system_config before importing train_bpe to avoid FileNotFoundError
# This mock needs to be outside the class or in a module-level setup
# to ensure it's active when train_bpe is imported.
with patch('quanta_tissu.tisslm.core.train_bpe.system_config') as mock_system_config:
    mock_system_config._project_root = project_root
    mock_system_config.bpe_tokenizer_prefix = os.path.join(project_root, "models", "test_trained_tokenizer")
    from quanta_tissu.tisslm.core.train_bpe import main as train_bpe_main
    from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer # Import BPETokenizer for persistence test

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

        # Suppress logging during tests
        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        if os.path.exists(self.test_dir):
            # Manually remove directory and its contents
            for root, dirs, files in os.walk(self.test_dir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
            os.rmdir(self.test_dir)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print') # Mock print to avoid console output during test
    def test_given_corpus_when_trained_then_artifacts_saved(self, mock_print, MockArgumentParser):
        """
        Scenario: Successful training and artifact generation
        Given: A clean environment and a valid text corpus.
        When: The tisslm model training pipeline is executed with specified vocabulary size and save path.
        Then: A trained model's vocabulary and merge files are created at the specified save path.
        And: The created vocabulary and merge files are not empty.
        """
        # Given: A valid text corpus (setup in setUp)
        # And: Mock ArgumentParser to control inputs to train_bpe_main
        mock_args = MagicMock()
        mock_args.corpus_path = self.corpus_dir
        mock_args.vocab_size = 500 # A reasonable vocab size for a small corpus
        mock_args.save_prefix = os.path.join(self.output_dir, "bdd_tokenizer")
        mock_args.verbose = False
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # When: The tisslm model training pipeline is executed
        train_bpe_main()

        # Then: A trained model's vocabulary and merge files are created at the specified save path.
        vocab_file = f"{mock_args.save_prefix}_vocab.json"
        merges_file = f"{mock_args.save_prefix}_merges.txt"

        self.assertTrue(os.path.exists(vocab_file), f"Vocabulary file not found: {vocab_file}")
        self.assertTrue(os.path.exists(merges_file), f"Merges file not found: {merges_file}")

        # And: The created vocabulary and merge files are not empty.
        self.assertGreater(os.path.getsize(vocab_file), 0, f"Vocabulary file is empty: {vocab_file}")
        self.assertGreater(os.path.getsize(merges_file), 0, f"Merges file is empty: {merges_file}")

class TestModelPersistenceBehavior(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_bdd_persistence")
        self.output_dir = os.path.join(self.test_dir, "output")
        os.makedirs(self.output_dir, exist_ok=True)
        self.save_prefix = os.path.join(self.output_dir, "bdd_persistent_tokenizer")

        # Create a dummy tokenizer for testing persistence
        self.original_tokenizer = BPETokenizer()
        self.original_tokenizer.vocab = {0: b'a', 1: b'b', 256: b'ab'}
        self.original_tokenizer.merges = {(0, 1): 256}
        self.original_tokenizer.reverse_vocab = {v: k for k, v in self.original_tokenizer.vocab.items()}

        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        if os.path.exists(self.test_dir):
            for root, dirs, files in os.walk(self.test_dir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
            os.rmdir(self.test_dir)

    def test_given_trained_model_when_saved_and_loaded_then_identical(self):
        """
        Scenario: Saving and loading a trained model
        Given: A trained tisslm model instance.
        When: The model is saved to a specified location.
        And: The model is then loaded from the same location into a new instance.
        Then: The loaded model's vocabulary and merges are identical to the original trained model's.
        """
        # Given: A trained tisslm model instance (setup in setUp)

        # When: The model is saved to a specified location.
        self.original_tokenizer.save(self.save_prefix)

        # And: The model is then loaded from the same location into a new instance.
        loaded_tokenizer = BPETokenizer()
        loaded_tokenizer.load(self.save_prefix)

        # Then: The loaded model's vocabulary and merges are identical to the original trained model's.
        self.assertEqual(self.original_tokenizer.vocab, loaded_tokenizer.vocab)
        self.assertEqual(self.original_tokenizer.merges, loaded_tokenizer.merges)

class TestTokenizerIntegrationBehavior(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_bdd_tokenizer_integration")
        self.output_dir = os.path.join(self.test_dir, "output")
        os.makedirs(self.output_dir, exist_ok=True)
        self.save_prefix = os.path.join(self.output_dir, "bdd_integrated_tokenizer")

        # Train a simple tokenizer for integration test
        self.tokenizer = BPETokenizer()
        text_for_training = "hello world this is a test string for tokenizer integration"
        self.tokenizer.train(text_for_training, vocab_size=300) # Small vocab for simple test

        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        if os.path.exists(self.test_dir):
            for root, dirs, files in os.walk(self.test_dir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
            os.rmdir(self.test_dir)

    def test_given_text_when_encoded_and_decoded_then_identical(self):
        """
        Scenario: Encoding and decoding text using the integrated tokenizer
        Given: A trained tisslm model with its integrated tokenizer. (Implicitly, the tokenizer is part of the model's ecosystem)
        And: A sample text string.
        When: The sample text is encoded into token IDs.
        And: The token IDs are then decoded back into text.
        Then: The decoded text is identical to the original sample text.
        """
        # Given: A trained tokenizer (setup in setUp)
        sample_text = "This is a sample text to encode and decode."

        # When: The sample text is encoded into token IDs.
        encoded_ids = self.tokenizer.encode(sample_text)

        # And: The token IDs are then decoded back into text.
        decoded_text = self.tokenizer.decode(encoded_ids)

        # Then: The decoded text is identical to the original sample text.
        self.assertEqual(sample_text, decoded_text)

# Note: Text Generation (Inference) test will be more complex as it requires a full model,
# not just a tokenizer. This will be implemented in a separate file or after
# understanding the model's generation interface.

if __name__ == '__main__':
    unittest.main()
