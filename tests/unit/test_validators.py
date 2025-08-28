# It's good practice to import the specific function or class you're testing.
from quanta_tissu.tisslm.core.validators import validate_output

def test_validate_output_no_repetition():
    """
    Tests that a sequence with no n-gram repetitions passes validation.
    """
    token_ids = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    assert validate_output(token_ids) is True, "Should pass with no repetitions"

def test_validate_output_acceptable_repetition():
    """
    Tests that a sequence with an acceptable number of n-gram repetitions passes validation.
    """
    # The n-gram (1, 2, 3) is repeated 3 times, which is the default max allowed.
    token_ids = [1, 2, 3, 4, 5, 1, 2, 3, 6, 7, 1, 2, 3, 8, 9]
    assert validate_output(token_ids, max_ngram_repetition=3, ngram_size=3) is True, "Should pass with max allowed repetitions"

def test_validate_output_excessive_repetition():
    """
    Tests that a sequence with excessive n-gram repetitions fails validation.
    """
    # The n-gram (1, 2, 3) is repeated 4 times, which is more than the default max of 3.
    token_ids = [1, 2, 3, 4, 5, 1, 2, 3, 6, 7, 1, 2, 3, 8, 9, 1, 2, 3]
    assert validate_output(token_ids, max_ngram_repetition=3, ngram_size=3) is False, "Should fail with excessive repetitions"

def test_validate_output_short_sequence():
    """
    Tests that a sequence shorter than the n-gram size passes validation.
    """
    token_ids = [1, 2]
    assert validate_output(token_ids, ngram_size=3) is True, "Should pass if sequence is too short to have n-grams"

def test_validate_output_empty_sequence():
    """
    Tests that an empty sequence passes validation.
    """
    token_ids = []
    assert validate_output(token_ids) is True, "Should pass for an empty sequence"

def test_validate_output_custom_ngram_size():
    """
    Tests validation with a custom n-gram size.
    """
    # The 2-gram (8, 8) is repeated 4 times.
    token_ids = [1, 2, 8, 8, 3, 4, 8, 8, 5, 6, 8, 8, 7, 8, 8]
    # Should fail with max_repetition = 3
    assert validate_output(token_ids, max_ngram_repetition=3, ngram_size=2) is False, "Should fail with custom n-gram and excessive repetitions"
    # Should pass with max_repetition = 4
    assert validate_output(token_ids, max_ngram_repetition=4, ngram_size=2) is True, "Should pass with custom n-gram and allowed repetitions"
