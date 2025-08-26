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

class FileIOError(SystemError):
    """Custom exception for file I/O errors."""
    pass

class ConfigurationError(SystemError):
    """Custom exception for configuration-related errors."""
    pass

def handle_errors(func):
    """A decorator to handle exceptions and classify them as ModelError or SystemError."""
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except (ModelError, ValueError, TypeError) as e:
            # Re-raise model-related or validation errors directly
            logger.error(f"Model or validation error in {func.__name__}: {e}")
            raise
        except IOError as e:
            logger.error(f"File I/O error in {func.__name__}: {e}", exc_info=True)
            raise FileIOError(f"File I/O error in {func.__name__}: {e}") from e
        except KeyError as e:
            logger.error(f"Configuration key error in {func.__name__}: {e}", exc_info=True)
            raise ConfigurationError(f"Configuration key error in {func.__name__}: {e}") from e
        except DatabaseConnectionError as e:
            logger.error(f"Database connection error in {func.__name__}: {e}", exc_info=True)
            raise # Re-raise the specific database error
        except Exception as e:
            # Log the full traceback for unexpected system errors
            logger.error(f"Unhandled system error in {func.__name__}: {e}", exc_info=True)
            # Wrap other exceptions in SystemError for clear differentiation
            raise SystemError(f"Unhandled system error in {func.__name__}: {e}") from e
    return wrapper