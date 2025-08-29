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
    "indent": "\033[47;30m",    # White background, black text
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
        """
        # Handle indentation first
        leading_whitespace_match = re.match(r'^(\s+)', text)
        if leading_whitespace_match:
            indent_text = leading_whitespace_match.group(1)
            highlighted_indent = f'{COLORS["indent"]}{indent_text}{COLORS["default"]}'
            text_to_process = text[len(indent_text):]
        else:
            highlighted_indent = ""
            text_to_process = text

        if not text_to_process:
            return highlighted_indent

        tokens = []
        for token_type, pattern in self.PATTERNS.items():
            for match in re.finditer(pattern, text_to_process):
                tokens.append({'start': match.start(), 'end': match.end(), 'type': token_type})

        if not tokens:
            return highlighted_indent + text_to_process

        tokens.sort(key=lambda t: t['start'])

        final_tokens = []
        last_end = -1
        for token in tokens:
            if token['start'] >= last_end:
                final_tokens.append(token)
                last_end = token['end']

        last_index = 0
        highlighted_line = ""
        for token in final_tokens:
            highlighted_line += text_to_process[last_index:token['start']]
            token_type = token['type']
            token_text = text_to_process[token['start']:token['end']]
            highlighted_line += f'{COLORS.get(token_type, COLORS["default"])}{token_text}{COLORS["default"]}'
            last_index = token['end']

        highlighted_line += text_to_process[last_index:]
        return highlighted_indent + highlighted_line
