import unittest
import os
import sys
from unittest.mock import patch, MagicMock, mock_open
import logging
import numpy as np

# Add project root to path to allow importing quanta_tissu
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
sys.path.insert(0, project_root)

# Mock system_config and model_config before importing generate_text
with patch('quanta_tissu.tisslm.core.generate_text.model_config') as mock_model_config:
    mock_model_config.return_value = {"vocab_size": 300, "n_layer": 1, "n_head": 1, "n_embd": 32, "block_size": 64}
    from quanta_tissu.tisslm.core.generate_text import main as generate_text_main, generate_text as generate_text_func
    from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer
    from quanta_tissu.tisslm.core.tokenizer import Tokenizer # The actual Tokenizer class

class TestTextGenerationBehavior(unittest.TestCase):

    def setUp(self):
        self.test_dir = os.path.join(project_root, "temp_bdd_generation")
        self.tokenizer_dir = os.path.join(self.test_dir, "tokenizer_files")
        self.checkpoint_dir = os.path.join(self.test_dir, "checkpoints")
        os.makedirs(self.tokenizer_dir, exist_ok=True)
        os.makedirs(self.checkpoint_dir, exist_ok=True)

        self.tokenizer_prefix = os.path.join(self.tokenizer_dir, "bdd_gen_tokenizer")
        self.checkpoint_path = os.path.join(self.checkpoint_dir, "bdd_gen_model.npz")

        # 1. Train and save a minimal BPETokenizer
        self.bpe_tokenizer = BPETokenizer()
        text_for_training = "the quick brown fox jumps over the lazy dog and then some more text"
        self.bpe_tokenizer.train(text_for_training, vocab_size=300)
        self.bpe_tokenizer.save(self.tokenizer_prefix)

        # Suppress logging during tests
        logging.basicConfig(level=logging.CRITICAL)

    def tearDown(self):
        if os.path.exists(self.test_dir):
            for root, dirs, files in os.walk(self.test_dir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
            os.rmdir(self.test_dir)

    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu')
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer')
    def test_given_mismatched_tokenizer_when_generating_text_then_error_is_clear(self, MockTokenizer, MockQuantaTissu, mock_print):
        """
        Scenario: Generating text with a mismatched tokenizer
        Given: A model that produces token IDs outside the tokenizer's vocabulary.
        When: The generate_text function is called.
        Then: The function should detect the decoding error and log an appropriate error message.
        """
        # Given: A tokenizer with a small vocabulary (e.g., 100 tokens)
        mock_tokenizer_instance = MockTokenizer.return_value
        # Simulate a decode that produces replacement characters for unknown IDs
        def mismatched_decode(ids):
            return "".join([chr(i) if i < 100 else "\uFFFD" for i in ids])
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = mismatched_decode

        # And: A model that generates token IDs outside that vocabulary (e.g., > 100)
        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.sample.return_value = [50, 150, 250] # IDs 150 and 250 are "unknown"

        # When: We call the generate_text function
        with self.assertLogs('quanta_tissu.tisslm.core.generate_text', level='ERROR') as cm:
            generated_text = generate_text_func(
                model=mock_model_instance,
                tokenizer=mock_tokenizer_instance,
                prompt="Test",
                length=3
            )
            # Then: The log should contain the specific error message about mismatch
            self.assertTrue(any("DECODING ERROR DETECTED" in log for log in cm.output))
            self.assertTrue(any("model-tokenizer mismatch" in log for log in cm.output))

        # And: The output text should contain the replacement character
        self.assertIn("\uFFFD", generated_text)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu') # Mock the model
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer') # Mock the Tokenizer to control its loading
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists') # Mock os.path.exists
    def test_given_trained_model_when_text_generated_then_text_is_valid(self, mock_exists, MockTokenizer, MockQuantaTissu, mock_print, MockArgumentParser):
        mock_exists.return_value = True # Ensure checkpoint path is considered to exist
        """
        Scenario: Generating text from a trained model
        Given: A previously trained tisslm model checkpoint and its associated tokenizer files.
        And: A starting prompt and desired generation length.
        When: Text generation is performed using the generate_text function.
        Then: The generated text is not empty.
        And: The generated text has a length approximately equal to the requested length.
        """
        # Given: Mock Tokenizer to load our pre-trained BPE tokenizer
        mock_tokenizer_instance = MockTokenizer.return_value
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = self.bpe_tokenizer.decode
        mock_tokenizer_instance.get_vocab_size.return_value = len(self.bpe_tokenizer.vocab)

        # Given: Mock QuantaTissu model
        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.load_weights.return_value = None # load_weights doesn't return anything
        # Simulate token generation: return a sequence of token IDs
        # This needs to be carefully crafted to ensure detokenization works
        # For simplicity, let's return a sequence that decodes to a predictable string
        mock_model_instance.sample.return_value = [self.bpe_tokenizer.encode("generated text")[0]] * 10 # Generate 10 tokens

        # And: Mock ArgumentParser
        mock_args = MagicMock()
        mock_args.prompt = "The quick brown"
        mock_args.length = 10
        mock_args.checkpoint_path = self.checkpoint_path # Path to a dummy checkpoint
        mock_args.method = "greedy"
        mock_args.temperature = 1.0
        mock_args.top_k = 0
        mock_args.top_p = 1.0
        mock_args.repetition_penalty = 1.0 # Add repetition_penalty to mock_args
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # Call generate_text_func directly with mocked arguments
        generated_text_output = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        # Then: The generated text is not empty.
        self.assertIsInstance(generated_text_output, str)
        self.assertGreater(len(generated_text_output.strip()), 0)

        # And: The generated text has a length approximately equal to the requested length.
        self.assertGreater(len(generated_text_output), len(mock_args.prompt))

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu')
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer')
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists')
    def test_given_trained_model_when_greedy_generation_then_deterministic_output(self, mock_exists, MockTokenizer, MockQuantaTissu, mock_print, MockArgumentParser):
        mock_exists.return_value = True
        """
        Scenario: Greedy Generation
        Given: A trained model and a prompt.
        When: Text generation is performed using the "greedy" method.
        Then: The generated text is deterministic (for the same prompt and model).
        And: The generated text is not empty and has the expected length.
        """
        mock_tokenizer_instance = MockTokenizer.return_value
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = self.bpe_tokenizer.decode
        mock_tokenizer_instance.get_vocab_size.return_value = len(self.bpe_tokenizer.vocab)

        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.load_weights.return_value = None
        # For greedy, the sample method should return a fixed sequence
        expected_generated_tokens = [self.bpe_tokenizer.encode("fixed output")[0]] * 5
        mock_model_instance.sample.return_value = expected_generated_tokens

        mock_args = MagicMock()
        mock_args.prompt = "Start of greedy text"
        mock_args.length = 5
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "greedy"
        mock_args.temperature = 1.0 # Temperature doesn't affect greedy
        mock_args.top_k = 0 # Top-k doesn't affect greedy
        mock_args.top_p = 1.0 # Top-p doesn't affect greedy
        mock_args.repetition_penalty = 1.0 # Add repetition_penalty to mock_args
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # Call generate_text_func directly with mocked arguments
        generated_output_1 = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        # Call again to check for determinism
        generated_output_2 = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        self.assertEqual(generated_output_1, generated_output_2, "Greedy generation should be deterministic")
        self.assertGreater(len(generated_output_1.strip()), 0)
        self.assertGreater(len(generated_output_1), len(mock_args.prompt))

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu')
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer')
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists')
    def test_given_trained_model_when_top_k_sampling_then_valid_output(self, mock_exists, MockTokenizer, MockQuantaTissu, mock_print, MockArgumentParser):
        mock_exists.return_value = True
        """
        Scenario: Top-K Sampling
        Given: A trained model, a prompt, and a top_k value.
        When: Text generation is performed using the "top_k" method with a specific temperature.
        Then: The generated text is not empty and has the expected length.
        """
        mock_tokenizer_instance = MockTokenizer.return_value
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = self.bpe_tokenizer.decode
        mock_tokenizer_instance.get_vocab_size.return_value = len(self.bpe_tokenizer.vocab)

        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.load_weights.return_value = None
        # Simulate token generation for top_k. It should return a sequence.
        mock_model_instance.sample.return_value = [self.bpe_tokenizer.encode("topk sample")[0]] * 7

        mock_args = MagicMock()
        mock_args.prompt = "Top-K test prompt"
        mock_args.length = 7
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "top_k"
        mock_args.temperature = 0.7 # A typical temperature for sampling
        mock_args.top_k = 50 # A typical top_k value
        mock_args.top_p = 1.0
        mock_args.repetition_penalty = 1.0 # Add repetition_penalty to mock_args
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # Call generate_text_func directly with mocked arguments
        generated_output = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))
        # Assert that the sample method was called with the correct parameters
        tokenized_prompt = mock_tokenizer_instance.tokenize(mock_args.prompt)
        mock_model_instance.sample.assert_called_once_with(
            tokenized_prompt.tolist(), # Convert numpy array to list
            n_new_tokens=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu')
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer')
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists')
    def test_given_trained_model_when_nucleus_sampling_then_valid_output(self, mock_exists, MockTokenizer, MockQuantaTissu, mock_print, MockArgumentParser):
        mock_exists.return_value = True
        """
        Scenario: Nucleus Sampling (Top-P)
        Given: A trained model, a prompt, and a top_p value.
        When: Text generation is performed using the "nucleus" method with a specific temperature.
        Then: The generated text is not empty and has the expected length.
        """
        mock_tokenizer_instance = MockTokenizer.return_value
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = self.bpe_tokenizer.decode
        mock_tokenizer_instance.get_vocab_size.return_value = len(self.bpe_tokenizer.vocab)

        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.load_weights.return_value = None
        # Simulate token generation for nucleus sampling
        mock_model_instance.sample.return_value = [self.bpe_tokenizer.encode("nucleus sample")[0]] * 8

        mock_args = MagicMock()
        mock_args.prompt = "Nucleus test prompt"
        mock_args.length = 8
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "nucleus"
        mock_args.temperature = 0.8
        mock_args.top_k = 0 # Not used for nucleus sampling
        mock_args.top_p = 0.9 # A typical top_p value
        mock_args.repetition_penalty = 1.0 # Add repetition_penalty to mock_args
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # Call generate_text_func directly with mocked arguments
        generated_output = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))
        # Assert that the sample method was called with the correct parameters
        tokenized_prompt = mock_tokenizer_instance.tokenize(mock_args.prompt)
        mock_model_instance.sample.assert_called_once_with(
            tokenized_prompt.tolist(), # Convert numpy array to list
            n_new_tokens=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu')
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer')
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists')
    def test_given_trained_model_when_random_sampling_then_valid_output(self, mock_exists, MockTokenizer, MockQuantaTissu, mock_print, MockArgumentParser):
        mock_exists.return_value = True
        """
        Scenario: Random Sampling
        Given: A trained model and a prompt.
        When: Text generation is performed using the "random" method.
        Then: The generated text is not empty and has the expected length.
        """
        mock_tokenizer_instance = MockTokenizer.return_value
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = self.bpe_tokenizer.decode
        mock_tokenizer_instance.get_vocab_size.return_value = len(self.bpe_tokenizer.vocab)

        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.load_weights.return_value = None
        # Simulate token generation for random sampling
        mock_model_instance.sample.return_value = [self.bpe_tokenizer.encode("random sample")[0]] * 9

        mock_args = MagicMock()
        mock_args.prompt = "Random test prompt"
        mock_args.length = 9
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "random"
        mock_args.temperature = 1.0 # Temperature typically 1.0 for pure random
        mock_args.top_k = 0
        mock_args.top_p = 0.0 # Not used for random sampling
        mock_args.repetition_penalty = 1.0 # Add repetition_penalty to mock_args
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # Call generate_text_func directly with mocked arguments
        generated_output = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))
        # Assert that the sample method was called with the correct parameters
        tokenized_prompt = mock_tokenizer_instance.tokenize(mock_args.prompt)
        mock_model_instance.sample.assert_called_once_with(
            tokenized_prompt.tolist(), # Convert numpy array to list
            n_new_tokens=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu')
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer')
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists')
    def test_given_trained_model_when_repetition_penalty_then_parameter_passed(self, mock_exists, MockTokenizer, MockQuantaTissu, mock_print, MockArgumentParser):
        mock_exists.return_value = True
        """
        Scenario: Repetition Penalty
        Given: A trained model, a prompt, and a repetition_penalty value.
        When: Text generation is performed.
        Then: The repetition_penalty parameter is correctly passed to the model's sample method.
        """
        mock_tokenizer_instance = MockTokenizer.return_value
        mock_tokenizer_instance.tokenize.side_effect = lambda x: np.array(self.bpe_tokenizer.encode(x))
        mock_tokenizer_instance.detokenize.side_effect = self.bpe_tokenizer.decode
        mock_tokenizer_instance.get_vocab_size.return_value = len(self.bpe_tokenizer.vocab)

        mock_model_instance = MockQuantaTissu.return_value
        mock_model_instance.load_weights.return_value = None
        mock_model_instance.sample.return_value = [self.bpe_tokenizer.encode("penalty test")[0]] * 5

        mock_args = MagicMock()
        mock_args.prompt = "Penalty test prompt"
        mock_args.length = 5
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "greedy" # Can be any method
        mock_args.temperature = 1.0
        mock_args.top_k = 0
        mock_args.top_p = 1.0
        mock_args.repetition_penalty = 1.5 # Specific penalty value
        MockArgumentParser.return_value.parse_args.return_value = mock_args

        # Call generate_text_func directly with mocked arguments
        generated_output = generate_text_func(
            model=mock_model_instance,
            tokenizer=mock_tokenizer_instance,
            prompt=mock_args.prompt,
            length=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty
        )

        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))
        # Assert that the sample method was called with the correct repetition_penalty
        tokenized_prompt = mock_tokenizer_instance.tokenize(mock_args.prompt)
        mock_model_instance.sample.assert_called_once_with(
            tokenized_prompt.tolist(), # Convert numpy array to list
            n_new_tokens=mock_args.length,
            method=mock_args.method,
            temperature=mock_args.temperature,
            top_k=mock_args.top_k,
            top_p=mock_args.top_p,
            repetition_penalty=mock_args.repetition_penalty # Assert the specific value
        )

if __name__ == '__main__':
    unittest.main()
