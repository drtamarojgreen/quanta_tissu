import functools
import logging
from ..parser.errors import TissError

# Configure a logger for the module
logger = logging.getLogger(__name__)


class TissModelError(TissError):
    """Base exception for all model-related errors."""

    def __init__(self, message="A model error occurred."):
        self.message = message
        super().__init__(self.message)

    def __str__(self):
        return f'{self.__class__.__name__}: {self.message}'


class InputValidationError(TissModelError):
    """Exception raised for errors in the input data provided to the model."""

    def __init__(self, message="Invalid input data provided to the model."):
        super().__init__(message)


class ModelProcessingError(TissModelError):
    """Exception raised for errors during model processing or inference."""

    def __init__(self, message="Error occurred during model processing."):
        super().__init__(message)


class ConfigurationError(TissModelError):
    """Exception raised for model configuration errors."""

    def __init__(self, message="Invalid model configuration."):
        super().__init__(message)


class InferenceError(ModelProcessingError):
    """Exception raised for errors specifically during the model inference stage."""

    def __init__(self, message="Error during model inference."):
        super().__init__(message)


class TissAssertionError(TissModelError):
    """Raised when an ASSERT command fails."""
    pass

class ModelInitializationError(ModelError):
    """Custom exception for errors during model initialization."""
    pass

class InferenceError(ModelError):
    """Custom exception for errors during model inference."""
    pass

class TrainingError(ModelError):
    """Custom exception for errors during model training."""
    pass

def handle_model_errors(func):
    """
    A decorator that wraps a function to catch and handle exceptions derived from TissModelError.
    It logs the error and can be configured to return a specific error response
    or re-raise the exception.
    """

    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except TissModelError as e:
            logger.error(f"A model-related error occurred in function '{func.__name__}': {e}", exc_info=True)
            # Re-raise to let the caller handle it, which is often a better practice in libraries.
            raise
        except Exception as e:
            logger.error(f"An unexpected error occurred in function '{func.__name__}': {e}", exc_info=True)
            # Wrap unexpected errors in a generic ModelProcessingError
            raise ModelProcessingError(f"An unexpected error occurred: {e}") from e

    return wrapper
