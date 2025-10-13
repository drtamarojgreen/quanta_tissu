from .matcher import _PATTERNS
from .errors import TissLangParserError

def handle_step_command(line: str, ast: list, line_number: int):
    """
    Handles parsing of a STEP command.
    """
    step_match = _PATTERNS['STEP'].match(line)
    if step_match:
        step_node = {'type': 'STEP', 'description': step_match.group(1), 'commands': []}
        ast.append(step_node)
        return step_node
    return None
