import sys
import os
import unittest
import numpy as np

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.core.model import QuantaTissu

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

    def test_generate(self):
        """Test the model's generate method."""
        # TODO: Implement the generate test
        pass

if __name__ == '__main__':
    unittest.main()
