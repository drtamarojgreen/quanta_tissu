import os
import re
import subprocess
from typing import Dict, Any

from .base import BaseTool
from ..system_error_handler import TissSystemError, TissSecurityError
from ..model_error_handler import TissModelError, TissAssertionError

class RunCommandTool(BaseTool):
    """Tool to execute a shell command."""
    def __init__(self, project_root: str):
        self.project_root = project_root

    def execute(self, state: Any, args: Dict[str, Any]):
        command = args.get("command")
        if not command:
            raise TissModelError("RUN command requires a 'command' argument.")

        try:
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
        except FileNotFoundError as e:
            raise TissSystemError(f"Command not found: '{command}': {e}") from e
        except Exception as e:
            raise TissSystemError(f"Failed to execute command '{command}': {e}") from e

class WriteFileTool(BaseTool):
    """Tool to write content to a file."""
    def __init__(self, project_root: str):
        self.project_root = project_root

    def _resolve_secure_path(self, target_path: str) -> str:
        if os.path.isabs(target_path):
            raise TissSecurityError(f"Absolute paths are not allowed: '{target_path}'")
        resolved_path = os.path.abspath(os.path.join(self.project_root, target_path))
        if os.path.commonpath([self.project_root, resolved_path]) != self.project_root:
            raise TissSecurityError(f"Path access violation: Attempted to access '{resolved_path}'")
        return resolved_path

    def execute(self, state: Any, args: Dict[str, Any]):
        path = args.get("path")
        content = args.get("content")
        if path is None or content is None:
            raise TissModelError("WRITE command requires 'path' and 'content' arguments.")

        secure_path = self._resolve_secure_path(path)
        try:
            os.makedirs(os.path.dirname(secure_path), exist_ok=True)
            with open(secure_path, 'w', encoding='utf-8') as f:
                f.write(content)
        except IOError as e:
            raise TissSystemError(f"Failed to write to file '{path}': {e}")

class ReadFileTool(BaseTool):
    """Tool to read the content of a file into a variable."""
    def __init__(self, project_root: str):
        self.project_root = project_root

    def _resolve_secure_path(self, target_path: str) -> str:
        # Duplicated for now, could be moved to a shared utility
        if os.path.isabs(target_path):
            raise TissSecurityError(f"Absolute paths are not allowed: '{target_path}'")
        resolved_path = os.path.abspath(os.path.join(self.project_root, target_path))
        if os.path.commonpath([self.project_root, resolved_path]) != self.project_root:
            raise TissSecurityError(f"Path access violation: Attempted to access '{resolved_path}'")
        return resolved_path

    def execute(self, state: Any, args: Dict[str, Any]):
        path = args.get("path")
        variable = args.get("variable")
        if not path or not variable:
            raise TissModelError("READ command requires 'path' and 'variable' arguments.")

        secure_path = self._resolve_secure_path(path)
        try:
            with open(secure_path, 'r', encoding='utf-8') as f:
                content = f.read()
            state.variables[variable] = content
        except FileNotFoundError:
            raise TissSystemError(f"File not found: '{path}'")
        except IOError as e:
            raise TissSystemError(f"Failed to read file '{path}': {e}")

class AssertConditionTool(BaseTool):
    """Tool to assert conditions about the last run command."""
    def execute(self, state: Any, args: Dict[str, Any]):
        condition_str = args.get("condition", "").strip()
        if not condition_str:
            raise TissModelError("ASSERT command requires a 'condition' argument.")

        if state.last_run_result is None:
            raise TissAssertionError("Cannot assert on LAST_RUN because no command has been run yet.")

        pattern_three_part = re.compile(r'^(LAST_RUN\.(?:STDOUT|STDERR|EXIT_CODE))\s+(==|!=|CONTAINS)\s+(.*)', re.IGNORECASE)
        pattern_two_part = re.compile(r'^(LAST_RUN\.(?:STDOUT|STDERR))\s+(IS_EMPTY)', re.IGNORECASE)

        match_three = pattern_three_part.match(condition_str)
        match_two = pattern_two_part.match(condition_str)

        if not match_three and not match_two:
            raise TissAssertionError(f"Invalid assertion syntax: '{condition_str}'")

        success = False
        actual_value = None

        if match_three:
            subject, operator, value_str = match_three.groups()
            subject_norm, operator = subject.upper(), operator.upper()

            if subject_norm == 'LAST_RUN.EXIT_CODE':
                actual_value = state.last_run_result['exit_code']
                expected_value = int(value_str)
            else:
                actual_value = state.last_run_result['stdout' if 'STDOUT' in subject_norm else 'stderr']
                expected_value = value_str.strip('"')

            if operator == '==': success = (actual_value == expected_value)
            elif operator == '!=': success = (actual_value != expected_value)
            elif operator == 'CONTAINS':
                if not isinstance(actual_value, str):
                    raise TissAssertionError(f"CONTAINS operator requires a string subject, but got {type(actual_value)}")
                success = (expected_value in actual_value)

        elif match_two:
            subject, operator = match_two.groups()
            actual_value = state.last_run_result['stdout' if 'STDOUT' in subject.upper() else 'stderr']
            success = (actual_value.strip() == "")

        if not success:
            raise TissAssertionError(f"Assertion failed: `{condition_str}`. Actual value was: '{actual_value}'")
