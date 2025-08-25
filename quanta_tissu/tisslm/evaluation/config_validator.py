import logging
from typing import Dict, Any

# Assuming these imports will be available from the main project context
# from quanta_tissu.tisslm.config import model_config, training_config, system_config, tokenizer_config



def validate_model_config(config: Dict[str, Any]):
    """
    Validates the model configuration.
    """
    logging.info("\n--- Validating Model Configuration ---")
    required_keys = ["vocab_size", "n_layer", "n_head", "n_embd", "block_size"]
    for key in required_keys:
        if key not in config:
            logging.error(f"Missing required key in model_config: {key}")
            return False
    # Add more specific validation rules here, e.g., type checks, value ranges
    if config["n_embd"] % config["n_head"] != 0:
        logging.error(f"n_embd ({config['n_embd']}) must be divisible by n_head ({config['n_head']}).")
        return False
    logging.info("Model configuration is valid.")
    return True

def validate_training_config(config: Dict[str, Any]):
    """
    Validates the training configuration.
    """
    logging.info("\n--- Validating Training Configuration ---")
    required_keys = ["num_epochs", "batch_size", "learning_rate", "weight_decay"]
    for key in required_keys:
        if key not in config:
            logging.error(f"Missing required key in training_config: {key}")
            return False
    # Add more specific validation rules
    if config["learning_rate"] <= 0:
        logging.error("Learning rate must be positive.")
        return False
    logging.info("Training configuration is valid.")
    return True

def validate_tokenizer_config(config: Dict[str, Any]):
    """
    Validates the tokenizer configuration.
    """
    logging.info("\n--- Validating Tokenizer Configuration ---")
    required_keys = ["max_len"]
    for key in required_keys:
        if key not in config:
            logging.error(f"Missing required key in tokenizer_config: {key}")
            return False
    # Add more specific validation rules
    if config["max_len"] <= 0:
        logging.error("max_len must be positive.")
        return False
    logging.info("Tokenizer configuration is valid.")
    return True

def validate_all_configs(model_cfg, training_cfg, tokenizer_cfg, system_cfg=None):
    """
    Validates all configurations.
    """
    logging.info("\n--- Validating All Configurations ---")
    all_valid = True
    if not validate_model_config(model_cfg):
        all_valid = False
    if not validate_training_config(training_cfg):
        all_valid = False
    if not validate_tokenizer_config(tokenizer_cfg):
        all_valid = False
    # Add system_config validation if needed
    if all_valid:
        logging.info("All configurations are valid.")
    else:
        logging.error("One or more configurations are invalid.")
    return all_valid

# Example usage (assuming configs are imported or passed)
# if __name__ == "__main__":
#     from quanta_tissu.tisslm.config import model_config, training_config, tokenizer_config, system_config
#     validate_all_configs(model_config, training_config, tokenizer_config, system_config)
