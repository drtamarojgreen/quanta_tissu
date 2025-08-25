import json
from typing import List, Dict, Any, Optional
from matcher import _PATTERNS
from setup import handle_setup_command
from step import handle_step_command

class TissLangParserError(Exception):
    """Custom exception for parsing errors in TissLang."""
    def __init__(self, message, line_number):
        super().__init__(f"Error on line {line_number}: {message}")
        self.line_number = line_number

class TissLangParser:
    """
    A stateful, line-by-line parser for TissLang scripts.

    This parser transforms a TissLang script into a JSON-serializable
    Abstract Syntax Tree (AST), represented as a list of command dictionaries.
    It is designed according to the specification in `docs/TissLang_plan.md`.
    """

    

    def __init__(self):
        self.ast: List[Dict[str, Any]] = []
        self._state: str = "IDLE"  # Can be IDLE, IN_STEP, IN_WRITE
        self._current_block: Optional[List[Dict[str, Any]]] = None
        self._heredoc_delimiter: Optional[str] = None
        self._heredoc_content: List[str] = []
        self._line_number: int = 0
        self.commands = [] # Added to fix attribute error from old code
        self.metadata = {} # Added to fix attribute error from old code

    def _dedent(self, text: str) -> str:
        """
        Removes common leading whitespace from every line in a string.
        Similar to textwrap.dedent.
        """
        lines = text.splitlines()

        # Find the minimum indentation of all non-empty lines
        min_indent = float('inf')
        for line in lines:
            stripped = line.lstrip()
            if stripped:
                indent = len(line) - len(stripped)
                min_indent = min(min_indent, indent)

        # Remove the minimum indentation from every line
        if min_indent != float('inf'):
            dedented_lines = [line[min_indent:] for line in lines]
            return "\n".join(dedented_lines)

        return text # Return original text if all lines are empty

    def parse(self, script_content: str) -> List[Dict[str, Any]]:
        """
        Parses the full content of a TissLang script.

        Args:
            script_content: A string containing the TissLang script.

        Returns:
            A list of dictionaries representing the script's AST.

        Raises:
            TissLangParserError: If a syntax error is found.
        """
        lines = script_content.splitlines()
        for line in lines:
            self._line_number += 1
            self._parse_line(line)

        if self._state != "IDLE":
            raise TissLangParserError(f"Unexpected end of script. Current state is '{self._state}'. A block may be unclosed.", self._line_number)

        return self.ast

    def _parse_line(self, line: str):
        """Processes a single line of the script based on the current state."""
        if _PATTERNS['COMMENT'].match(line) or _PATTERNS['EMPTY'].match(line):
            return

        if self._state == "IN_WRITE":
            self._handle_write_block(line)
            return

        if self._state == "IDLE":
            self._handle_idle_state(line)
        elif self._state == "IN_STEP":
            self._handle_in_step_state(line)
        else:
            raise TissLangParserError(f"Invalid parser state: {self._state}", self._line_number)

    def _handle_idle_state(self, line: str):
        """Handles parsing when in the top-level 'IDLE' state."""
        task_match = _PATTERNS['TASK'].match(line)
        if task_match:
            self.ast.append({'type': 'TASK', 'description': task_match.group(1)})
            return

        handled, new_state, new_current_block = handle_step_command(line, self.ast, self._current_block, self._state, self._line_number)
        if handled:
            self._state = new_state
            self._current_block = new_current_block
            return

        handled, new_state, new_current_block = handle_setup_command(line, self.ast, self._current_block, self._state, self._line_number)
        if handled:
            self._state = new_state
            self._current_block = new_current_block
            return

        raise TissLangParserError(f"Unexpected command. Expected TASK, STEP, or SETUP.", self._line_number)

    def _handle_in_step_state(self, line: str):
        """Handles parsing when inside a 'STEP' or 'SETUP' block."""
        if _PATTERNS['BLOCK_END'].match(line):
            self._state = "IDLE"
            self._current_block = None
            return

        run_match = _PATTERNS['RUN'].match(line)
        if run_match:
            self._current_block.append({'type': 'RUN', 'command': run_match.group(2)})
            return

        assert_match = _PATTERNS['ASSERT'].match(line)
        if assert_match:
            self._current_block.append({'type': 'ASSERT', 'condition': assert_match.group(1)})
            return

        read_match = _PATTERNS['READ'].match(line)
        if read_match:
            self._current_block.append({'type': 'READ', 'path': read_match.group(1), 'variable': read_match.group(2)})
            return

        write_match = _PATTERNS['WRITE'].match(line)
        if write_match:
            self._heredoc_delimiter = write_match.group(2)
            write_node = {'type': 'WRITE', 'path': write_match.group(1), 'language': self._heredoc_delimiter, 'content': ''}
            self._current_block.append(write_node)
            self._state = "IN_WRITE"
            return

        raise TissLangParserError(f"Unexpected command inside STEP/SETUP block.", self._line_number)

    def _handle_write_block(self, line: str):
        """Handles parsing when inside a 'WRITE' heredoc block."""
        if line.strip() == self._heredoc_delimiter:
            # Last command added to the current block must be the WRITE command
            write_node = self._current_block[-1]

            # Join, dedent, and strip the collected lines
            content = "".join(self._heredoc_content)
            write_node['content'] = self._dedent(content).rstrip('\n')

            # Reset state
            self._heredoc_delimiter = None
            self._heredoc_content = []
            self._state = "IN_STEP"
        else:
            self._heredoc_content.append(line + '\n')
