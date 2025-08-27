import sys
import os
import numpy as np

# This is a common pattern to make sure the test can find the source code
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.tisslm.core.layers import (
    softmax,
    LayerNorm,
    scaled_dot_product_attention,
    MultiHeadAttention,
    FeedForward,
)
from tests.test_utils import assert_allclose, assert_equal, assert_raises

# Set a seed for reproducibility of random inputs
np.random.seed(42)

def test_softmax():
    """Tests the softmax function."""
    x = np.array([1.0, 2.0, 3.0])
    probs = softmax(x)
    assert_allclose(np.sum(probs), 1.0, msg="Softmax probabilities should sum to 1")
    expected_probs = np.array([0.09003057, 0.24472847, 0.66524096])
    assert_allclose(probs, expected_probs, msg="Softmax computed incorrectly")

def test_softmax_with_temperature():
    """Tests the softmax function with temperature scaling."""
    x = np.array([1.0, 2.0, 3.0])
    # High temperature should make probabilities more uniform
    probs_high_temp = softmax(x, temperature=100.0)
    assert np.std(probs_high_temp) < 0.01, "High temperature should produce near-uniform probabilities"
    # Low temperature should make probabilities more peaky
    probs_low_temp = softmax(x, temperature=0.1)
    assert_allclose(probs_low_temp, np.array([2.06115362e-09, 4.53999298e-05, 9.99954600e-01]), msg="Low temperature softmax incorrect")

def test_softmax_invalid_temperature():
    """Tests that softmax raises an error for invalid temperatures."""
    x = np.array([1.0, 2.0, 3.0])
    assert_raises(ValueError, softmax, x, temperature=0)
    assert_raises(ValueError, softmax, x, temperature=-1.0)


def test_layer_norm():
    """Tests the LayerNorm layer."""
    d_model = 16
    batch_size = 1
    seq_len = 10
    ln = LayerNorm(d_model)
    x = np.random.randn(batch_size, seq_len, d_model) # (batch_size, seq_len, d_model)
    normed_x, _ = ln(x)
    assert_equal(x.shape, normed_x.shape, "LayerNorm should not change shape")
    # The mean of the output should be close to 0
    assert_allclose(np.mean(normed_x, axis=-1), np.zeros(x.shape[:-1]), atol=1e-7, msg="Mean of LayerNorm output should be 0")
    # The variance of the output should be close to 1
    assert_allclose(np.var(normed_x, axis=-1), np.ones(x.shape[:-1]), atol=1e-7, msg="Variance of LayerNorm output should be 1")

def test_layer_norm_zero_variance():
    """Tests LayerNorm with input that has zero variance."""
    d_model = 16
    batch_size = 1
    seq_len = 10
    ln = LayerNorm(d_model)
    # Create an input where all vectors are identical
    x = np.ones((batch_size, seq_len, d_model))
    normed_x, _ = ln(x)
    # The output should be all zeros, and not contain NaNs or Infs
    assert_allclose(normed_x, np.zeros_like(x), msg="LayerNorm with zero variance should output zeros")


def test_scaled_dot_product_attention():
    """Tests the scaled dot product attention function."""
    batch_size, num_heads, seq_len, d_k = 1, 2, 5, 8
    q = np.random.randn(batch_size, num_heads, seq_len, d_k)
    k = np.random.randn(batch_size, num_heads, seq_len, d_k)
    v = np.random.randn(batch_size, num_heads, seq_len, d_k)
    output, _ = scaled_dot_product_attention(q, k, v)
    assert_equal(output.shape, v.shape, "Attention output shape should match value shape")


def test_multi_head_attention():
    """Tests the MultiHeadAttention layer."""
    d_model = 32
    num_heads = 4
    batch_size = 1
    seq_len = 10
    mha = MultiHeadAttention(d_model, num_heads)
    x = np.random.randn(batch_size, seq_len, d_model)
    output, _ = mha(x)
    assert_equal(output.shape, x.shape, "MHA output shape should match input shape")

def test_multi_head_attention_constructor_fail():
    """Tests that the MHA constructor fails with non-divisible num_heads."""
    d_model = 32
    num_heads = 5  # 32 is not divisible by 5
    assert_raises(AssertionError, MultiHeadAttention, d_model, num_heads)


def test_feed_forward():
    """Tests the FeedForward layer."""
    d_model = 32
    d_ff = 64
    batch_size = 1
    seq_len = 10
    ffn = FeedForward(d_model, d_ff)
    x = np.random.randn(batch_size, seq_len, d_model)
    output, _ = ffn(x)
    assert_equal(output.shape, x.shape, "FeedForward output shape should match input shape")

if __name__ == "__main__":
    test_softmax()
    test_softmax_with_temperature()
    test_softmax_invalid_temperature()
    test_layer_norm()
    test_layer_norm_zero_variance()
    test_scaled_dot_product_attention()
    test_multi_head_attention()
    test_multi_head_attention_constructor_fail()
    test_feed_forward()
    print("All layer tests passed!")