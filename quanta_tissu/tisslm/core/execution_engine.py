import os
import re
import time
import subprocess
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Callable, Protocol

# This file will contain the core components for the TissLang execution engine.
# It will include the State, ToolRegistry, and ExecutionEngine classes.

@dataclass
class State:
    """
    Encapsulates all mutable state for a given TissLang execution run.

    Attributes:
        last_run_result (Optional[Dict[str, Any]]): A dictionary containing the
            `stdout`, `stderr`, and `exit_code` of the most recent RUN command.
        variables (Dict[str, str]): A dictionary mapping variable names from
            READ commands to their string content.
        is_halted (bool): A flag that, when True, stops further command execution.
        execution_log (List[Dict[str, Any]]): A structured log of every command
            executed and its outcome.
    """
    last_run_result: Optional[Dict[str, Any]] = None
    variables: Dict[str, str] = field(default_factory=dict)
    is_halted: bool = False
    execution_log: List[Dict[str, Any]] = field(default_factory=list)


class Tool(Protocol):
    """A protocol defining the standard interface for a TissLang tool."""
    def __call__(self, state: State, args: Dict[str, Any]) -> None:
        ...

class ToolRegistry:
    """
    A registry that maps TissLang command types to their implementation.

    This class decouples the ExecutionEngine from the concrete tool
    implementations, allowing for modular and extensible toolsets.
    """
    def __init__(self):
        self._tools: Dict[str, Tool] = {}

    def register(self, command_type: str, tool: Tool):
        """
        Registers a tool for a given command type.

        Args:
            command_type: The name of the command (e.g., "RUN", "WRITE").
            tool: A callable that implements the tool's logic.
        """
        self._tools[command_type.upper()] = tool

    def get_tool(self, command_type: str) -> Tool:
        """
        Retrieves the tool for a given command type.

        Args:
            command_type: The name of the command.

        Returns:
            The callable tool implementation.

        Raises:
            KeyError: If no tool is registered for the command type.
        """
        tool = self._tools.get(command_type.upper())
        if tool is None:
            raise KeyError(f"No tool registered for command type: {command_type}")
        return tool


class TissCommandError(Exception):
    """Raised when a TissLang command fails during execution."""
    pass

class TissAssertionError(TissCommandError):
    """Raised when an ASSERT command fails."""
    pass

class TissSecurityError(TissCommandError):
    """Raised when a command attempts a forbidden action."""
    pass


