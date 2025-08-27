import sys
import os
import unittest

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.core.model import QuantaTissu

class TestQuantaTissuModel(unittest.TestCase):

    def test_model_initialization(self):
        """
        Tests that the QuantaTissu model can be initialized without errors.
        """
        # A minimal config dictionary required by the model and its components.
        # The actual values may not matter for a simple initialization test,
        # but the keys might. Based on the code, a nested structure is likely.
        # I will create a mock config that is sufficient for initialization.
        config = {
            'vocab_size': 100,
            'd_model': 64,
            'n_layers': 2,
            'n_heads': 2,
            'd_ff': 128,
            'max_seq_len': 256,
            'dropout': 0.1
        }

        try:
            model = QuantaTissu(config)
            self.assertIsNotNone(model, "Model should not be None after initialization.")
            self.assertIsNotNone(model.model, "Core model architecture should be initialized.")
            self.assertIsNotNone(model.generator, "Generator should be initialized.")
        except Exception as e:
            self.fail(f"QuantaTissu model initialization failed with an exception: {e}")

if __name__ == '__main__':
    unittest.main()
