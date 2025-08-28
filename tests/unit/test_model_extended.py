import sys
import os
import unittest
import numpy as np
from numpy.testing import assert_allclose
from unittest.mock import MagicMock

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.layers import softmax

class TestModelExtended(unittest.TestCase):

    def setUp(self):
        """Set up a model and some dummy data for the tests."""
        self.config = {
            'vocab_size': 100,
            'n_embd': 64,
            'n_layer': 2,
            'n_head': 2,
            'd_ff': 128,
            'max_seq_len': 256,
            'dropout': 0.1
        }
        self.model = QuantaTissu(self.config)
        self.dummy_input = np.random.randint(0, self.config['vocab_size'], size=(1, 10))

    def test_forward_pass(self):
        """Test the model's forward pass."""
        logits, cache = self.model.forward(self.dummy_input)

        self.assertEqual(logits.shape, (1, 10, self.config['vocab_size']), "Logits shape is incorrect.")
        self.assertFalse(np.all(logits == 0), "Logits should not be all zeros.")

    def test_backward_pass(self):
        """Test the model's backward pass."""
        logits, cache = self.model.forward(self.dummy_input)

        # Create a dummy gradient for the logits
        d_logits = np.random.randn(*logits.shape)

        # Perform backward pass
        self.model.backward(d_logits, cache)

        # Check that gradients are not all zeros for some parameters
        # This is a basic check, more rigorous checks would involve numerical gradient checking
        all_params_zero = True
        for param in self.model.parameters():
            if param.grad is not None and np.any(param.grad != 0):
                all_params_zero = False
                break
        self.assertFalse(all_params_zero, "Gradients should not be all zeros after backward pass.")

    def test_generate_greedy(self):
        """Test the model's sample method with greedy decoding."""
        prompt_tokens = [1, 2, 3]
        n_new_tokens = 5
        vocab_size = self.config['vocab_size']

        mock_logits_sequence = []

        # First call: for initial prompt processing. Logits for the last token of the prompt.
        logits_prompt_processing = np.zeros((1, len(prompt_tokens), vocab_size))
        logits_prompt_processing[0, -1, 2] = 1.0 # This will make the first generated token be 2
        mock_logits_sequence.append((logits_prompt_processing, None))

        # Subsequent calls: for each new token generated.
        # Logits for the second generated token (after generating 2)
        logits_step1 = np.zeros((1, 1, vocab_size))
        logits_step1[0, 0, 1] = 1.0 # Predicts token 1
        mock_logits_sequence.append((logits_step1, None))

        # Logits for the third generated token (after generating 1)
        logits_step2 = np.zeros((1, 1, vocab_size))
        logits_step2[0, 0, 3] = 1.0 # Predicts token 3
        mock_logits_sequence.append((logits_step2, None))

        # Logits for the fourth generated token (after generating 3)
        logits_step3 = np.zeros((1, 1, vocab_size))
        logits_step3[0, 0, 4] = 1.0 # Predicts token 4
        mock_logits_sequence.append((logits_step3, None))

        # Logits for the fifth generated token (after generating 4)
        logits_step4 = np.zeros((1, 1, vocab_size))
        logits_step4[0, 0, 0] = 1.0 # Predicts token 0
        mock_logits_sequence.append((logits_step4, None))

        # Logits for the sixth call (after generating 0). This call is made, but its output is not used.
        # It's just to prevent StopIteration.
        logits_step5 = np.zeros((1, 1, vocab_size))
        logits_step5[0, 0, 0] = 1.0 # Dummy value
        mock_logits_sequence.append((logits_step5, None))

        self.model.model.forward = MagicMock(side_effect=mock_logits_sequence)

        generated_tokens = self.model.sample(prompt_tokens, n_new_tokens, method="greedy")

        expected_tokens = [2, 1, 3, 4, 0] # Based on the mock_logits_sequence

        self.assertEqual(generated_tokens, expected_tokens, "Generated tokens do not match expected sequence.")
        self.assertEqual(len(generated_tokens), n_new_tokens, "Incorrect number of tokens generated.")

    def test_generate_eos_id(self):
        """Test the model's sample method with an end-of-sequence ID."""
        prompt_tokens = [10, 20]
        n_new_tokens = 10 # Try to generate up to 10 new tokens
        eos_id = 99
        vocab_size = self.config['vocab_size']

        mock_logits_sequence_eos = []

        # First call: for initial prompt processing. Logits for the last token of the prompt.
        logits_eos_prompt_processing = np.zeros((1, len(prompt_tokens), vocab_size))
        logits_eos_prompt_processing[0, -1, 2] = 1.0 # This will make the first generated token be 2
        mock_logits_sequence_eos.append((logits_eos_prompt_processing, None))

        # Second call: for the first new token (which is 2). Logits for the next token, which should be eos_id.
        logits_eos_step1 = np.zeros((1, 1, vocab_size))
        logits_eos_step1[0, 0, eos_id] = 1.0 # Predicts eos_id (99)
        mock_logits_sequence_eos.append((logits_eos_step1, None))

        mock_model_forward_eos = MagicMock(side_effect=mock_logits_sequence_eos)
        self.model.model.forward = mock_model_forward_eos

        generated_tokens = self.model.sample(prompt_tokens, n_new_tokens, method="greedy", eos_id=eos_id)

        expected_tokens = [2] # Only token 2 should be generated before eos_id

        self.assertEqual(generated_tokens, expected_tokens, "Generated tokens should stop at eos_id.")
        self.assertLess(len(generated_tokens), n_new_tokens, "Generation should have stopped before n_new_tokens.")

    def test_generate_topp(self):
        """Test the model's sample method with nucleus sampling (top-p)."""
        prompt_tokens = [5, 15, 25]
        n_new_tokens = 1
        vocab_size = self.config['vocab_size']
        top_p = 0.9

        # Create logits where one token (index 42) has a very high probability (e.g., > 0.9)
        # and other tokens have low probabilities.
        # With top_p = 0.9, only the token with the highest probability should be in the nucleus.
        mock_logits = np.zeros((1, 1, vocab_size))
        mock_logits[0, 0, 42] = 10.0  # High logit for token 42, giving it a high probability after softmax
        mock_logits[0, 0, 55] = 1.0   # Lower logit for another token

        # Mock the forward pass for the prompt and for the new token generation
        self.model.model.forward = MagicMock(side_effect=[
            (np.zeros((1, len(prompt_tokens), vocab_size)), None), # Processing prompt
            (mock_logits, None) # Generating the new token
        ])

        # Mock the model's forward pass to return our crafted logits at the last position
        mock_logits_for_prompt = np.zeros((1, len(prompt_tokens), vocab_size))
        mock_logits_for_prompt[0, -1, 42] = 10.0
        mock_logits_for_prompt[0, -1, 55] = 1.0
        self.model.model.forward = MagicMock(return_value=(mock_logits_for_prompt, None))

        # Even with a nucleus of one, the implementation uses np.random.choice, so we mock it.
        with unittest.mock.patch('numpy.random.choice', return_value=42) as mock_choice:
            generated_tokens = self.model.sample(prompt_tokens, n_new_tokens, method="nucleus", top_p=top_p)

            # Verify that the probabilities passed to choice were correct
            self.assertTrue(mock_choice.called)
            args, kwargs = mock_choice.call_args
            passed_probs = kwargs['p']
            # The nucleus should contain only token 42
            self.assertAlmostEqual(passed_probs[42], 1.0)
            self.assertAlmostEqual(np.sum(passed_probs), 1.0)


        expected_token = [42]
        self.assertEqual(generated_tokens, expected_token, "Generated token with top-p sampling is incorrect.")

    def test_generate_with_empty_prompt(self):
        """Tests how the model handles an empty prompt."""
        prompt_tokens = []
        n_new_tokens = 5

        # Depending on the implementation, this could raise an error or return an empty list.
        # Based on the code, it's likely to raise an IndexError when accessing logits.
        # A robust implementation might handle this, but we test the current behavior.
        with self.assertRaises(IndexError):
            self.model.sample(prompt_tokens, n_new_tokens)

    def test_generate_with_temperature(self):
        """Test that temperature scaling is correctly applied to logits."""
        prompt_tokens = [1, 2]
        n_new_tokens = 1
        vocab_size = self.config['vocab_size']
        temperature = 0.5

        # Logits that will be scaled by temperature.
        # Let's use simple values to make the calculation clear.
        mock_logits = np.zeros((1, 1, vocab_size))
        mock_logits[0, 0, 10] = 2.0
        mock_logits[0, 0, 20] = 1.0

        # The core of this test is to verify that the probabilities passed to the sampling function
        # are correctly scaled by temperature.
        # We will directly call _predict_from_logits to get the probabilities.
        
        # The _predict_from_logits method expects a 1D logits array.
        # mock_logits is (1, 1, vocab_size), so we need to squeeze it.
        _, passed_probs = self.model.generator._predict_from_logits(
            mock_logits[0, 0, :], # Pass the 1D logits array
            method="sampling",
            temperature=temperature
        )

        # Calculate the expected probabilities
        # Create a full logits array for calculation
        full_expected_logits = np.zeros(vocab_size)
        full_expected_logits[10] = 2.0
        full_expected_logits[20] = 1.0

        # scaled_logits = original_logits / temperature
        scaled_logits = full_expected_logits / temperature

        # Manually calculate softmax over the full vocabulary
        exp_scaled_logits = np.exp(scaled_logits - np.max(scaled_logits))
        expected_full_probs = exp_scaled_logits / np.sum(exp_scaled_logits)

        # We only care about the probabilities of the tokens we set logits for.
        # The rest should be close to zero.
        self.assertAlmostEqual(passed_probs[10], expected_full_probs[10], places=5)
        self.assertAlmostEqual(passed_probs[20], expected_full_probs[20], places=5)
        self.assertAlmostEqual(np.sum(passed_probs), 1.0, places=5)


if __name__ == '__main__':
    unittest.main()