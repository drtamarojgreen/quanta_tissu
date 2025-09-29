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
from quanta_tissu.tisslm.core.model_error_handler import ConfigurationError

class TestTextGeneration(unittest.TestCase):

    def setUp(self):
        self.mock_model = MagicMock(spec=QuantaTissu)
        self.mock_model.sample.return_value = [101, 102, 103, 104, 105]
        self.mock_tokenizer = MagicMock(spec=Tokenizer)
        self.mock_tokenizer.tokenize.return_value = np.array([1, 2, 3])
        self.mock_tokenizer.detokenize.return_value = " generated text"

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

        self.mock_model.sample.assert_called_once()
        call_args, call_kwargs = self.mock_model.sample.call_args
        np.testing.assert_array_equal(call_args[0], np.array([1, 2, 3]))
        self.assertEqual(call_kwargs['n_new_tokens'], length)
        self.assertEqual(call_kwargs['method'], "greedy")

        self.mock_tokenizer.detokenize.assert_called_once()
        np.testing.assert_array_equal(self.mock_tokenizer.detokenize.call_args[0][0], np.array([101, 102, 103, 104, 105]))


    def test_top_k_generation(self):
        prompt = "Test"
        length = 10
        top_k = 50

        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="top_k",
            temperature=0.7,
            top_k=top_k,
            top_p=None
        )

        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['top_k'], top_k)


    def test_nucleus_generation(self):
        prompt = "Another test"
        length = 8
        top_p = 0.9

        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="nucleus",
            temperature=0.9,
            top_k=None,
            top_p=top_p
        )

        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['top_p'], top_p)


    def test_random_generation(self):
        prompt = "Random prompt"
        length = 3

        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="random",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['method'], "random")


    def test_repetition_penalty(self):
        prompt = "Repeat"
        length = 5
        repetition_penalty = 1.5

        generate_text(
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

        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['repetition_penalty'], repetition_penalty)


    def test_empty_prompt(self):
        prompt = ""
        length = 5
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
        self.mock_model.sample.assert_not_called()

    def test_zero_length_generation(self):
        prompt = "Hello"
        length = 0
        self.mock_model.sample.return_value = []
        self.mock_tokenizer.detokenize.return_value = ""

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

        self.assertEqual(result, prompt)
        self.mock_model.sample.assert_called_once()
        call_args, call_kwargs = self.mock_model.sample.call_args
        np.testing.assert_array_equal(call_args[0], np.array([1, 2, 3]))
        self.assertEqual(call_kwargs['n_new_tokens'], 0)

        self.mock_tokenizer.detokenize.assert_called_once()
        detokenize_args, _ = self.mock_tokenizer.detokenize.call_args
        np.testing.assert_array_equal(detokenize_args[0], np.array([]))


    def test_invalid_method_raises_error(self):
        prompt = "Invalid"
        length = 5
        self.mock_model.sample.side_effect = ConfigurationError("Invalid method")

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

    def test_beam_search_generation(self):
        prompt = "Beam"
        length = 5
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="beam_search",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['method'], "beam_search")

    def test_contrastive_search_generation(self):
        prompt = "Contrastive"
        length = 5
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="contrastive_search",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['method'], "contrastive_search")

    def test_mirostat_sampling_generation(self):
        prompt = "Mirostat"
        length = 5
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="mirostat_sampling",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['method'], "mirostat_sampling")

    def test_top_a_sampling_generation(self):
        prompt = "Top-A"
        length = 5
        expected_generated_text = prompt + " generated text"

        result = generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="top_a",
            temperature=1.0,
            top_k=None,
            top_p=None
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['method'], "top_a")

    def test_bias_token_id_and_strength(self):
        prompt = "Bias"
        length = 5
        bias_token_id = 100
        bias_strength = 0.5
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
            bias_token_id=bias_token_id,
            bias_strength=bias_strength
        )

        self.assertEqual(result, expected_generated_text)
        self.mock_model.sample.assert_called_once()
        _, call_kwargs = self.mock_model.sample.call_args
        self.assertEqual(call_kwargs['bias_token_id'], bias_token_id)
        self.assertEqual(call_kwargs['bias_strength'], bias_strength)

    def test_repetition_penalty_edge_cases(self):
        prompt = "Edge Case Repetition"
        length = 5

        # Test with repetition_penalty = 0 (should effectively prevent any repetition)
        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=0.0
        )
        self.mock_model.sample.assert_called_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=0.0,
            bias_token_id=None,
            bias_strength=0.0
        )
        self.mock_model.sample.reset_mock()

        # Test with a very high repetition_penalty (should strongly discourage repetition)
        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=100.0
        )
        self.mock_model.sample.assert_called_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="greedy",
            temperature=1.0,
            top_k=None,
            top_p=None,
            repetition_penalty=100.0,
            bias_token_id=None,
            bias_strength=0.0
        )
        self.mock_model.sample.reset_mock()

        # Test with temperature = 0 (should be equivalent to greedy)
        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=0.0
        )
        self.mock_model.sample.assert_called_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="greedy",
            temperature=0.0,
            top_k=None,
            top_p=None,
            repetition_penalty=1.0,
            bias_token_id=None,
            bias_strength=0.0
        )
        self.mock_model.sample.reset_mock()

        # Test with a very high temperature (should make probabilities more uniform)
        generate_text(
            model=self.mock_model,
            tokenizer=self.mock_tokenizer,
            prompt=prompt,
            length=length,
            method="greedy",
            temperature=100.0
        )
        self.mock_model.sample.assert_called_with(
            np.array([1, 2, 3]),
            n_new_tokens=length,
            method="greedy",
            temperature=100.0,
            top_k=None,
            top_p=None,
            repetition_penalty=1.0,
            bias_token_id=None,
            bias_strength=0.0
        )

if __name__ == '__main__':
    unittest.main()
