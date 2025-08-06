import numpy as np
import pytest
from quanta_tissu.quanta_tissu.tokenizer import tokenize, detokenize
from quanta_tissu.quanta_tissu.layers import softmax, LayerNorm, scaled_dot_product_attention, MultiHeadAttention, FeedForward
from quanta_tissu.quanta_tissu.model import TransformerBlock, PositionalEncoding, QuantaTissu
from quanta_tissu.quanta_tissu.config import vocab

# 1.1 Tokenizer
def test_tokenize():
    assert np.array_equal(tokenize("hello world"), np.array([vocab["hello"], vocab["world"]]))

def test_tokenize_unknown_words():
    assert np.array_equal(tokenize("hello foobar"), np.array([vocab["hello"], vocab["<unk>"]]))

def test_detokenize():
    assert detokenize(np.array([vocab["hello"], vocab["world"]])) == "hello world"

# 1.2 Core Components
def test_softmax():
    x = np.array([1.0, 2.0, 3.0])
    expected = np.array([0.09003057, 0.24472847, 0.66524096])
    assert np.allclose(softmax(x), expected, atol=1e-8)

def test_layer_norm():
    d_model = 4
    ln = LayerNorm(d_model)
    x = np.random.randn(10, d_model)
    output = ln(x)
    assert output.shape == (10, d_model)
    # Check that the mean is close to 0 and std is close to 1
    assert np.allclose(output.mean(), 0, atol=1e-7)
    assert np.allclose(output.std(), 1, atol=1e-7)

def test_scaled_dot_product_attention():
    Q = np.random.randn(1, 2, 4, 8)
    K = np.random.randn(1, 2, 4, 8)
    V = np.random.randn(1, 2, 4, 8)
    output = scaled_dot_product_attention(Q, K, V)
    assert output.shape == (1, 2, 4, 8)

def test_multi_head_attention():
    d_model = 16
    num_heads = 4
    mha = MultiHeadAttention(d_model, num_heads)
    x = np.random.randn(10, d_model)
    output = mha(x)
    assert output.shape == (10, d_model)

def test_feed_forward():
    d_model = 16
    d_ff = 32
    ff = FeedForward(d_model, d_ff)
    x = np.random.randn(10, d_model)
    output = ff(x)
    assert output.shape == (10, d_model)

def test_positional_encoding():
    d_model = 16
    pe = PositionalEncoding(d_model)
    x = np.zeros((10, d_model))
    output = pe(x)
    assert output.shape == (10, d_model)
    # Check that positional encoding is not zero
    assert not np.allclose(output, np.zeros((10, d_model)))

# 1.3 Transformer Block
def test_transformer_block():
    d_model = 16
    num_heads = 4
    d_ff = 32
    block = TransformerBlock(d_model, num_heads, d_ff)
    x = np.random.randn(10, d_model)
    output = block(x)
    assert output.shape == (10, d_model)

# 1.4 QuantaTissu Model
@pytest.fixture
def model_config():
    return {
        "d_model": 16,
        "vocab_size": len(vocab),
        "num_heads": 4,
        "d_ff": 32,
        "n_layers": 2,
    }

def test_forward_pass(model_config):
    model = QuantaTissu(model_config)
    token_ids = np.array([1, 2, 3])
    logits = model.forward(token_ids)
    assert logits.shape == (3, model_config["vocab_size"])

def test_predict(model_config):
    model = QuantaTissu(model_config)
    token_ids = np.array([1, 2, 3])
    next_token = model.predict(token_ids)
    assert isinstance(next_token, np.integer)
