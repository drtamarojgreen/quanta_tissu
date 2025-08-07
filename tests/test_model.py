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
from tests.test_utils import assert_equal, assert_true, assert_raises

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


def test_positional_encoding_long_sequence():
    """Tests that positional encoding fails for sequences longer than max_len."""
    d_model = 16
    max_len = 50
    pos_encoder = PositionalEncoding(d_model, max_len=max_len)

    # This sequence is too long
    x_long = np.zeros((max_len + 1, d_model))

    # Expect a ValueError because the slice self.pe[:seq_len] will be out of bounds
    assert_raises(ValueError, pos_encoder, x_long)


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


def test_quanta_tissu_generate_sequence():
    """Tests generating a sequence of tokens."""
    model = QuantaTissu(model_config)

    # Start with the token for "hello"
    initial_ids = np.array([model_config["vocab_size"] - 4]) # A known token from the config
    num_to_generate = 5

    generated_ids = list(initial_ids)
    for _ in range(num_to_generate):
        next_token_id = model.predict(np.array(generated_ids))
        generated_ids.append(next_token_id)

    # The total length should be the initial length plus the number generated
    expected_length = len(initial_ids) + num_to_generate
    assert_equal(len(generated_ids), expected_length, "Generated sequence has incorrect length")

def test_quanta_tissu_predict_top_k():
    """Tests the top-k sampling method of the predict function."""
    model = QuantaTissu(model_config)
    token_ids = np.random.randint(0, model_config["vocab_size"], size=5)
    k = 5

    # Get the logits to find the actual top k tokens
    logits = model.forward(token_ids)
    top_k_true_indices = np.argsort(logits[-1])[-k:]

    # Run top-k prediction multiple times to increase chance of catching errors
    for _ in range(10):
        next_token_id = model.predict(token_ids, method="top_k", top_k=k)
        assert_true(isinstance(next_token_id, (int, np.integer)), "Top-k should return an integer ID")
        assert_true(next_token_id in top_k_true_indices, "Token from top-k must be in the true top-k set")

def test_quanta_tissu_predict_nucleus():
    """Tests the nucleus sampling method of the predict function."""
    model = QuantaTissu(model_config)
    token_ids = np.random.randint(0, model_config["vocab_size"], size=5)
    p = 0.9

    # Run nucleus prediction multiple times
    for _ in range(10):
        next_token_id = model.predict(token_ids, method="nucleus", top_p=p)
        assert_true(isinstance(next_token_id, (int, np.integer)), "Nucleus sampling should return an integer ID")
        # It's hard to verify correctness without re-implementing the logic,
        # so we mainly test that it runs and returns a valid token ID.
        assert_true(0 <= next_token_id < model_config["vocab_size"], "Token ID from nucleus must be in vocab range")


def test_predict_raises_for_missing_args():
    """Tests that predict raises ValueError for missing arguments."""
    model = QuantaTissu(model_config)
    token_ids = np.array([1, 2, 3])

    # Test top_k
    assert_raises(ValueError, model.predict, token_ids, method="top_k")
    assert_raises(ValueError, model.predict, token_ids, method="top_k", top_k=None)

    # Test nucleus
    assert_raises(ValueError, model.predict, token_ids, method="nucleus")
    assert_raises(ValueError, model.predict, token_ids, method="nucleus", top_p=None)

    # Test unknown method
    assert_raises(ValueError, model.predict, token_ids, method="unknown_method")
