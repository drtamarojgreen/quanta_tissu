import re


class TissLangLinter:
    """Handles syntax and semantic linting for TissLang code."""

    # Keywords that start a new indented block
    BLOCK_KEYWORDS = {"TASK", "STEP", "SETUP", "IF", "ELSE", "TRY", "CATCH", "CHOOSE", "PARALLEL", "ESTIMATE_COST"}
    # All valid keywords recognized by the language
    VALID_KEYWORDS = BLOCK_KEYWORDS.union({
        "WRITE", "RUN", "ASSERT", "READ", "PROMPT_AGENT", "SET_BUDGET", "REQUEST_REVIEW", "PAUSE",
        "AS", "INTO", "OPTION"
    })

    def lint(self, lines):
        """
        Analyzes a list of lines for syntax and semantic issues.
        Returns a dictionary of errors keyed by line number.
        """
        errors = {}
        indent_stack = []  # Stack to manage expected indentation levels

        for i, line in enumerate(lines):
            line_num = i + 1
            stripped_line = line.strip()
            current_indent = len(line) - len(line.lstrip(' '))

            if not stripped_line or stripped_line.startswith("#"):
                continue

            line_errors = []

            # --- Indentation and Block Management ---
            if indent_stack:
                # If the line is not empty, check its indentation level
                expected_indent = indent_stack[-1]
                if current_indent < expected_indent:
                    # Dedenting closes the current block
                    indent_stack.pop()
                    # Re-check against the new expected indent if stack is not empty
                    if indent_stack:
                        expected_indent = indent_stack[-1]
                        if current_indent != expected_indent:
                             line_errors.append(f"IndentationError: Unexpected dedent. Expected {expected_indent} spaces.")
                elif current_indent > expected_indent:
                    line_errors.append(f"IndentationError: Unexpected indent. Expected {expected_indent} spaces.")
                # If current_indent == expected_indent, it's correct.
            elif current_indent > 0:
                line_errors.append("IndentationError: Unexpected indent at root level.")

            # --- Keyword and Syntax Validation ---
            parts = stripped_line.split()
            command = parts[0]

            if command not in self.VALID_KEYWORDS and not command.endswith("{"):
                line_errors.append(f"SyntaxError: Unknown command '{command}'.")

            # Check for block-starting keywords
            if command in self.BLOCK_KEYWORDS or stripped_line.endswith("{"):
                # Expect the next line to be indented further
                indent_stack.append(current_indent + 4) # Assuming 4-space indents

            # Specific command syntax rules
            if command == "READ":
                if not re.match(r'READ\s+".*"\s+AS\s+\w+', stripped_line):
                    line_errors.append('SyntaxWarning: `READ` should be in the format: READ "filename" AS variable.')
            elif command == "PROMPT_AGENT":
                if not re.match(r'PROMPT_AGENT\s+".*"\s+INTO\s+\w+', stripped_line):
                    line_errors.append('SyntaxWarning: `PROMPT_AGENT` should be in the format: PROMPT_AGENT "prompt" INTO variable.')
            elif command == "SET_BUDGET":
                if not re.match(r'SET_BUDGET\s+\w+\s*=\s*.+', stripped_line):
                    line_errors.append('SyntaxWarning: `SET_BUDGET` should be in the format: SET_BUDGET KEY = VALUE.')
            elif command == "WRITE":
                 if not (len(parts) > 1 and (parts[1].startswith('"') or parts[1].startswith('<<'))):
                    line_errors.append("SyntaxWarning: WRITE command should be followed by a string or heredoc.")
            elif command == "ASSERT":
                if len(parts) < 2:
                    line_errors.append("SyntaxWarning: ASSERT command is missing an expression.")

            if line_errors:
                errors[line_num] = errors.get(line_num, []) + line_errors

        return errors
