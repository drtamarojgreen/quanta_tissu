import sys
import os
import pytest

def main():
    """
    Runs the BDD test suite for the language model using pytest.
    """
    # The tests are in the same directory as this runner.
    test_dir = os.path.dirname(os.path.abspath(__file__))

    print("--- Running Language Model BDD Test Suite ---")
    sys.stdout.flush()

    # Pytest arguments.
    # -v for verbose output
    # We point pytest to the directory containing the tests.
    args = [
        "-v",
        test_dir,
    ]

    # Run pytest.
    # The exit code from pytest will be used as the exit code of this script.
    exit_code = pytest.main(args)

    print("--- Language Model BDD Test Suite Finished ---")
    sys.exit(exit_code)

if __name__ == "__main__":
    main()
