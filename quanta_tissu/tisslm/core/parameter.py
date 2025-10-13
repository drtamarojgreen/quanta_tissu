from dataclasses import dataclass
import numpy as np

@dataclass
class Parameter:
    """A container for a trainable parameter in the model."""
    value: np.ndarray
    name: str # Add name attribute
    grad: np.ndarray = None
    quantized: bool = False
    scale: float = None
    zero_point: int = None

    def __post_init__(self):
        # Initialize gradient with zeros, same shape as the parameter value.
        self.grad = np.zeros_like(self.value, dtype=np.float32)

    def quantize(self):
        """Quantizes the parameter value to INT8."""
        if not self.quantized:
            # Find the scale and zero point for quantization
            q_min, q_max = -128, 127
            min_val, max_val = self.value.min(), self.value.max()
            
            # Prevent division by zero if all values are the same
            if min_val == max_val:
                self.scale = 1.0
                self.zero_point = 0
            else:
                self.scale = (max_val - min_val) / (q_max - q_min)
                self.zero_point = q_min - min_val / self.scale
            
            # Quantize the value and store as int8
            quantized_value = np.clip(np.round(self.value / self.scale + self.zero_point), q_min, q_max)
            self.value = quantized_value.astype(np.int8)
            self.quantized = True

    def dequantize(self):
        """Dequantizes the parameter value to float."""
        if self.quantized:
            return (self.value.astype(np.float32) - self.zero_point) * self.scale
        else:
            return self.value

class LoRALayer:
    def __init__(self, d_in, d_out, rank, name=""):
        self.A = Parameter(np.random.randn(d_in, rank) / np.sqrt(d_in), name=f"{name}.lora_A")
        self.B = Parameter(np.zeros((rank, d_out)), name=f"{name}.lora_B")

    def __call__(self, x):
        return (x @ self.A.value) @ self.B.value

    def parameters(self):
        return [self.A, self.B]