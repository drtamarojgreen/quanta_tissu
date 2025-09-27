import sys
import os
import unittest
import numpy as np
from unittest.mock import patch, MagicMock

from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.layers import softmax

from quanta_tissu.tisslm.core.generation.config import GenerationConfig

class TestModelExtended(unittest.TestCase):

    def setUp(self):
        self.config = {
            "n_embd": 64,
            "vocab_size": 1000,
            "n_head": 4,
            "d_ff": 128,
            "n_layer": 2,
            "dropout_p": 0.1,
            "tie_weights": False,
            "bias": True,
            "use_conv_attention": False,
            "kernel_size": 3
        }
        self.seq_len = 10
        self.vocab_size = self.config["vocab_size"]
        self.model = QuantaTissu(self.config)
        self.dummy_input = np.array([[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]])

    def test_forward_pass(self):
        """Test the model's forward pass."""
        logits, cache, _ = self.model.forward(self.dummy_input)
        self.assertEqual(logits.shape, (1, self.seq_len, self.vocab_size))
        # Further assertions on cache can be added here if needed

    def test_backward_pass(self):
        # Test the model's backward pass.
        logits, cache, _ = self.model.forward(self.dummy_input)
        d_logits = np.ones_like(logits)
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

        self.model.model.forward = MagicMock(side_effect=[(logits, None, None) for logits, _ in mock_logits_sequence])

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

        mock_model_forward_eos = MagicMock(side_effect=[(logits, None, None) for logits, _ in mock_logits_sequence_eos])
        self.model.model.forward = mock_model_forward_eos

        generated_tokens = self.model.sample(prompt_tokens, n_new_tokens, method="greedy", eos_id=eos_id, suppress_eos=True)

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
            (np.zeros((1, len(prompt_tokens), vocab_size)), None, None), # Processing prompt
            (mock_logits, None, None) # Generating the new token
        ])

        # Mock the model's forward pass to return our crafted logits at the last position
        mock_logits_for_prompt = np.zeros((1, len(prompt_tokens), vocab_size))
        mock_logits_for_prompt[0, -1, 42] = 10.0
        mock_logits_for_prompt[0, -1, 55] = 1.0
        self.model.model.forward = MagicMock(return_value=(mock_logits_for_prompt, None, None))

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

        generated_tokens = self.model.sample(prompt_tokens, n_new_tokens)
        self.assertEqual(generated_tokens, [], "Generating with an empty prompt should return an empty list.")

    def test_generate_with_temperature(self):
        # Test that temperature scaling is correctly applied to logits.
        mock_logits = np.array([[[1.0, 2.0, 3.0, 4.0, 5.0]]]) # Example logits
        temperature = 0.5

        # Create a GenerationConfig object for the test
        config = GenerationConfig(method="sampling", temperature=temperature)

        # Mock the model's forward pass to return our dummy logits
        with patch.object(self.model.model, 'forward', return_value=(mock_logits, None, None)) as mock_forward:
            # Call the generator's _predict_from_logits directly for this test
            # We need to pass the config object now
            _, passed_probs = self.model.generator._predict_from_logits(
                mock_logits[0, 0, :], # Pass the 1D logits array
                config=config
            )

            # Expected probabilities with temperature scaling
            expected_logits = mock_logits[0, 0, :] / temperature
            expected_probs = np.exp(expected_logits) / np.sum(np.exp(expected_logits))

            np.testing.assert_allclose(passed_probs, expected_probs, rtol=1e-5)

    def test_moe_architecture(self):
        """Test that MoE is correctly configured when specified in the config."""
        moe_config = {
            "n_embd": 64,
            "vocab_size": 1000,
            "n_head": 4,
            "d_ff": 128,
            "n_layer": 2,
            "dropout_p": 0.1,
            "tie_weights": False,
            "bias": True,
            "use_conv_attention": False,
            "kernel_size": 3,
            "moe": {"num_experts": 4, "top_k": 2}
        }
        model = QuantaTissu(moe_config)
        # Assuming the FFN in TransformerBlock is replaced by MoE when moe_config is present
        # This requires inspecting the internal structure of the model
        from quanta_tissu.tisslm.core.layers import MoE
        self.assertIsInstance(model.model.transformer_blocks[0].ffn, MoE)
        self.assertEqual(model.model.transformer_blocks[0].ffn.num_experts, 4)
        self.assertEqual(model.model.transformer_blocks[0].ffn.top_k, 2)

    def test_shared_input_output_embeddings(self):
        """Test that input and output embeddings are tied when tie_weights is True."""
        config_tied = {
            "n_embd": 64,
            "vocab_size": 1000,
            "n_head": 4,
            "d_ff": 128,
            "n_layer": 2,
            "dropout_p": 0.1,
            "tie_weights": True, # Set tie_weights to True
            "bias": True,
            "use_conv_attention": False,
            "kernel_size": 3
        }
        model_tied = QuantaTissu(config_tied)
        self.assertIs(model_tied.model.embeddings, model_tied.model.output_proj, "Input and output embeddings should be the same object when tied.")

    def test_no_bias_layernorm(self):
        """Test that LayerNorm layers are initialized without bias when bias is False in config."""
        config_no_bias = {
            "n_embd": 64,
            "vocab_size": 1000,
            "n_head": 4,
            "d_ff": 128,
            "n_layer": 2,
            "dropout_p": 0.1,
            "tie_weights": False,
            "bias": False, # Set bias to False
            "use_conv_attention": False,
            "kernel_size": 3
        }
        model_no_bias = QuantaTissu(config_no_bias)
        # Check a LayerNorm instance within a transformer block
        from quanta_tissu.tisslm.core.layers import LayerNorm
        self.assertIsInstance(model_no_bias.model.transformer_blocks[0].ln1, LayerNorm)
        self.assertIsNone(model_no_bias.model.transformer_blocks[0].ln1.beta, "LayerNorm beta should be None when bias is False.")

    def test_depthwise_separable_convolutions(self):
        """Test that ConvTransformerBlock is used when use_conv_attention is True."""
        config_conv_attn = {
            "n_embd": 64,
            "vocab_size": 1000,
            "n_head": 4,
            "d_ff": 128,
            "n_layer": 2,
            "dropout_p": 0.1,
            "tie_weights": False,
            "bias": True,
            "use_conv_attention": True, # Set use_conv_attention to True
            "kernel_size": 3
        }
        model_conv_attn = QuantaTissu(config_conv_attn)
        # Check that the transformer blocks are instances of ConvTransformerBlock
        from quanta_tissu.tisslm.core.architecture.llm import ConvTransformerBlock
        self.assertIsInstance(model_conv_attn.model.transformer_blocks[0], ConvTransformerBlock)

if __name__ == '__main__':
    unittest.main()