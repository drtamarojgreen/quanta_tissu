import re

# ANSI escape codes and curses color codes
COLORS = {
    "default": (0, "\033[0m"),
    "keyword": (1, "\033[93m"),      # Yellow
    "string": (2, "\033[92m"),       # Green
    "comment": (3, "\033[90m"),      # Grey
    "directive": (4, "\033[95m"),    # Magenta
    "heredoc": (5, "\033[32m"),       # Green (same as string)
    "line_number": (6, "\033[90m"),  # Grey for line numbers
    "error": (7, "\033[91m"),        # Red for errors/warnings
    "special_var": (8, "\033[96m"),  # Cyan
    "operator": (9, "\033[91m"),     # Red
    "indent": (10, "\033[47;30m"),    # White background, black text
}

class TissLangHighlighter:
    """Handles syntax highlighting for TissLang code."""
    PATTERNS = {
        'comment': r'#.*',
        'string': r'"[^"]*"',
        'keyword': r'\b(TASK|STEP|WRITE|RUN|ASSERT|READ|PROMPT_AGENT|SET_BUDGET|REQUEST_REVIEW|PARALLEL|CHOOSE|ESTIMATE_COST|AS|INTO|IF|ELSE|TRY|CATCH|PAUSE)\b',
        'directive': r'@[a-zA-Z_]+',
        'special_var': r'\b(LAST_RUN)\b',
        'operator': r'\b(CONTAINS|IS_EMPTY|==)\b',
    }

    def __init__(self):
        # Build a single regex from all patterns
        self.token_regex = re.compile('|'.join(f'(?P<{name}>{pattern})' for name, pattern in self.PATTERNS.items()))

    def highlight(self, text):
        """
        Generates a list of (token_type, token_string) tuples for a line of code.
        """
        tokens = []
        last_end = 0
        for match in self.token_regex.finditer(text):
            start = match.start()
            end = match.end()
            if start > last_end:
                tokens.append(('default', text[last_end:start]))

            token_type = match.lastgroup
            tokens.append((token_type, match.group(token_type)))
            last_end = end

        if last_end < len(text):
            tokens.append(('default', text[last_end:]))

        return tokens

    def get_color_code(self, token_type):
        """
        Returns the curses color code for a given token type.
        """
        return COLORS.get(token_type, COLORS['default'])[0]
