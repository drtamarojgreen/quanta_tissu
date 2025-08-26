class ModelError(Exception):
    """Custom exception for errors originating from the model's logic."""
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
