from abc import ABC, abstractmethod
from typing import Dict, Any

# Assuming State is defined elsewhere, but for modularity, we might redefine a protocol
# For now, we rely on the State object from the execution engine context.
# from ..execution_engine import State
# To avoid circular dependency, we can use forward declaration if State is complex,
# but since it's a dataclass, importing it should be fine if we structure it well.
# Let's assume the engine will pass the state object.

class BaseTool(ABC):
    """
    Abstract base class for all tools executable by the ExecutionEngine.
    """
    @abstractmethod
    def execute(self, state: Any, args: Dict[str, Any]):
        """
        Executes the tool's logic.

        Args:
            state (Any): The current execution state. This is of type `State` from the
                         execution engine, but typed as `Any` to avoid circular imports.
            args (Dict[str, Any]): A dictionary of arguments for the tool, parsed
                                   from the TissLang script.

        Returns:
            None. The tool is expected to modify the state object directly.
        """
        pass
