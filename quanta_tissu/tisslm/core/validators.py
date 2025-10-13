def validate_output(token_ids, max_ngram_repetition=3, ngram_size=3):
    """
    Validates the generated output to prevent simple failure modes like repetition.

    Args:
        token_ids (list[int]): The list of generated token IDs.
        max_ngram_repetition (int): The maximum number of times an n-gram is allowed to repeat.
        ngram_size (int): The size of the n-gram to check for repetition.

    Returns:
        bool: True if the output is valid, False otherwise.
    """
    if len(token_ids) < ngram_size:
        return True  # Too short to have meaningful repetitions

    ngrams = {}
    for i in range(len(token_ids) - ngram_size + 1):
        ngram = tuple(token_ids[i:i + ngram_size])
        ngrams[ngram] = ngrams.get(ngram, 0) + 1

    for ngram, count in ngrams.items():
        if count > max_ngram_repetition:
            print(f"--- VALIDATION FAILED ---")
            print(f"Reason: The {ngram_size}-gram {list(ngram)} was repeated {count} times (max allowed: {max_ngram_repetition}).")
            print(f"This indicates a potential 'Sleepicture' moment.")
            print(f"-------------------------")
            return False

    return True
