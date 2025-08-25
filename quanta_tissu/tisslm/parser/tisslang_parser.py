import json
from typing import List, Dict, Any, Optional
from .matcher import _PATTERNS
from .conditional import CONDITIONAL_PATTERNS
from .setup import handle_setup_command
from .step import handle_step_command

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
        """Initializes the parser, setting up the initial state."""
        self.ast: List[Dict[str, Any]] = []
        self._state_stack: List[str] = ["IDLE"]
        self._block_stack: List[List[Dict[str, Any]]] = [self.ast]
        self._heredoc_delimiter: Optional[str] = None
        self._heredoc_content: List[str] = []
        self._line_number: int = 0
        self._patterns = {**_PATTERNS, **CONDITIONAL_PATTERNS}

    def _current_state(self) -> str:
        """Returns the current state from the top of the state stack."""
        return self._state_stack[-1]

    def _current_block(self) -> List[Dict[str, Any]]:
        """Returns the current block from the top of the block stack."""
        return self._block_stack[-1]

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

        if self._current_state() != "IDLE":
            raise TissLangParserError(f"Unexpected end of script. Current state is '{self._current_state()}'. A block may be unclosed.", self._line_number)

        return self.ast

    def _parse_line(self, line: str):
        """Processes a single line of the script based on the current state."""
        if self._patterns['EMPTY'].match(line):
            return

        if self._current_state() == "IN_WRITE":
            self._handle_write_block(line)
            return

        comment_match = self._patterns['COMMENT'].match(line)
        if comment_match:
            # Comments are added to the AST for better representation,
            # unless we are inside a block, in which case they are added to the block's command list.
            target_list = self._current_block() if self._current_state() != "IDLE" else self.ast
            target_list.append({'type': 'COMMENT', 'text': line.strip()})
            return

        if self._current_state() == "IDLE":
            self._handle_idle_state(line)
        elif self._current_state() in ["IN_STEP", "IN_IF", "IN_ELSE"]:
            self._handle_in_block_state(line)
        else:
            raise TissLangParserError(f"Invalid parser state: {self._current_state()}", self._line_number)

    def _handle_idle_state(self, line: str):
        """Handles parsing when in the top-level 'IDLE' state."""
        task_match = self._patterns['TASK'].match(line)
        if task_match:
            self.ast.append({'type': 'TASK', 'description': task_match.group(1)})
            return

        if handle_step_command(line, self.ast, self._state_stack, self._block_stack, self._line_number):
            return

        if handle_setup_command(line, self.ast, self._state_stack, self._block_stack, self._line_number):
            return

        if_match = self._patterns['IF'].match(line)
        if if_match:
            if_node = {
                'type': 'IF',
                'condition': if_match.group(1),
                'true_branch': [],
                'false_branch': []
            }
            self._current_block().append(if_node)
            self._state_stack.append("IN_IF")
            self._block_stack.append(if_node['true_branch'])
            return

        raise TissLangParserError(f"Unexpected command in IDLE state. Expected TASK, STEP, SETUP, or IF.", self._line_number)

    def _handle_in_block_state(self, line: str):
        """Handles parsing when inside any block (STEP, IF, etc.)."""
        if self._current_state() == "IN_STEP":
            if self._patterns['BLOCK_END'].match(line):
                self._state_stack.pop()
                self._block_stack.pop()
                return

        run_match = self._patterns['RUN'].match(line)
        if run_match:
            self._current_block().append({'type': 'RUN', 'command': run_match.group(2)})
            return

        log_match = self._patterns['LOG'].match(line)
        if log_match:
            self._current_block().append({'type': 'LOG', 'message': log_match.group(2)})
            return

        assert_match = self._patterns['ASSERT'].match(line)
        if assert_match:
            self._current_block().append({'type': 'ASSERT', 'condition': assert_match.group(1)})
            return

        read_match = self._patterns['READ'].match(line)
        if read_match:
            self._current_block().append({'type': 'READ', 'path': read_match.group(1), 'variable': read_match.group(2)})
            return

        write_match = self._patterns['WRITE'].match(line)
        if write_match:
            self._heredoc_delimiter = write_match.group(2)
            write_node = {'type': 'WRITE', 'path': write_match.group(1), 'language': self._heredoc_delimiter, 'content': ''}
            self._current_block().append(write_node)
            self._state_stack.append("IN_WRITE")
            return

        if_match = self._patterns['IF'].match(line)
        if if_match:
            if_node = {
                'type': 'IF',
                'condition': if_match.group(1),
                'true_branch': [],
                'false_branch': []
            }
            self._current_block().append(if_node)
            self._state_stack.append("IN_IF")
            self._block_stack.append(if_node['true_branch'])
            return

        else_match = self._patterns['ELSE'].match(line)
        if else_match:
            if self._current_state() != "IN_IF":
                raise TissLangParserError("ELSE found without a preceding IF.", self._line_number)

            self._state_stack.pop() # Pop IN_IF
            self._state_stack.append("IN_ELSE")

            self._block_stack.pop() # Pop true_branch
            # The current block is now the parent of the IF node
            if_node = self._current_block()[-1]
            self._block_stack.append(if_node['false_branch'])
            return

        endif_match = self._patterns['ENDIF'].match(line)
        if endif_match:
            if self._current_state() not in ["IN_IF", "IN_ELSE"]:
                raise TissLangParserError("ENDIF found without a preceding IF.", self._line_number)

            self._state_stack.pop()
            self._block_stack.pop()
            return

        raise TissLangParserError(f"Unexpected command inside {self._current_state()} block.", self._line_number)

    def _handle_write_block(self, line: str):
        """Handles parsing when inside a 'WRITE' heredoc block."""
        if line.strip() == self._heredoc_delimiter:
            # Last command added to the current block must be the WRITE command
            write_node = self._current_block()[-1]

            # Join, dedent, and strip the collected lines
            content = "".join(self._heredoc_content)
            write_node['content'] = self._dedent(content).rstrip('\n')

            # Reset state
            self._heredoc_delimiter = None
            self._heredoc_content = []
            self._state_stack.pop() # Exit IN_WRITE state
        else:
            self._heredoc_content.append(line + '\n')
