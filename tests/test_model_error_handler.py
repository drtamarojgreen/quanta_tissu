import logging
import io
from quanta_tissu.tisslm.core.model_error_handler import (
    handle_model_errors,
    TissModelError,
    ModelProcessingError,
    InferenceError,
    ConfigurationError,
)

# --- Test Setup ---

# Configure a logger to capture log output for verification
log_capture_string = io.StringIO()
# We target the logger used in the module we are testing
logger = logging.getLogger('quanta_tissu.tisslm.core.model_error_handler')
# Ensure we capture ERROR level logs
logger.setLevel(logging.ERROR)
# Create a handler that writes to our in-memory string buffer
handler = logging.StreamHandler(log_capture_string)
logger.addHandler(handler)


# --- Test Functions for the Decorator ---

@handle_model_errors
def function_that_succeeds():
    """A simple function that should execute without errors."""
    return "success"

@handle_model_errors
def function_that_raises_tiss_error():
    """A function that raises a specific, expected type of error."""
    raise InferenceError("A specific inference error occurred.")

@handle_model_errors
def function_that_raises_unexpected_error():
    """A function that raises a generic error that should be wrapped."""
    raise ValueError("An unexpected value error.")


def test_decorator_success_path():
    """Tests that the decorator returns the function's result when no error occurs."""
    assert function_that_succeeds() == "success"

def test_decorator_catches_and_reraises_tiss_model_error():
    """Tests that the decorator catches a TissModelError and re-raises it."""
    try:
        function_that_raises_tiss_error()
        # If this line is reached, the test fails because no exception was raised.
        assert False, "Expected InferenceError to be raised"
    except InferenceError as e:
        # Check that the correct exception was raised.
        assert "A specific inference error occurred." in str(e)
    except Exception:
        # Fail if any other exception type is caught.
        assert False, "Expected InferenceError, but a different exception was raised"

def test_decorator_wraps_unexpected_error():
    """Tests that the decorator wraps a generic exception in ModelProcessingError."""
    try:
        function_that_raises_unexpected_error()
        assert False, "Expected ModelProcessingError to be raised"
    except ModelProcessingError as e:
        assert "An unexpected error occurred: An unexpected value error." in str(e)
        # Check that the original exception is preserved as the cause for better debugging.
        assert isinstance(e.__cause__, ValueError)
    except Exception:
        assert False, "Expected ModelProcessingError, but a different exception was raised"

def test_decorator_logs_errors_correctly():
    """Tests that the decorator logs both expected and unexpected errors."""
    # --- Test logging for TissModelError ---
    log_capture_string.truncate(0)
    log_capture_string.seek(0)

    try:
        function_that_raises_tiss_error()
    except TissModelError:
        pass  # Exception is expected

    log_output = log_capture_string.getvalue()
    assert "A model-related error occurred" in log_output
    assert "InferenceError" in log_output

    # --- Test logging for unexpected error ---
    log_capture_string.truncate(0)
    log_capture_string.seek(0)

    try:
        function_that_raises_unexpected_error()
    except ModelProcessingError:
        pass  # Exception is expected

    log_output = log_capture_string.getvalue()
    assert "An unexpected error occurred" in log_output
    assert "ValueError" in log_output


# --- Test for Custom Exception Classes ---

def test_custom_exception_str_representation():
    """Tests the __str__ representation of custom exception classes."""
    base_error = TissModelError("A base error.")
    assert str(base_error) == "TissModelError: A base error."

    config_error = ConfigurationError("A config error.")
    assert str(config_error) == "ConfigurationError: A config error."

    inference_error = InferenceError("An inference error.")
    assert str(inference_error) == "InferenceError: An inference error."
