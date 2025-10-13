import unittest
import os
import sys
from unittest.mock import patch, MagicMock
import logging
import numpy as np

# Add project root to path to allow importing quanta_tissu
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

# Import common steps
from tests.model.behavior.steps.common_steps import cleanup_test_dir

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
        cleanup_test_dir(self.test_dir)

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
        mock_model_instance.sample.return_value = np.array([self.bpe_tokenizer.encode("generated text")[0]] * 10) # Return numpy array

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

        # Assert that the sample method was called with the correct parameters
        # Using np.testing.assert_array_equal for numpy array comparison
        mock_model_instance.sample.assert_called_once()
        call_args, call_kwargs = mock_model_instance.sample.call_args
        np.testing.assert_array_equal(call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        self.assertEqual(call_kwargs['n_new_tokens'], mock_args.length)
        self.assertEqual(call_kwargs['method'], mock_args.method)
        self.assertEqual(call_kwargs['temperature'], mock_args.temperature)
        self.assertEqual(call_kwargs['top_k'], mock_args.top_k)
        self.assertEqual(call_kwargs['top_p'], mock_args.top_p)
        self.assertEqual(call_kwargs['repetition_penalty'], mock_args.repetition_penalty)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu') # Mock the model
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer') # Mock the Tokenizer to control its loading
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists') # Mock os.path.exists
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
        expected_generated_tokens = np.array([self.bpe_tokenizer.encode("fixed output")[0]] * 5)
        mock_model_instance.sample.return_value = expected_generated_tokens

        mock_args = MagicMock()
        mock_args.prompt = "Start of greedy text"
        mock_args.length = 5
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "greedy"
        mock_args.temperature = 1.0
        mock_args.top_k = 0
        mock_args.top_p = 1.0
        mock_args.repetition_penalty = 1.0
        MockArgumentParser.return_value.parse_args.return_value = mock_args

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
        self.assertIsInstance(generated_output_1, str)
        self.assertGreater(len(generated_output_1.strip()), 0)
        self.assertGreater(len(generated_output_1), len(mock_args.prompt))

        # Assert that sample was called twice with the same arguments
        self.assertEqual(mock_model_instance.sample.call_count, 2)
        # Get the arguments of the first call
        first_call_args, first_call_kwargs = mock_model_instance.sample.call_args_list[0]
        # Get the arguments of the second call
        second_call_args, second_call_kwargs = mock_model_instance.sample.call_args_list[1]

        # Assert that the arguments for both calls are identical
        np.testing.assert_array_equal(first_call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        np.testing.assert_array_equal(second_call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        self.assertEqual(first_call_kwargs, second_call_kwargs) # All keyword args should be identical

        # And verify the parameters of one of the calls (they are identical)
        self.assertEqual(first_call_kwargs['n_new_tokens'], mock_args.length)
        self.assertEqual(first_call_kwargs['method'], mock_args.method)
        self.assertEqual(first_call_kwargs['temperature'], mock_args.temperature)
        self.assertEqual(first_call_kwargs['top_k'], mock_args.top_k)
        self.assertEqual(first_call_kwargs['top_p'], mock_args.top_p)
        self.assertEqual(first_call_kwargs['repetition_penalty'], mock_args.repetition_penalty)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu') # Mock the model
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer') # Mock the Tokenizer to control its loading
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists') # Mock os.path.exists
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
        mock_model_instance.sample.return_value = np.array([self.bpe_tokenizer.encode("topk sample")[0]] * 7)

        mock_args = MagicMock()
        mock_args.prompt = "Top-K test prompt"
        mock_args.length = 7
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "top_k"
        mock_args.temperature = 0.7
        mock_args.top_k = 50
        mock_args.top_p = 1.0
        mock_args.repetition_penalty = 1.0
        MockArgumentParser.return_value.parse_args.return_value = mock_args

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

        self.assertIsInstance(generated_output, str)
        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))

        mock_model_instance.sample.assert_called_once()
        call_args, call_kwargs = mock_model_instance.sample.call_args
        np.testing.assert_array_equal(call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        self.assertEqual(call_kwargs['n_new_tokens'], mock_args.length)
        self.assertEqual(call_kwargs['method'], mock_args.method)
        self.assertEqual(call_kwargs['temperature'], mock_args.temperature)
        self.assertEqual(call_kwargs['top_k'], mock_args.top_k)
        self.assertEqual(call_kwargs['top_p'], mock_args.top_p)
        self.assertEqual(call_kwargs['repetition_penalty'], mock_args.repetition_penalty)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu') # Mock the model
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer') # Mock the Tokenizer to control its loading
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists') # Mock os.path.exists
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
        mock_model_instance.sample.return_value = np.array([self.bpe_tokenizer.encode("nucleus sample")[0]] * 8)

        mock_args = MagicMock()
        mock_args.prompt = "Nucleus test prompt"
        mock_args.length = 8
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "nucleus"
        mock_args.temperature = 0.8
        mock_args.top_k = 0
        mock_args.top_p = 0.9
        mock_args.repetition_penalty = 1.0
        MockArgumentParser.return_value.parse_args.return_value = mock_args

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

        self.assertIsInstance(generated_output, str)
        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))

        mock_model_instance.sample.assert_called_once()
        call_args, call_kwargs = mock_model_instance.sample.call_args
        np.testing.assert_array_equal(call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        self.assertEqual(call_kwargs['n_new_tokens'], mock_args.length)
        self.assertEqual(call_kwargs['method'], mock_args.method)
        self.assertEqual(call_kwargs['temperature'], mock_args.temperature)
        self.assertEqual(call_kwargs['top_k'], mock_args.top_k)
        self.assertEqual(call_kwargs['top_p'], mock_args.top_p)
        self.assertEqual(call_kwargs['repetition_penalty'], mock_args.repetition_penalty)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu') # Mock the model
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer') # Mock the Tokenizer to control its loading
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists') # Mock os.path.exists
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
        mock_model_instance.sample.return_value = np.array([self.bpe_tokenizer.encode("random sample")[0]] * 9)

        mock_args = MagicMock()
        mock_args.prompt = "Random test prompt"
        mock_args.length = 9
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "random"
        mock_args.temperature = 1.0
        mock_args.top_k = 0
        mock_args.top_p = 0.0
        mock_args.repetition_penalty = 1.0
        MockArgumentParser.return_value.parse_args.return_value = mock_args

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

        self.assertIsInstance(generated_output, str)
        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))

        mock_model_instance.sample.assert_called_once()
        call_args, call_kwargs = mock_model_instance.sample.call_args
        np.testing.assert_array_equal(call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        self.assertEqual(call_kwargs['n_new_tokens'], mock_args.length)
        self.assertEqual(call_kwargs['method'], mock_args.method)
        self.assertEqual(call_kwargs['temperature'], mock_args.temperature)
        self.assertEqual(call_kwargs['top_k'], mock_args.top_k)
        self.assertEqual(call_kwargs['top_p'], mock_args.top_p)
        self.assertEqual(call_kwargs['repetition_penalty'], mock_args.repetition_penalty)

    @patch('argparse.ArgumentParser')
    @patch('builtins.print')
    @patch('quanta_tissu.tisslm.core.generate_text.QuantaTissu') # Mock the model
    @patch('quanta_tissu.tisslm.core.generate_text.Tokenizer') # Mock the Tokenizer to control its loading
    @patch('quanta_tissu.tisslm.core.generate_text.os.path.exists') # Mock os.path.exists
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
        mock_model_instance.sample.return_value = np.array([self.bpe_tokenizer.encode("penalty test")[0]] * 5)

        mock_args = MagicMock()
        mock_args.prompt = "Penalty test prompt"
        mock_args.length = 5
        mock_args.checkpoint_path = self.checkpoint_path
        mock_args.method = "greedy" # Can be any method
        mock_args.temperature = 1.0
        mock_args.top_k = 0
        mock_args.top_p = 1.0
        mock_args.repetition_penalty = 1.5
        MockArgumentParser.return_value.parse_args.return_value = mock_args

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

        self.assertIsInstance(generated_output, str)
        self.assertGreater(len(generated_output.strip()), 0)
        self.assertGreater(len(generated_output), len(mock_args.prompt))

        mock_model_instance.sample.assert_called_once()
        call_args, call_kwargs = mock_model_instance.sample.call_args
        np.testing.assert_array_equal(call_args[0], mock_tokenizer_instance.tokenize(mock_args.prompt))
        self.assertEqual(call_kwargs['n_new_tokens'], mock_args.length)
        self.assertEqual(call_kwargs['method'], mock_args.method)
        self.assertEqual(call_kwargs['temperature'], mock_args.temperature)
        self.assertEqual(call_kwargs['top_k'], mock_args.top_k)
        self.assertEqual(call_kwargs['top_p'], mock_args.top_p)
        self.assertEqual(call_kwargs['repetition_penalty'], mock_args.repetition_penalty)

if __name__ == '__main__':
    unittest.main()