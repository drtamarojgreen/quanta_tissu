from dataclasses import dataclass, field
from typing import Optional, List, Dict, Any

@dataclass
class GenerationConfig:
    """
    Configuration for text generation.
    """
    method: str = "greedy"
    temperature: float = 1.0
    top_k: Optional[int] = None
    top_p: Optional[float] = None
    top_a: float = 0.0
    repetition_penalty: float = 1.0
    eos_id: Optional[int] = None
    no_repeat_ngram_size: int = 0
    logit_bias: Optional[Dict[int, float]] = None
    temperature_schedule: Optional[List[float]] = None
    suppress_eos: bool = False
