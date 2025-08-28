import unittest
import os
import sys
from unittest.mock import MagicMock, patch
import numpy as np

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model_error_handler import ConfigurationError, ModelProcessingError

class TestTextGeneration(unittest.TestCase):

    def setUp(self):
        # Mock the QuantaTissu model and its sample method
        self.mock_model = MagicMock(spec=QuantaTissu)
        # Configure the sample method to return predictable token IDs
        # This mock will be overridden in specific tests if needed
        self.mock_model.sample.return_value = [101, 102, 103, 104, 105] # Example generated token IDs

        # Mock the Tokenizer and its tokenize/detokenize methods
        self.mock_tokenizer = MagicMock(spec=Tokenizer)
        self.mock_tokenizer.tokenize.return_value = np.array([1, 2, 3]) # Example tokenized prompt
        self.mock_tokenizer.detokenize.return_value = " generated text" # Example detokenized text

    def test_greedy_generation(self):
        prompt = "Hello"
        length = 5
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_tokenizer.tokenize.assert_called_once_with(prompt)
        self.mock_model.sample.assert_called_once_with(
            np.array([1, 2, 3]), # Mocked prompt tokens
            n_new_tokens=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=1.0
        )
        self.mock_tokenizer.detokenize.assert_called_once_with(np.array([101, 102, 103, 104, 105]))

    def test_top_k_generation(self):
        prompt = "Test"
        length = 10
        top_k = 50
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="top_k",
            temperature=0.7,
            top_k=top_k,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="top_k",
            temperature=0.7,
            top_k=top_k,
            top_p=None,
            repetition_penalty=1.0
        )

    def test_nucleus_generation(self):
        prompt = "Another test"
        length = 8
        top_p = 0.9
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="nucleus",
            temperature=0.9,
            top_k=None,
            top_p=top_p
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="nucleus",
            temperature=0.9,
            top_k=None,
            top_p=top_p,
            repetition_penalty=1.0
        )

    def test_random_generation(self):
        prompt = "Random prompt"
        length = 3
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="random",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="random",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=1.0
        )

    def test_repetition_penalty(self):
        prompt = "Repeat"
        length = 5
        repetition_penalty = 1.5
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=repetition_penalty
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=repetition_penalty
        )

    def test_empty_prompt(self):
        prompt = ""
        length = 5
        # When prompt is empty, tokenizer.tokenize returns an empty numpy array.
        # generate_text should handle this and return an empty string.
        self.mock_tokenizer.tokenize.return_value = np.array([])

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, "")
        self.mock_tokenizer.tokenize.assert_called_once_with(prompt)
        # model.sample should not be called if prompt is empty
        self.mock_model.sample.assert_not_called()

    def test_zero_length_generation(self):
        prompt = "Hello"
        length = 0
        expected_generated_text = prompt + ""

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_tokenizer.tokenize.assert_called_once_with(prompt)
        # model.sample should not be called if length is 0
        self.mock_model.sample.assert_not_called()
        self.mock_tokenizer.detokenize.assert_not_called()

    def test_eos_id_stops_generation(self):
        prompt = "Start"
        length = 10 # Try to generate 10 tokens
        eos_id = 999 # Define an end-of-sequence ID

        # Configure mock_model.sample to return tokens including the eos_id early
        self.mock_model.sample.return_value = [101, 102, eos_id, 103, 104] # eos_id at 3rd position
        # Configure detokenize to return text that reflects early stopping
        self.mock_tokenizer.detokenize.return_value = " generated text with EOS"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            eos_id=eos_id
        )

        # The detokenized text should reflect that generation stopped at eos_id
        self.assertEqual(result, prompt + " generated text with EOS")
        # Verify that sample was called with eos_id
        self.mock_model.sample.assert_called_once_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=1.0,
            eos_id=eos_id # Ensure eos_id is passed to sample
        )

    def test_invalid_method_raises_error(self):
        prompt = "Invalid"
        length = 5

        with self.assertRaises(ConfigurationError):
            generate_text(
                model=self.mock_model,
                tokenizer=self.mock_tokenizer,
                prompt=prompt,
                length=length,
                method="invalid_method",
                temperature=1.0,
                top_k=None,
                top_p=None
            )

    @patch('quanta_tissu.tisslm.core.generation.generator.Generator._apply_orchestration_policy')
    def test_orchestration_policy_biasing(self, mock_apply_orchestration_policy):
        prompt = "Bias test"
        length = 1
        bias_token_id = 50
        bias_strength = 0.5

        # Mock the initial probabilities returned by softmax
        initial_probs = np.array([0.1, 0.2, 0.05, 0.6, 0.05]) # Example probabilities
        # Ensure the mock returns a modified probability array
        mock_apply_orchestration_policy.side_effect = lambda probs, *args, **kwargs: (
            probs.copy() if kwargs.get('bias_token_id') is None else (
                (probs.copy() / np.sum(probs.copy())) if kwargs.get('bias_token_id') >= len(probs) else (
                    (probs.copy() + (np.eye(len(probs))[kwargs.get('bias_token_id')] * kwargs.get('bias_strength'))) / np.sum(probs.copy() + (np.eye(len(probs))[kwargs.get('bias_token_id')] * kwargs.get('bias_strength')))
                )
            )
        )

        # Mock the model's forward pass to return predictable logits
        self.mock_model.forward.return_value = (np.array([[[0.1, 0.2, 0.05, 0.6, 0.05]]]), None) # Logits corresponding to initial_probs

        # Mock the tokenizer to return a specific token ID after detokenization
        self.mock_tokenizer.detokenize.return_value = " biased token"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            bias_token_id=bias_token_id,
            bias_strength=bias_strength
        )

        # Assert that _apply_orchestration_policy was called with the correct bias parameters
        mock_apply_orchestration_policy.assert_called_once_with(
            np.array([0.1, 0.2, 0.05, 0.6, 0.05]), # Initial probabilities
            "greedy", 1.0, None, None, np.array([1, 2, 3]), 1.0, # Other parameters
            bias_token_id, bias_strength # Bias parameters
        )

        # Assert that the generated text reflects the bias (e.g., the biased token was chosen)
        # This requires the mock_model.sample to return the biased token ID
        # For this test, we'll assume the bias is strong enough to make the biased_token_id the argmax
        # The mock_model.sample.return_value should be set to [bias_token_id] for this assertion to pass
        self.mock_model.sample.return_value = [bias_token_id] # Override default mock for this test
        self.assertEqual(result, prompt + " biased token")

if __name__ == '__main__':
    unittest.main()
