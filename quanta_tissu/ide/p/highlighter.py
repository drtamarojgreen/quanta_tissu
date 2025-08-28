import re

# ANSI escape codes for colors
COLORS = {
    "default": "\033[0m",
    "keyword": "\033[93m",      # Yellow
    "string": "\033[92m",       # Green
    "comment": "\033[90m",      # Grey
    "directive": "\033[95m",    # Magenta
    "heredoc": "\033[32m",       # Green (same as string)
    "line_number": "\033[90m",  # Grey for line numbers
    "error": "\033[91m",        # Red for errors/warnings
    "special_var": "\033[96m",  # Cyan
    "operator": "\033[91m",     # Red
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

    def highlight(self, text):
        """
        Applies syntax highlighting to a line of TissLang code.
        This version correctly handles overlapping tokens by giving
        comments and strings priority over other tokens.
        """
        tokens = []
        for token_type, pattern in self.PATTERNS.items():
            for match in re.finditer(pattern, text):
                tokens.append({'start': match.start(), 'end': match.end(), 'type': token_type})

        if not tokens:
            return text

        # Sort tokens by their starting position
        tokens.sort(key=lambda t: t['start'])

        # Filter out overlapping tokens. The first token in a region "wins".
        final_tokens = []
        last_end = -1
        for token in tokens:
            if token['start'] >= last_end:
                final_tokens.append(token)
                last_end = token['end']

        last_index = 0
        highlighted_line = ""
        for token in final_tokens:
            # Add the text between the last token and this one
            highlighted_line += text[last_index:token['start']]
            # Add the highlighted token
            token_type = token['type']
            token_text = text[token['start']:token['end']]
            highlighted_line += f'{COLORS.get(token_type, COLORS["default"])}{token_text}{COLORS["default"]}'
            last_index = token['end']

        # Add any remaining text after the last token
        highlighted_line += text[last_index:]
        return highlighted_line
