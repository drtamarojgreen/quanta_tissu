import json
from typing import List, Dict, Any, Optional
from .matcher import _PATTERNS
from .setup import handle_setup_command
from .step import handle_step_command
from .errors import TissLangParserError
from .value_parser import parse_value


class TissLangParser:
    """
    A stateful, line-by-line parser for TissLang scripts.

    This parser transforms a TissLang script into a JSON-serializable
    Abstract Syntax Tree (AST), represented as a list of command dictionaries.
    It is designed according to the specification in `docs/TissLang_plan.md`.
    """

    

    def __init__(self):
        self.ast: List[Dict[str, Any]] = []
        self._state: str = "IDLE"  # Can be IDLE, IN_STEP, IN_WRITE, IN_CHOOSE
        self._state_stack: List[str] = []
        self._current_block: Optional[List[Dict[str, Any]]] = None
        self._block_stack: List[Optional[List[Dict[str, Any]]]] = []
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
        if _PATTERNS['EMPTY'].match(line):
            return

        if self._state == "IN_WRITE":
            self._handle_write_block(line)
            return

        comment_match = _PATTERNS['COMMENT'].match(line)
        if comment_match:
            # Comments are added to the AST for better representation,
            # unless we are inside a STEP/SETUP block, in which case they
            # are added to the block's command list.
            target_list = self._current_block if self._state == "IN_STEP" else self.ast
            target_list.append({'type': 'COMMENT', 'text': line.strip()})
            return

        if self._state == "IDLE":
            self._handle_idle_state(line)
        elif self._state == "IN_STEP":
            self._handle_in_step_state(line)
        elif self._state == "IN_CHOOSE":
            self._handle_in_choose_state(line)
        else:
            raise TissLangParserError(f"Invalid parser state: {self._state}", self._line_number)

    def _handle_idle_state(self, line: str):
        """Handles parsing when in the top-level 'IDLE' state."""
        task_match = _PATTERNS['TASK'].match(line)
        if task_match:
            self.ast.append({'type': 'TASK', 'description': task_match.group(1)})
            return

        step_node = handle_step_command(line, self.ast, self._line_number)
        if step_node:
            self._state_stack.append(self._state)
            self._block_stack.append(self._current_block)
            self._state = "IN_STEP"
            self._current_block = step_node['commands']
            return

        setup_node = handle_setup_command(line, self.ast, self._line_number)
        if setup_node:
            self._state_stack.append(self._state)
            self._block_stack.append(self._current_block)
            self._state = "IN_STEP"
            self._current_block = setup_node['commands']
            return

        raise TissLangParserError(f"Unexpected command. Expected TASK, STEP, or SETUP.", self._line_number)

    def _handle_in_step_state(self, line: str):
        """Handles parsing when inside a 'STEP' or 'SETUP' block."""
        if _PATTERNS['BLOCK_END'].match(line):
            self._state = self._state_stack.pop()
            self._current_block = self._block_stack.pop()
            return

        run_match = _PATTERNS['RUN'].match(line)
        if run_match:
            self._current_block.append({'type': 'RUN', 'command': run_match.group(2)})
            return

        log_match = _PATTERNS['LOG'].match(line)
        if log_match:
            self._current_block.append({'type': 'LOG', 'message': log_match.group(2)})
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

        prompt_agent_match = _PATTERNS['PROMPT_AGENT'].match(line)
        if prompt_agent_match:
            prompt = prompt_agent_match.group(1)
            variable = prompt_agent_match.group(2)
            node = {'type': 'PROMPT_AGENT', 'prompt': prompt}
            if variable:
                node['variable'] = variable
            self._current_block.append(node)
            return

        set_budget_match = _PATTERNS['SET_BUDGET'].match(line)
        if set_budget_match:
            variable = set_budget_match.group(1)
            value_str = set_budget_match.group(2)
            value = parse_value(value_str) # Use parse_value
            self._current_block.append({'type': 'SET_BUDGET', 'variable': variable, 'value': value})
            return

        request_review_match = _PATTERNS['REQUEST_REVIEW'].match(line)
        if request_review_match:
            message = request_review_match.group(1)
            self._current_block.append({'type': 'REQUEST_REVIEW', 'message': message})
            return

        pause_match = _PATTERNS['PAUSE'].match(line)
        if pause_match:
            message = pause_match.group(1)
            self._current_block.append({'type': 'PAUSE', 'message': message})
            return

        choose_match = _PATTERNS['CHOOSE'].match(line)
        if choose_match:
            choose_node = {'type': 'CHOOSE', 'options': []}
            self._current_block.append(choose_node)
            self._state_stack.append(self._state)
            self._block_stack.append(self._current_block)
            self._state = "IN_CHOOSE"
            self._current_block = choose_node['options']
            return

        raise TissLangParserError(f"Unexpected command inside STEP/SETUP block. Expected RUN, LOG, ASSERT, READ, WRITE, PROMPT_AGENT, SET_BUDGET, REQUEST_REVIEW, PAUSE, IF, CHOOSE, or ELSE.", self._line_number)

    def _handle_in_choose_state(self, line: str):
        """Handles parsing when inside a 'CHOOSE' block."""
        if _PATTERNS['BLOCK_END'].match(line):
            self._state = self._state_stack.pop()
            self._current_block = self._block_stack.pop()
            return

        option_match = _PATTERNS['OPTION'].match(line)
        if option_match:
            option_node = {'type': 'OPTION', 'description': option_match.group(1), 'commands': []}
            self._current_block.append(option_node)
            self._state_stack.append(self._state)
            self._block_stack.append(self._current_block)
            self._state = "IN_STEP"
            self._current_block = option_node['commands']
            return

        raise TissLangParserError(f"Unexpected command inside CHOOSE block. Expected OPTION or }.", self._line_number)

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
