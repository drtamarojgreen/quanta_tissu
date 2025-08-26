import time
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Callable

# The Protocol is no longer needed here, as the BaseTool ABC serves as the interface.
# from .tools.base import BaseTool

from .system_error_handler import TissSystemError
from .model_error_handler import TissModelError, ConfigurationError

@dataclass
class State:
    """
    Encapsulates all mutable state for a given TissLang execution run.
    This remains unchanged as it's a pure data class.
    """
    last_run_result: Optional[Dict[str, Any]] = None
    variables: Dict[str, str] = field(default_factory=dict)
    is_halted: bool = False
    execution_log: List[Dict[str, Any]] = field(default_factory=list)

class ToolRegistry:
    """
    A registry that maps TissLang command types to their tool implementation.
    This class is now more important as it's the bridge between the engine and the tools.
    """
    def __init__(self):
        # The tool type is now `BaseTool` from our new module.
        self._tools: Dict[str, Any] = {}

    def register(self, command_type: str, tool: Any):
        """
        Registers a tool for a given command type.

        Args:
            command_type: The name of the command (e.g., "RUN", "WRITE").
            tool: An instance of a class that inherits from `BaseTool`.
        """
        self._tools[command_type.upper()] = tool

    def get_tool(self, command_type: str) -> Any:
        """
        Retrieves the tool for a given command type.
        """
        tool = self._tools.get(command_type.upper())
        if tool is None:
            raise ConfigurationError(f"No tool registered for command type: {command_type}")
        return tool

class ExecutionEngine:
    """
    Executes a TissLang AST by dispatching commands to registered tools.
    This engine is now a pure dispatcher and contains no tool-specific logic.
    """
    def __init__(self, tool_registry: ToolRegistry):
        self.tool_registry = tool_registry

    def execute(self, ast: List[Dict[str, Any]]) -> State:
        """
        Executes a TissLang script from its AST representation.
        """
        state = State()
        for command in ast:
            if state.is_halted:
                break
            self._execute_command(command, state)
        return state

    def _execute_command(self, command: Dict[str, Any], state: State):
        """Executes a single command by dispatching it to a registered tool."""
        command_type = command.get("type")
        if not command_type:
            return

        log_entry = {'command': command, 'status': 'SKIPPED'}

        # Structural commands are still handled by the engine itself
        if command_type in ("TASK", "STEP", "SETUP"):
            # This logic can be kept as it's about control flow, not tool execution
            if command_type == "TASK":
                log_entry['status'] = 'SUCCESS'
            else: # STEP or SETUP
                for sub_command in command.get("commands", []):
                    if state.is_halted: break
                    self._execute_command(sub_command, state)
                log_entry['status'] = 'FAILURE' if state.is_halted else 'SUCCESS'
            state.execution_log.append(log_entry)
            return

        # Tool-based commands are dispatched
        start_time = time.monotonic()
        try:
            tool = self.tool_registry.get_tool(command_type)
            args = {k: v for k, v in command.items() if k != 'type'}

            # --- Variable Substitution ---
            import re
            for key, value in args.items():
                if isinstance(value, str):
                    matches = re.findall(r"\{\{([a-zA-Z_][a-zA-Z0-9_]*)\}\}", value)
                    for var_name in matches:
                        if var_name in state.variables:
                            value = value.replace(f"{{{{{var_name}}}}}", state.variables[var_name])
                    args[key] = value
            # --- End Variable Substitution ---

            # The tool's execute method is called
            tool.execute(state, args)
            log_entry['status'] = 'SUCCESS'
        except (TissSystemError, TissModelError) as e:
            log_entry['status'] = 'FAILURE'
            log_entry['error'] = str(e)
            # Halt execution for any tool-related error
            state.is_halted = True
        finally:
            end_time = time.monotonic()
            log_entry['duration_ms'] = (end_time - start_time) * 1000
            state.execution_log.append(log_entry)
