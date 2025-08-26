import functools
import logging
import traceback
from .model_error_handler import ModelError

logger = logging.getLogger(__name__)

class SystemError(Exception):
    """Custom exception for system-level errors (e.g., I/O, unexpected exceptions)."""
    pass

class DatabaseConnectionError(SystemError):
    """Custom exception for errors related to database connection."""
    pass

def handle_errors(func):
    """A decorator to handle exceptions and classify them as ModelError or SystemError."""
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except (ModelError, ValueError, TypeError) as e:
            # Re-raise model-related or validation errors directly
            logger.error(f"Error in {func.__name__}: {e}")
            raise
        except Exception as e:
            # Log the full traceback for unexpected system errors
            logger.error(f"Unhandled system error in {func.__name__}: {e}", exc_info=True)
            # Wrap other exceptions in SystemError for clear differentiation
            raise SystemError(f"Unhandled system error in {func.__name__}: {e}") from e
    return wrapper