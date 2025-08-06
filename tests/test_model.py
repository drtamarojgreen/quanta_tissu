import sys
import os
import numpy as np

# This is a common pattern to make sure the test can find the source code
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.quanta_tissu.model import (
    TransformerBlock,
    PositionalEncoding,
    QuantaTissu,
)
from quanta_tissu.quanta_tissu.config import model_config
from tests.test_utils import assert_equal, assert_true

# Set a seed for reproducibility of random inputs
np.random.seed(42)

def test_positional_encoding():
    """Tests the PositionalEncoding class."""
    d_model = 16
    seq_len = 10
    pos_encoder = PositionalEncoding(d_model)
    x = np.zeros((seq_len, d_model))
    encoded_x = pos_encoder(x)

    assert_equal(encoded_x.shape, x.shape, "PositionalEncoding should not change shape")
    # The encoding for the first element should not be all zeros
    assert_true(np.any(encoded_x[0] != 0), "First position should have non-zero encoding")
    # Different positions should have different encodings
    assert_true(np.any(encoded_x[0] != encoded_x[1]), "Different positions should have different encodings")


def test_transformer_block():
    """Tests the TransformerBlock."""
    d_model = 32
    num_heads = 4
    d_ff = 128
    seq_len = 10

    block = TransformerBlock(d_model, num_heads, d_ff)
    x = np.random.randn(seq_len, d_model)
    output = block(x)

    assert_equal(output.shape, x.shape, "TransformerBlock output shape should match input shape")


def test_quanta_tissu_forward_pass():
    """Tests the forward pass of the QuantaTissu model."""
    # Use the model_config from the project's config file
    model = QuantaTissu(model_config)

    seq_len = 5
    # Create a random sequence of token IDs within the vocab size
    token_ids = np.random.randint(0, model_config["vocab_size"], size=seq_len)

    logits = model.forward(token_ids)

    expected_shape = (seq_len, model_config["vocab_size"])
    assert_equal(logits.shape, expected_shape, f"Logits shape should be {expected_shape}")


def test_quanta_tissu_predict():
    """Tests the predict method of the QuantaTissu model."""
    model = QuantaTissu(model_config)

    seq_len = 5
    token_ids = np.random.randint(0, model_config["vocab_size"], size=seq_len)

    # Test greedy prediction
    next_token_id = model.predict(token_ids, method="greedy")

    assert_true(isinstance(next_token_id, (int, np.integer)), "Predicted token should be an integer ID")
    assert_true(0 <= next_token_id < model_config["vocab_size"], "Predicted token ID should be within vocab size")

    # Test that other methods also return a single token
    next_token_id_random = model.predict(token_ids, method="random")
    assert_true(isinstance(next_token_id_random, (int, np.integer)), "Predicted token (random) should be an integer ID")
