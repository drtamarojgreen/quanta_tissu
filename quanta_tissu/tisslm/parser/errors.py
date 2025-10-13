class TissLangParserError(Exception):
    """Custom exception for parsing errors in TissLang."""

    def __init__(self, message: str, line_number: int = -1, line: str = ""):
        super().__init__(message)
        self.line_number = line_number
        self.line = line

    def __str__(self) -> str:
        if self.line_number != -1:
            return f"Error at line {self.line_number}: {super().__str__()}\n  > {self.line}"
        return super().__str__()

class TissError(Exception):
    """Base exception for all TissLM errors."""
    pass