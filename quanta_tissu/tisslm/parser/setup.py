from .matcher import _PATTERNS
from .errors import TissLangParserError

def handle_setup_command(line: str, ast: list, line_number: int):
    """
    Handles parsing of a SETUP command.
    """
    setup_match = _PATTERNS['SETUP'].match(line)
    if setup_match:
        setup_node = {'type': 'SETUP', 'description': setup_match.group(1), 'commands': []}
        ast.append(setup_node)
        return setup_node
    return None
