import re
import json
from typing import List, Dict, Any, Optional

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

    # Regex patterns for matching TissLang commands
    _PATTERNS = {
        'TASK': re.compile(r'^\s*TASK\s+"([^"]+)"\s*$'),
        'STEP': re.compile(r'^\s*STEP\s+"([^"]+)"\s*\{\s*$'),
        'SETUP': re.compile(r'^\s*SETUP\s*\{\s*$'),
        'BLOCK_END': re.compile(r'^\s*\}\s*$'),
        'RUN': re.compile(r'^\s*RUN\s+"([^"]+)"\s*$'),
        'WRITE': re.compile(r'^\s*WRITE\s+"([^"]+)"\s+<<(\w+)\s*$'),
        'ASSERT': re.compile(r'^\s*ASSERT\s+(.+?)\s*$'),
        'READ': re.compile(r'^\s*READ\s+"([^"]+)"\s+AS\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*$'),
        'COMMENT': re.compile(r'^\s*#.*$'),
        'EMPTY': re.compile(r'^\s*$'),
    }

    def __init__(self):
        self.ast: List[Dict[str, Any]] = []
        self._state: str = "IDLE"  # Can be IDLE, IN_STEP, IN_WRITE
        self._current_block: Optional[List[Dict[str, Any]]] = None
        self._heredoc_delimiter: Optional[str] = None
        self._heredoc_content: List[str] = []
        self._line_number: int = 0


    def parse(self, tiss_code):
        lines = tiss_code.strip().splitlines()
        i = 0
        while i < len(lines):
            line = lines[i].strip()

            if not line:
                i += 1
                continue

            if line.startswith("#TISS!"):
                i += 1
                continue
            elif line.startswith("Language="):
                key, value = line.split("=", 1)
                self.metadata[key.strip()] = value.strip()
                i += 1
            elif self._is_setup(line):
                command, i = self._parse_setup(lines, i)
                self.commands.append(command)
            elif self._is_step(line):
                command, i = self._parse_step(lines, i)
                self.commands.append(command)
            elif self._is_write(line):
                command, i = self._parse_write(lines, i)
                if command:
                    self.commands.append(command)
            elif self._is_conditional(line):
                conditional = self._parse_conditional(line)
                if conditional:
                    self.commands.append(conditional)
                i += 1
            elif self._is_query(line):
                self.commands.append({"type": "query", "content": self._parse_query(line)})
                i += 1
            elif self._is_task(line):
                self.commands.append(self._parse_task(line))
                i += 1
            elif self._is_run(line):
                self.commands.append(self._parse_run(line))
                i += 1
            elif self._is_read(line):
                self.commands.append(self._parse_read(line))
                i += 1
            elif self._is_assert(line):
                self.commands.append(self._parse_assert(line))
                i += 1
            elif self._is_greeting(line):
                self.commands.append({"type": "greeting", "content": line})
                i += 1
            else:
                self.commands.append({"type": "text", "content": line})
                i += 1

    def _is_query(self, line):
        return line.startswith("Query=") and line.endswith("ENDQuery")

    def _parse_query(self, line):
        return line[len("Query="): -len("ENDQuery")].strip()

    def _is_greeting(self, line):
        greetings = ["hi", "hello", "gm", "good morning", "hey", "greetings"]
        return any(greet in line.lower() for greet in greetings)

    def _is_conditional(self, line):
        return line.startswith("W{") and "}T{" in line

    def _parse_conditional(self, line):
        pattern = r'W\{([^}]*)\}T\{([^}]*)\}(?:E\{([^}]*)\})?'
        match = re.search(pattern, line)
        if not match:
            return None
        condition, true_branch, else_branch = match.groups()
        return {
            "type": "conditional",
            "condition": condition.strip(),
            "true_branch": true_branch.strip(),
            "else_branch": else_branch.strip() if else_branch else None
        }

    def _is_task(self, line):
        return line.startswith("TASK ")

    def _parse_task(self, line):
        match = re.search(r'TASK "([^"]*)"', line)
        return {"type": "task", "description": match.group(1) if match else ""}

    def _is_run(self, line):
        return line.startswith("RUN ")

    def _parse_run(self, line):
        match = re.search(r'RUN "([^"]*)"', line)
        return {"type": "run", "command": match.group(1) if match else ""}

    def _is_read(self, line):
        return line.startswith("READ ")

    def _parse_read(self, line):
        match = re.search(r'READ "([^"]*)" AS \[([^\]]*)\]', line)
        if not match:
            return None
        return {"type": "read", "path": match.group(1), "variable": match.group(2)}

    def _is_assert(self, line):
        return line.startswith("ASSERT ")

    def _parse_assert(self, line):
        return {"type": "assert", "condition": line[len("ASSERT "):].strip()}

    def _is_setup(self, line):
        return line.strip() == "SETUP {"

    def _parse_setup(self, lines, i):
        block_lines = []
        i += 1  # Move past "SETUP {"
        while i < len(lines) and lines[i].strip() != "}":
            block_lines.append(lines[i])
            i += 1
        if i < len(lines):
            i += 1  # Move past "}"

        # Recursively parse the block content
        block_parser = TissLangParser()
        block_parser.parse("\n".join(block_lines))

        return {"type": "setup", "commands": block_parser.commands}, i

    def _is_step(self, line):
        return line.startswith("STEP ") and line.strip().endswith(" {")

    def _parse_step(self, lines, i):
        first_line = lines[i].strip()
        match = re.search(r'STEP "([^"]*)"', first_line)
        description = match.group(1) if match else ""

        block_lines = []
        i += 1  # Move past the STEP line
        while i < len(lines) and lines[i].strip() != "}":
            block_lines.append(lines[i])
            i += 1
        if i < len(lines):
            i += 1  # Move past "}"

        # Recursively parse the block content
        block_parser = TissLangParser()
        block_parser.parse("\n".join(block_lines))

        return {
            "type": "step",
            "description": description,
            "commands": block_parser.commands
        }, i

    def _is_write(self, line):
        return line.startswith("WRITE ") and "<<" in line

    def _parse_write(self, lines, i):
        first_line = lines[i].strip()
        match = re.search(r'WRITE "([^"]*)" <<(\S+)', first_line)
        if not match:
            return None, i + 1

        path, delimiter = match.groups()

        content_lines = []
        i += 1  # Move to the line after the WRITE statement
        while i < len(lines) and lines[i].strip() != delimiter:
            content_lines.append(lines[i])
            i += 1

        if i < len(lines):  # Found the closing delimiter
            i += 1  # Consume the delimiter line

        return {
            "type": "write",
            "path": path,
            "delimiter": delimiter,
            "content": "\n".join(content_lines)
        }, i


