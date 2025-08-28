class TissLangLinter:
    """Handles syntax and semantic linting for TissLang code."""

    def lint(self, lines):
        """
        Analyzes a list of lines for syntax and semantic issues.
        Returns a dictionary of errors keyed by line number.
        """
        errors = {}
        in_task_block = False
        task_indent_level = -1

        for i, line in enumerate(lines):
            line_num = i + 1
            stripped_line = line.strip()
            current_indent = len(line) - len(line.lstrip(' '))

            if not stripped_line or stripped_line.startswith("#"):
                continue

            line_errors = []

            # Rule: Check indentation and block context
            if stripped_line.startswith("TASK"):
                in_task_block = True
                task_indent_level = current_indent
            # A line with less or equal indentation than the TASK line breaks the block
            elif in_task_block and current_indent <= task_indent_level:
                in_task_block = False
                task_indent_level = -1

            # Rule: STEP should be inside a TASK
            if stripped_line.startswith("STEP") and not in_task_block:
                line_errors.append("Warning: STEP command should be inside a TASK block.")

            # Rule: WRITE should be followed by a string or heredoc
            if stripped_line.startswith("WRITE"):
                rest_of_line = stripped_line[5:].lstrip()
                if not (rest_of_line.startswith('"') or rest_of_line.startswith('<<')):
                    line_errors.append("Warning: WRITE command should be followed by a string or heredoc.")

            # Rule: ASSERT should have an expression
            if stripped_line.startswith("ASSERT") and len(stripped_line.split()) < 2:
                line_errors.append("Warning: ASSERT command is missing an expression.")

            if line_errors:
                errors[line_num] = line_errors

        return errors
