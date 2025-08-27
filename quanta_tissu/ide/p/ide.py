import sys
import re
import os

# ANSI escape codes for colors
COLORS = {
    "default": "\033[0m",
    "keyword": "\033[93m",  # Yellow
    "string": "\033[92m",   # Green
    "comment": "\033[90m",  # Grey
    "directive": "\033[95m", # Magenta
    "heredoc": "\033[32m",   # Green (same as string)
}

# Regex patterns for syntax highlighting
PATTERNS = {
    'keyword': r'\b(TASK|STEP|WRITE|RUN|ASSERT|READ|PROMPT_AGENT|SET_BUDGET|REQUEST_REVIEW|PARALLEL|CHOOSE|ESTIMATE_COST|AS|INTO)\b',
    'string': r'"[^"]*"',
    'comment': r'#.*$',
    'directive': r'@[a-zA-Z_]+',
}

def highlight(text):
    """
    Applies syntax highlighting to a line of TissLang code.
    This version is simplified and processes tokens sequentially.
    A more robust version would build a token list and sort by position.
    """
    # Create a list of all found tokens with their spans and types
    tokens = []
    for token_type, pattern in PATTERNS.items():
        for match in re.finditer(pattern, text):
            tokens.append((match.start(), match.end(), token_type))

    # Sort tokens by their starting position
    tokens.sort()

    last_index = 0
    highlighted_line = ""
    for start, end, token_type in tokens:
        # Add the text between the last token and this one
        highlighted_line += text[last_index:start]
        # Add the highlighted token
        highlighted_line += f"{COLORS[token_type]}{text[start:end]}{COLORS['default']}"
        last_index = end

    # Add any remaining text after the last token
    highlighted_line += text[last_index:]
    return highlighted_line

def main():
    """
    Main function for the TissLang CLI tool.
    Reads a .tiss file and prints it with syntax highlighting.
    """
    if len(sys.argv) != 2:
        print("Usage: python ide.py <path/to/file.tiss>")
        sys.exit(1)

    filepath = sys.argv[1]
    if not os.path.exists(filepath):
        print(f"Error: File not found at '{filepath}'")
        sys.exit(1)

    if not filepath.endswith(".tiss"):
        print("Warning: File does not have a .tiss extension.")

    print(f"--- Displaying: {filepath} ---\n")

    try:
        with open(filepath, "r", encoding="utf-8") as f:
            in_heredoc = False
            heredoc_delimiter = None
            for line in f:
                stripped_line = line.strip()
                if in_heredoc:
                    # Print heredoc content in green
                    sys.stdout.write(f"{COLORS['heredoc']}{line}{COLORS['default']}")
                    if stripped_line == heredoc_delimiter:
                        in_heredoc = False
                        heredoc_delimiter = None
                else:
                    # Check for start of a heredoc block
                    heredoc_match = re.search(r'<<(\w+)', stripped_line)
                    if heredoc_match:
                        in_heredoc = True
                        heredoc_delimiter = heredoc_match.group(1)

                    # Apply highlighting to the line
                    highlighted_line = highlight(line)
                    sys.stdout.write(highlighted_line)

    except Exception as e:
        print(f"\nAn error occurred while reading the file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
