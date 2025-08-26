import functools
import logging
import traceback
from .model_error_handler import ModelError
from ..parser.errors import TissError

logger = logging.getLogger(__name__)

class TissSystemError(TissError):
    """Custom exception for system-level errors (e.g., I/O, unexpected exceptions)."""
    pass

class DatabaseConnectionError(TissSystemError):
    """Custom exception for errors related to database connection."""
    pass

class TissCommandError(TissSystemError):
    """Raised when a TissLang command fails during execution."""
    pass

class TissSecurityError(TissCommandError):
    """Raised when a command attempts a forbidden action."""
    pass

def handle_errors(func):
    """A decorator to handle exceptions and classify them as ModelError or TissSystemError."""
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
            # Wrap other exceptions in TissSystemError for clear differentiation
            raise TissSystemError(f"Unhandled system error in {func.__name__}: {e}") from e
    return wrapper