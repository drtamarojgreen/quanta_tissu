import logging
import os
from datetime import datetime
import sys

def setup_evaluation_logger(log_file: str = None, level=logging.INFO):
    """
    Sets up a logger for evaluation scripts.

    Args:
        log_file: Optional path to a file where logs will be written.
                  If None, logs only to console.
        level: Logging level (e.g., logging.INFO, logging.DEBUG).
    """
    # Get the root logger
    logger = logging.getLogger()
    logger.setLevel(level)

    # Clear existing handlers to prevent duplicate logs
    for handler in logger.handlers[:]:
        logger.removeHandler(handler)

    # Create a formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # Console handler
    console_handler = logging.StreamHandler(sys.stdout) # Specify stream
    console_handler.setFormatter(formatter)
    console_handler.setLevel(level) # Set level for handler
    
    logger.addHandler(console_handler)

    # File handler (optional)
    if log_file:
        log_dir = os.path.dirname(log_file)
        if log_dir and not os.path.exists(log_dir):
            os.makedirs(log_dir)
        file_handler = logging.FileHandler(log_file, encoding='utf-8')
        file_handler.setFormatter(formatter)
        logger.addHandler(file_handler)

    logging.info(f"Logger set up. Logging level: {logging.getLevelName(level)}")
    if log_file:
        logging.info(f"Logs also being written to: {log_file}")

def log_section_header(logger: logging.Logger, title: str):
    """
    Logs a formatted section header for better readability in logs.
    """
    logger.info(f"\n{'='*50}\n{title.upper()}\n{'='*50}\n")

def log_subsection_header(logger: logging.Logger, title: str):
    """
    Logs a formatted subsection header.
    """
    logger.info(f"\n{'-'*40}\n{title}\n{'-'*40}\n")

# Example usage:
# if __name__ == "__main__":
#     # Setup logger to console and a file
#     log_path = f"logs/evaluation_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
#     setup_evaluation_logger(log_file=log_path, level=logging.DEBUG)
# 
#     my_logger = logging.getLogger(__name__)
# 
#     log_section_header(my_logger, "Overall Evaluation Run")
#     my_logger.info("Starting evaluation process...")
# 
#     log_subsection_header(my_logger, "Model Configuration Check")
#     my_logger.debug("Checking model parameters...")
# 
#     my_logger.info("Evaluation complete.")
