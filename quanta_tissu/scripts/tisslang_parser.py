import re

class TissLangParser:
    def __init__(self):
        self.commands = []
        self.metadata = {}

    def parse(self, tiss_code):
        lines = tiss_code.strip().splitlines()
        for line in lines:
            line = line.strip()
            if not line:
                continue
            if line.startswith("#TISS!"):
                continue  # shebang marker
            elif line.startswith("Language="):
                key, value = line.split("=", 1)
                self.metadata[key.strip()] = value.strip()
            elif self._is_conditional(line):
                conditional = self._parse_conditional(line)
                if conditional:
                    self.commands.append({"type": "conditional", **conditional})
            elif self._is_query(line):
                self.commands.append({"type": "query", "content": self._parse_query(line)})
            elif self._is_greeting(line):
                self.commands.append({"type": "greeting", "content": line})
            else:
                self.commands.append({"type": "text", "content": line})

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
            "condition": condition.strip(),
            "true_branch": true_branch.strip(),
            "else_branch": else_branch.strip() if else_branch else None
        }


# Example usage:
tiss_code = """
#TISS!
GM CG! U today, how are?
Language=Py
Query=Read text file into a listENDQuery
W{Language=Py}T{Generate Fibonacci code}E{Ask for supported language}
"""

parser = TissLangParser()
parser.parse(tiss_code)
for cmd in parser.commands:
    print(cmd)