# Example usage:
tiss_code = """
#TISS!
Language=Py
TASK "Setup a new project and run initial tests"

SETUP {
    pip install -r requirements.txt
}

STEP "Read the project configuration" {
    READ "project.conf" AS [config]
}

STEP "Create a main file" {

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
        if self._PATTERNS['COMMENT'].match(line) or self._PATTERNS['EMPTY'].match(line):
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
        task_match = self._PATTERNS['TASK'].match(line)
        if task_match:
            self.ast.append({'type': 'TASK', 'description': task_match.group(1)})
            return

        step_match = self._PATTERNS['STEP'].match(line)
        if step_match:
            step_node = {'type': 'STEP', 'description': step_match.group(1), 'commands': []}
            self.ast.append(step_node)
            self._current_block = step_node['commands']
            self._state = "IN_STEP"
            return

        setup_match = self._PATTERNS['SETUP'].match(line)
        if setup_match:
            setup_node = {'type': 'SETUP', 'commands': []}
            self.ast.append(setup_node)
            self._current_block = setup_node['commands']
            # We can reuse the IN_STEP state as the allowed commands are identical
            self._state = "IN_STEP"
            return

        raise TissLangParserError(f"Unexpected command. Expected TASK, STEP, or SETUP.", self._line_number)

    def _handle_in_step_state(self, line: str):
        """Handles parsing when inside a 'STEP' or 'SETUP' block."""
        if self._PATTERNS['BLOCK_END'].match(line):
            self._state = "IDLE"
            self._current_block = None
            return

        run_match = self._PATTERNS['RUN'].match(line)
        if run_match:
            self._current_block.append({'type': 'RUN', 'command': run_match.group(1)})
            return

        assert_match = self._PATTERNS['ASSERT'].match(line)
        if assert_match:
            self._current_block.append({'type': 'ASSERT', 'condition': assert_match.group(1)})
            return

        read_match = self._PATTERNS['READ'].match(line)
        if read_match:
            self._current_block.append({'type': 'READ', 'path': read_match.group(1), 'variable': read_match.group(2)})
            return

        write_match = self._PATTERNS['WRITE'].match(line)
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
            # Join all collected lines and remove the final trailing newline
            write_node['content'] = "".join(self._heredoc_content).rstrip('\n')

            # Reset state
            self._heredoc_delimiter = None
            self._heredoc_content = []
            self._state = "IN_STEP"
        else:
            self._heredoc_content.append(line + '\n')


if __name__ == '__main__':
    # Example usage: Parse the script from the TissLang plan.
    example_script = """
#TISS! Language=Python

TASK "Create and test a simple Python hello world script"

STEP "Create the main application file" {

    WRITE "main.py" <<PYTHON
import sys

def main():
    print(f"Hello, {sys.argv[1]}!")


if __name__ == "__main__":
    main()
PYTHON
}


STEP "Run the script and verify its output" {
    RUN "python main.py TissLang"
    ASSERT LAST_RUN.EXIT_CODE == 0
    ASSERT LAST_RUN.STDOUT CONTAINS "Hello, TissLang!"
    READ "main.py" AS main_code

"""

    print("--- Parsing TissLang Script ---")
    parser = TissLangParser()
    try:
        parsed_ast = parser.parse(example_script)
        print("--- Parse Successful. AST: ---")
        print(json.dumps(parsed_ast, indent=2))
    except TissLangParserError as e:
        print(f"Parsing failed: {e}")