import os
import subprocess
from typing import Dict, Any

# Assuming State is defined in execution_engine, we'll need to import it.
# from .execution_engine import State 
# For now, using 'Any' to avoid circular dependency issues until all files are in place.
from .execution_engine import State


class TissCommandError(Exception):
    """Custom exception for errors during TissLang command execution."""
    pass

def run_command(state: State, args: Dict[str, Any]):
    """
    Executes a shell command and captures its output.

    Args:
        state: The current execution state.
        args: A dictionary containing the 'command' to run.
    """
    command = args.get("command")
    if not command:
        raise TissCommandError("RUN command is missing the 'command' argument.")

    print(f"Executing command: {command}")
    # Security Warning: Running arbitrary commands is dangerous.
    # In a real-world scenario, this should be heavily sandboxed.
    try:
        result = subprocess.run(
            command,
            shell=True,
            capture_output=True,
            text=True,
            cwd=os.getcwd() # Ensure command runs in the project's root directory
        )
        state.last_run_result = {
            "stdout": result.stdout,
            "stderr": result.stderr,
            "exit_code": result.returncode,
        }
        print(f"Command finished with exit code: {result.returncode}")
    except Exception as e:
        error_message = f"Failed to execute command '{command}': {e}"
        print(error_message)
        state.last_run_result = {
            "stdout": "",
            "stderr": error_message,
            "exit_code": 1,
        }
        state.is_halted = True


def _is_path_safe(path: str) -> bool:
    """
    Validates that the path is within the current working directory.
    Prevents directory traversal attacks (e.g., '../../etc/passwd').
    """
    project_root = os.path.abspath(os.getcwd())
    requested_path = os.path.abspath(os.path.join(project_root, path))
    return os.path.commonpath([project_root]) == os.path.commonpath([project_root, requested_path])


def write_file(state: State, args: Dict[str, Any]):
    """
    Writes content to a specified file, with path validation.

    Args:
        state: The current execution state.
        args: A dictionary containing 'path' and 'content'.
    """
    path = args.get("path")
    content = args.get("content", "")
    if not path:
        raise TissCommandError("WRITE command is missing the 'path' argument.")

    if not _is_path_safe(path):
        state.is_halted = True
        raise TissCommandError(f"Security Error: Path '{path}' is outside the allowed project directory.")

    try:
        # Ensure the directory exists
        dir_name = os.path.dirname(path)
        if dir_name:
            os.makedirs(dir_name, exist_ok=True)
        
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"Successfully wrote to file: {path}")
    except IOError as e:
        state.is_halted = True
        raise TissCommandError(f"IO Error writing to file '{path}': {e}")


def read_file(state: State, args: Dict[str, Any]):
    """
    Reads a file's content into a variable in the execution state.

    Args:
        state: The current execution state.
        args: A dictionary containing 'path' and 'variable'.
    """
    path = args.get("path")
    variable_name = args.get("variable")
    if not path or not variable_name:
        raise TissCommandError("READ command is missing 'path' or 'variable' argument.")

    if not _is_path_safe(path):
        state.is_halted = True
        raise TissCommandError(f"Security Error: Path '{path}' is outside the allowed project directory.")

    try:
        with open(path, "r", encoding="utf-8") as f:
            content = f.read()
        state.variables[variable_name] = content
        print(f"Successfully read file '{path}' into variable '{variable_name}'.")
    except FileNotFoundError:
        state.is_halted = True
        raise TissCommandError(f"File not found: {path}")
    except IOError as e:
        state.is_halted = True
        raise TissCommandError(f"IO Error reading file '{path}': {e}")


import re

def _resolve_value(path: str, state: State):
    """Resolves a value from the state based on a path-like string (e.g., 'LAST_RUN.EXIT_CODE')."""
    if path == "LAST_RUN.EXIT_CODE":
        if state.last_run_result:
            return state.last_run_result.get("exit_code")
        return None
    if path == "LAST_RUN.STDOUT":
        if state.last_run_result:
            return state.last_run_result.get("stdout")
        return None
    if path == "LAST_RUN.STDERR":
        if state.last_run_result:
            return state.last_run_result.get("stderr")
        return None
    # Can be extended to support `variables.my_var` in the future
    raise TissCommandError(f"Cannot resolve value for '{path}'.")

def assert_condition(state: State, args: Dict[str, Any]):
    """
    Evaluates an assertion condition against the current state.

    Args:
        state: The current execution state.
        args: A dictionary containing the 'condition' string.
    """
    condition = args.get("condition")
    if not condition:
        raise TissCommandError("ASSERT command is missing the 'condition' argument.")

    # Regex to parse 'LHS OPERATOR RHS'
    # Supports ==, !=, and CONTAINS operators. RHS can be quoted.
    match = re.match(r"^\s*(LAST_RUN\.\w+)\s+(==|!=|CONTAINS)\s+(\S+?|\".*?\")\s*$", condition)
    if not match:
        raise TissCommandError(f"Invalid ASSERT syntax: '{condition}'")

    lhs_path, operator, rhs_raw = match.groups()
    
    # Clean up RHS value (remove quotes)
    if rhs_raw.startswith('"') and rhs_raw.endswith('"'):
        rhs = rhs_raw[1:-1]
    else:
        rhs = rhs_raw

    # Get the actual value from the state
    lhs_value = _resolve_value(lhs_path, state)

    # Perform the comparison
    result = False
    if operator == "==":
        # Attempt to cast RHS to the type of LHS for comparison
        try:
            result = (lhs_value == type(lhs_value)(rhs))
        except (ValueError, TypeError):
            result = False # Type mismatch
    elif operator == "!=":
        try:
            result = (lhs_value != type(lhs_value)(rhs))
        except (ValueError, TypeError):
            result = True # Type mismatch
    elif operator == "CONTAINS":
        result = isinstance(lhs_value, str) and rhs in lhs_value

    # Update state based on result
    if result:
        print(f"Assertion PASSED: {condition}")
    else:
        print(f"Assertion FAILED: {condition}")
        print(f"  -> Expected: {lhs_path} {operator} {rhs}")
        print(f"  -> Actual: {lhs_path} resolved to '{lhs_value}'")
        state.is_halted = True
