import re

class TissLangParser:
    def __init__(self):
        self.commands = []
        self.metadata = {}

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
    WRITE "main.py" <<PYTHON
import sys

def main():
    print("Hello, TissLang!")

if __name__ == "__main__":
    main()
PYTHON
}

STEP "Run the new file" {
    RUN "python main.py"
    ASSERT STDOUT CONTAINS "Hello, TissLang!"
}
"""

parser = TissLangParser()
parser.parse(tiss_code)
for cmd in parser.commands:
    print(cmd)
