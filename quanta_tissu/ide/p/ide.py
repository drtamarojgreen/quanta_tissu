import sys
import re
import os
from .highlighter import TissLangHighlighter, COLORS
from .linter import TissLangLinter
from .editor import Editor


class TissLangIDE:
    """
    A simple IDE for the TissLang language.
    Provides syntax highlighting and linting.
    """
    def __init__(self):
        self.highlighter = TissLangHighlighter()
        self.linter = TissLangLinter()
        self.editor = Editor()

    def run(self, filepath):
        """
        Reads and displays a .tiss file with highlighting and linting.
        """
        if not os.path.exists(filepath):
            print(f"Error: File not found at '{filepath}'")
            sys.exit(1)

        if not filepath.endswith(".tiss"):
            print("Warning: File does not have a .tiss extension.")

        print(f"--- Displaying: {filepath} ---
")

        try:
            with open(filepath, "r", encoding="utf-8") as f:
                lines = f.readlines()

            self._process_and_display(lines)

        except Exception as e:
            print(f"\nAn error occurred while reading the file: {e}")
            sys.exit(1)

    def _process_and_display(self, lines):
        """
        Processes and displays the lines of code with highlighting and linting.
        """
        max_line_num_width = len(str(len(lines)))
        in_heredoc = False
        heredoc_delimiter = None
        all_errors = self.linter.lint(lines) # Get all errors at once

        for i, line in enumerate(lines):
            line_num = i + 1
            line_num_str = f" {str(line_num).rjust(max_line_num_width)} | "
            sys.stdout.write(f"{COLORS['line_number']}{line_num_str}{COLORS['default']}")

            proc_line = line.rstrip('\n')
            stripped_line = proc_line.strip()

            if in_heredoc:
                sys.stdout.write(f"{COLORS['heredoc']}{proc_line}{COLORS['default']}\n")
                if stripped_line == heredoc_delimiter:
                    in_heredoc = False
                    heredoc_delimiter = None
            else:
                heredoc_match = re.search(r'<<(\w+)', stripped_line)
                if heredoc_match:
                    in_heredoc = True
                    heredoc_delimiter = heredoc_match.group(1)

                highlighted_line = self.highlighter.highlight(proc_line)
                sys.stdout.write(highlighted_line + "\n")

                if line_num in all_errors:
                    padding = " " * (max_line_num_width + 4)
                    for error in all_errors[line_num]:
                        sys.stdout.write(f"{padding}{COLORS['error']}└─> {error}{COLORS['default']}\n")

    def search_and_replace_in_file(self, filepath, regex, replacement):
        """
        Performs search and replace on a file.
        """
        try:
            with open(filepath, "r", encoding="utf-8") as f:
                content = f.read()

            new_content = self.editor.search_and_replace(content, regex, replacement)

            with open(filepath, "w", encoding="utf-8") as f:
                f.write(new_content)

            print(f"Successfully replaced pattern '{regex}' with '{replacement}' in {filepath}")

        except (IOError) as e:
            print(f"Error during search and replace: {e}")
            sys.exit(1)


def main():
    """
    Main function for the TissLang CLI tool.
    """
    if len(sys.argv) < 2:
        print("Usage: python -m quanta_tissu.ide.p.ide <path/to/file.tiss> [--replace <regex> <replacement>]")
        sys.exit(1)

    filepath = sys.argv[1]
    ide = TissLangIDE()

    if "--replace" in sys.argv:
        try:
            replace_index = sys.argv.index("--replace")
            regex = sys.argv[replace_index + 1]
            replacement = sys.argv[replace_index + 2]
            ide.search_and_replace_in_file(filepath, regex, replacement)

        except IndexError:
            print("Error: --replace requires two arguments: <regex> and <replacement>")
            print("Usage: ... --replace <regex> <replacement>")
            sys.exit(1)

    ide.run(filepath)


if __name__ == "__main__":
    main()

