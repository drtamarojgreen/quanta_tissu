from .matcher import _PATTERNS
from .errors import TissLangParserError

def handle_setup_command(line: str, ast: list, current_block: list, state: str, line_number: int) -> tuple[bool, str, list]:
    """
    Handles parsing of a SETUP command.

    Returns:
        A tuple: (was_handled, new_state, new_current_block)
    """
    setup_match = _PATTERNS['SETUP'].match(line)
    if setup_match:
        setup_node = {'type': 'SETUP', 'description': setup_match.group(1), 'commands': []}
        ast.append(setup_node)
        current_block = setup_node['commands']
        # We can reuse the IN_STEP state as the allowed commands are identical
        state = "IN_STEP"
        return True, state, current_block
    return False, state, current_block
