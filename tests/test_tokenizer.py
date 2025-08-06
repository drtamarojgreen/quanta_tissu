import sys
import os
import numpy as np

# This is a common pattern to make sure the test can find the source code
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quanta_tissu.quanta_tissu.tokenizer import tokenize, detokenize
from quanta_tissu.quanta_tissu.config import vocab
from tests.test_utils import assert_equal, assert_allclose

def test_tokenize_simple():
    """Tests tokenization of a simple, known sentence."""
    text = "hello world ."
    expected_ids = np.array([vocab["hello"], vocab["world"], vocab["."]])
    token_ids = tokenize(text)
    assert_allclose(token_ids, expected_ids, msg="test_tokenize_simple")

def test_tokenize_unknown_words():
    """Tests that unknown words are mapped to the <unk> token."""
    text = "hello foobar test"
    expected_ids = np.array([vocab["hello"], vocab["<unk>"], vocab["test"]])
    token_ids = tokenize(text)
    assert_allclose(token_ids, expected_ids, msg="test_tokenize_unknown_words")

def test_detokenize_simple():
    """Tests detokenization of a simple sequence of IDs."""
    token_ids = np.array([vocab["this"], vocab["is"], vocab["a"], vocab["test"]])
    expected_text = "this is a test"
    text = detokenize(token_ids)
    assert_equal(text, expected_text, msg="test_detokenize_simple")

def test_tokenize_empty_string():
    """Tests tokenization of an empty string."""
    text = ""
    expected_ids = np.array([])
    token_ids = tokenize(text)
    assert_allclose(token_ids, expected_ids, msg="test_tokenize_empty_string")

def test_detokenize_empty_sequence():
    """Tests detokenization of an empty sequence."""
    token_ids = np.array([])
    expected_text = ""
    text = detokenize(token_ids)
    assert_equal(text, expected_text, msg="test_detokenize_empty_sequence")
