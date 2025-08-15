from dataclasses import dataclass
import numpy as np

@dataclass
class Parameter:
    """A container for a trainable parameter in the model."""
    value: np.ndarray
    name: str # Add name attribute
    grad: np.ndarray = None

    def __post_init__(self):
        # Initialize gradient with zeros, same shape as the parameter value.
        self.grad = np.zeros_like(self.value)