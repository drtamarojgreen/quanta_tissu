import sys
import os
import unittest
from unittest.mock import patch, MagicMock
import numpy as np

# Add project root to path to allow importing quanta_tissu
# This structure assumes the test is run from the project root or 'tests' directory
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.model import QuantaTissu, TissSystemError
from quanta_tissu.tisslm.core.system_error_handler import DatabaseConnectionError

class TestQuantaTissuModel(unittest.TestCase):

    def setUp(self):
        """Set up a standard configuration for the model before each test."""
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

    def test_model_initialization(self):
        """
        Tests that the QuantaTissu model can be initialized without errors.
        """
        self.assertIsNotNone(self.model, "Model should not be None after initialization.")
        self.assertIsNotNone(self.model.model, "Core model architecture should be initialized.")
        self.assertIsNotNone(self.model.generator, "Generator should be initialized.")

    @patch('os.path.exists')
    @patch('numpy.load')
    def test_load_weights_success(self, mock_np_load, mock_path_exists):
        """Tests successful weight loading from a standard checkpoint."""
        mock_path_exists.return_value = True

        # Create mock weights that match the model's parameter names and shapes
        mock_weights = {p.name: np.random.rand(*p.value.shape).astype(np.float32) for p in self.model.parameters()}
        mock_np_load.return_value = mock_weights

        self.model.load_weights('dummy_path.npz')

        # Verify that numpy.load was called with the correct path
        mock_np_load.assert_called_once_with('dummy_path.npz', allow_pickle=True)

        # Check if the model's parameters have been updated
        for param in self.model.parameters():
            self.assertTrue(np.array_equal(param.value, mock_weights[param.name]))

    @patch('os.path.exists')
    def test_load_weights_file_not_found(self, mock_path_exists):
        """Tests that loading weights from a non-existent file is handled gracefully."""
        mock_path_exists.return_value = False

        # This should not raise an error, but log a warning
        self.model.load_weights('non_existent_path.npz')
        mock_path_exists.assert_called_once_with('non_existent_path.npz')

    @patch('os.path.exists')
    @patch('numpy.load')
    def test_load_weights_shape_mismatch(self, mock_np_load, mock_path_exists):
        """Tests that shape mismatches during weight loading are handled gracefully."""
        mock_path_exists.return_value = True

        # Create mock weights with a shape mismatch for one parameter
        mock_weights = {p.name: np.random.rand(*p.value.shape).astype(np.float32) for p in self.model.parameters()}

        # Introduce a shape mismatch
        param_to_mismatch = self.model.parameters()[0]
        mismatched_shape = list(param_to_mismatch.value.shape)
        mismatched_shape[0] += 1 # Make it different
        mock_weights[param_to_mismatch.name] = np.random.rand(*mismatched_shape).astype(np.float32)

        mock_np_load.return_value = mock_weights

        # This should not raise an error but should log a warning
        self.model.load_weights('dummy_path.npz')

        # The mismatched parameter should not be updated
        self.assertFalse(np.array_equal(self.model.parameters()[0].value, mock_weights[param_to_mismatch.name]))

    def test_forward_pass(self):
        """Tests the forward pass with a sample input."""
        batch_size = 2
        seq_len = 10
        token_ids = np.random.randint(0, self.config['vocab_size'], size=(batch_size, seq_len))

        logits, _, _ = self.model.forward(token_ids)

        expected_shape = (batch_size, seq_len, self.config['vocab_size'])
        self.assertEqual(logits.shape, expected_shape, "Output shape of the forward pass is incorrect.")

    def test_sample_generation(self):
        """Tests the text generation (sample) method."""
        prompt_tokens = [1, 2, 3]
        n_new_tokens = 5

        generated_tokens = self.model.sample(prompt_tokens, n_new_tokens)

        self.assertEqual(len(generated_tokens), n_new_tokens, "Number of generated tokens is incorrect.")

    @patch('quanta_tissu.tisslm.core.model.KnowledgeBase')
    def test_db_integration_success(self, MockKnowledgeBase):
        """Tests that the KnowledgeBase is initialized when use_db is True."""
        QuantaTissu(self.config, use_db=True)
        MockKnowledgeBase.assert_called_once()

    @patch('quanta_tissu.tisslm.core.model.KnowledgeBase', side_effect=DatabaseConnectionError("Connection failed"))
    def test_db_integration_failure(self, MockKnowledgeBase):
        """Tests that a TissSystemError is raised if the DB connection fails."""
        with self.assertRaises(TissSystemError):
            QuantaTissu(self.config, use_db=True)

    def test_model_initialization_missing_config(self):
        """
        Tests that model initialization fails with a KeyError if a required key is missing.
        """
        config = {
            'vocab_size': 100,
            # 'n_embd' is missing
            'n_layer': 2,
            'n_head': 2,
            'd_ff': 128,
            'max_seq_len': 256,
            'dropout': 0.1
        }
        with self.assertRaises(KeyError):
            QuantaTissu(config)

    def test_model_initialization_invalid_value(self):
        """
        Tests that model initialization fails with a ValueError for invalid config values.
        """
        config = {
            'vocab_size': -100, # Invalid value
            'n_embd': 64,
            'n_layer': 2,
            'n_head': 2,
            'd_ff': 128,
            'max_seq_len': 256,
            'dropout': 0.1
        }
        # This will likely raise a ValueError from numpy when trying to create an array with a negative dimension.
        with self.assertRaises(ValueError):
            QuantaTissu(config)

    @patch('os.path.exists')
    @patch('numpy.load')
    def test_load_weights_legacy_format(self, mock_np_load, mock_path_exists):
        """Tests successful weight loading from a legacy checkpoint format."""
        mock_path_exists.return_value = True

        # Create mock weights in legacy format
        mock_legacy_weights = {
            'param_0': np.random.rand(64, 100).astype(np.float32), # Example legacy param
            'param_1': np.random.rand(100, 64).astype(np.float32)
        }
        mock_np_load.return_value = mock_legacy_weights

        # Temporarily adjust model config to match legacy param count if needed
        original_n_layer = self.config['n_layer']
        self.config['n_layer'] = 1 # Simplify for this test
        model_legacy = QuantaTissu(self.config)
        self.config['n_layer'] = original_n_layer # Restore

        model_legacy.load_weights('legacy_path.npz')

        # Verify that numpy.load was called with the correct path
        mock_np_load.assert_called_once_with('legacy_path.npz', allow_pickle=True)

        # Basic check: ensure some parameters were updated
        # Verify that the values of the model's parameters have changed from their initial random state
        initial_param_values = {p.name: p.value.copy() for p in model_legacy.parameters()}
        model_legacy.load_weights('legacy_path.npz')

        # Verify that numpy.load was called with the correct path
        mock_np_load.assert_called_once_with('legacy_path.npz', allow_pickle=True)

        # Check if at least one parameter's value has changed
        updated_count = 0
        for i, param in enumerate(model_legacy.parameters()):
            if i < len(param_keys) and param.value.shape == mock_legacy_weights[param_keys[i]].shape:
                if not np.array_equal(param.value, initial_param_values[param.name]):
                    updated_count += 1
        self.assertGreater(updated_count, 0, "No parameters were updated from legacy weights.")

if __name__ == '__main__':
    unittest.main()
