import os
import sys

# Explicitly add the directory containing the *inner* 'quanta_tissu' package to sys.path
# This script is at c:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\temp_tokenizer_test.py
# The directory containing the 'quanta_tissu' package (which contains tisslm) is c:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu
sys.path.insert(0, "c:\\Users\\tamar\\Documents\\DataAnnotation\\Gemini\\quanta_tissu")

try:
    from quanta_tissu.tisslm.core.tokenizer import Tokenizer
    import numpy as np

    # The tokenizer path is now correctly identified
    tokenizer_path = os.path.join("c:\\Users\\tamar\\Documents\\DataAnnotation\\Gemini\\quanta_tissu", "tokenizer", "trained_tokenizer")

    tokenizer = Tokenizer(tokenizer_path=tokenizer_path)

    test_string = "The meaning of life is"
    encoded_tokens = tokenizer.tokenize(test_string) # Corrected method call
    decoded_string = tokenizer.detokenize(encoded_tokens) # Corrected method call

    print(f"Original string: '{test_string}'")
    print(f"Encoded tokens: {encoded_tokens}")
    print(f"Decoded string: '{decoded_string}'")

    # Test with a string that has the problematic words
    test_problem_string = "cohesive enric soothing ortex ect"
    encoded_problem_tokens = tokenizer.tokenize(test_problem_string) # Corrected method call
    decoded_problem_string = tokenizer.detokenize(encoded_problem_tokens) # Corrected method call

    print(f"Original problem string: '{test_problem_string}'")
    print(f"Encoded problem tokens: {encoded_problem_tokens}")
    print(f"Decoded problem string: '{decoded_problem_string}'")

except FileNotFoundError as e:
    print(f"Error: Tokenizer files not found. Please ensure 'tokenizer/trained_tokenizer' exists. {e}")
except Exception as e:
    print(f"An unexpected error occurred: {e}")
    print(f"sys.path: {sys.path}")
    print(f"Current working directory: {os.getcwd()}")