class ExecutionEngine:
    """
    Executes a TissLang AST by dispatching commands to registered tools.
    """
    def __init__(self, tool_registry: ToolRegistry, project_root: str = "."):
        self.tool_registry = tool_registry
        self.project_root = os.path.abspath(project_root)
        self._register_tools()

    def _register_tools(self):
        """Registers the built-in TissLang tools."""
        self.tool_registry.register("RUN", self.run_command)
        self.tool_registry.register("WRITE", self.write_file)
        self.tool_registry.register("READ", self.read_file)
        self.tool_registry.register("ASSERT", self.assert_condition)

    def _resolve_secure_path(self, target_path: str) -> str:
        """
        Resolves a path relative to the project root and ensures it's secure.
        """
        if os.path.isabs(target_path):
            raise TissSecurityError(f"Absolute paths are not allowed: '{target_path}'")

        resolved_path = os.path.abspath(os.path.join(self.project_root, target_path))

        if os.path.commonpath([self.project_root, resolved_path]) != self.project_root:
            raise TissSecurityError(f"Path access violation: Attempted to access '{resolved_path}' which is outside the project root.")

        return resolved_path

    def execute(self, ast: List[Dict[str, Any]]) -> State:
        """
        Executes a TissLang script from its AST representation.

        Args:
            ast: The list of command dictionaries from the TissLangParser.

        Returns:
            The final state of the execution environment.
        """
        state = State()
        for command in ast:
            if state.is_halted:
                break
            self._execute_command(command, state)
        return state

    def _execute_command(self, command: Dict[str, Any], state: State):
        """Executes a single command and updates the state."""
        command_type = command.get("type")
        if not command_type:
            return

        log_entry = {'command': command, 'status': 'SKIPPED'}

        # Handle structural commands that don't map to a single tool
        if command_type == "TASK":
            # TASK is just a descriptor, does not have sub-commands.
            log_entry['status'] = 'SUCCESS'
            state.execution_log.append(log_entry)
            return

        if command_type in ("STEP", "SETUP"):
            # Execute sub-commands
            for sub_command in command.get("commands", []):
                if state.is_halted:
                    break
                self._execute_command(sub_command, state)
            
            # If any sub-command failed, the whole block is considered failed.
            if not state.is_halted:
                log_entry['status'] = 'SUCCESS'
            else:
                log_entry['status'] = 'FAILURE'

            state.execution_log.append(log_entry)
            return

        # Handle tool-based commands
        start_time = time.monotonic()
        try:
            tool = self.tool_registry.get_tool(command_type)
            args = {k: v for k, v in command.items() if k != 'type'}
            tool(state, args)
            log_entry['status'] = 'SUCCESS'
        except (TissCommandError, KeyError) as e:
            log_entry['status'] = 'FAILURE'
            log_entry['error'] = str(e)
            state.is_halted = True
        finally:
            end_time = time.monotonic()
            log_entry['duration_ms'] = (end_time - start_time) * 1000
            state.execution_log.append(log_entry)

    # --- Tool Implementations ---

    def run_command(self, state: State, args: Dict[str, Any]):
        command = args.get("command")
        if not command:
            raise TissCommandError("RUN command requires a 'command' argument.")
        
        try:
            # Note: shell=True is a security risk. Proper sandboxing is required for production.
            result = subprocess.run(
                command,
                shell=True,
                capture_output=True,
                text=True,
                cwd=self.project_root,
                encoding='utf-8'
            )
            state.last_run_result = {
                "stdout": result.stdout,
                "stderr": result.stderr,
                "exit_code": result.returncode,
            }
        except Exception as e:
            raise TissCommandError(f"Failed to execute command '{command}': {e}")

    def write_file(self, state: State, args: Dict[str, Any]):
        path = args.get("path")
        content = args.get("content")
        if path is None or content is None:
            raise TissCommandError("WRITE command requires 'path' and 'content' arguments.")

        secure_path = self._resolve_secure_path(path)
        
        try:
            os.makedirs(os.path.dirname(secure_path), exist_ok=True)
            with open(secure_path, 'w', encoding='utf-8') as f:
                f.write(content)
        except IOError as e:
            raise TissCommandError(f"Failed to write to file '{path}': {e}")

    def read_file(self, state: State, args: Dict[str, Any]):
        path = args.get("path")
        variable = args.get("variable")
        if not path or not variable:
            raise TissCommandError("READ command requires 'path' and 'variable' arguments.")

        secure_path = self._resolve_secure_path(path)

        try:
            with open(secure_path, 'r', encoding='utf-8') as f:
                content = f.read()
            state.variables[variable] = content
        except FileNotFoundError:
            raise TissCommandError(f"File not found: '{path}'")
        except IOError as e:
            raise TissCommandError(f"Failed to read file '{path}': {e}")

    def assert_condition(self, state: State, args: Dict[str, Any]):
        condition_str = args.get("condition", "").strip()
        if not condition_str:
            raise TissCommandError("ASSERT command requires a 'condition' argument.")

        if state.last_run_result is None:
            raise TissAssertionError("Cannot assert on LAST_RUN because no command has been run yet.")

        # Pattern for: [SUBJECT] [OPERATOR] [VALUE]
        pattern_three_part = re.compile(r'^(LAST_RUN\.(?:STDOUT|STDERR|EXIT_CODE))\s+(==|!=|CONTAINS)\s+(.*)$', re.IGNORECASE)
        # Pattern for: [SUBJECT] [OPERATOR]
        pattern_two_part = re.compile(r'^(LAST_RUN\.(?:STDOUT|STDERR))\s+(IS_EMPTY)$', re.IGNORECASE)

        match_three = pattern_three_part.match(condition_str)
        match_two = pattern_two_part.match(condition_str)

        if not match_three and not match_two:
            raise TissAssertionError(f"Invalid assertion syntax: '{condition_str}'")

        success = False
        actual_value = None

        if match_three:
            subject, operator, value_str = match_three.groups()
            subject_norm = subject.upper()
            operator = operator.upper()

            if subject_norm == 'LAST_RUN.EXIT_CODE':
                actual_value = state.last_run_result['exit_code']
                expected_value = int(value_str)
            else: # STDOUT or STDERR
                actual_value = state.last_run_result['stdout' if 'STDOUT' in subject_norm else 'stderr']
                expected_value = value_str.strip('"')

            if operator == '==':
                success = (actual_value == expected_value)
            elif operator == '!=':
                success = (actual_value != expected_value)
            elif operator == 'CONTAINS':
                if not isinstance(actual_value, str):
                     raise TissAssertionError(f"CONTAINS operator requires a string subject, but got {type(actual_value)} for {subject}")
                success = (expected_value in actual_value)

        elif match_two:
            subject, operator = match_two.groups()
            subject_norm = subject.upper() # LAST_RUN.STDOUT or LAST_RUN.STDERR
            actual_value = state.last_run_result['stdout' if 'STDOUT' in subject_norm else 'stderr']
            success = (actual_value.strip() == "")

        if not success:
            raise TissAssertionError(f"Assertion failed: `{condition_str}`. Actual value was: '{actual_value}'")
