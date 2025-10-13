from .matcher import _PATTERNS
from .errors import TissLangParserError

def handle_block_command(line: str, ast: list, current_block: list, state: str, line_number: int, command_type: str, pattern_key: str) -> tuple[bool, str, list]:
    """
    Handles parsing of generic block commands like PARALLEL, CHOOSE, ESTIMATE_COST.

    Returns:
        A tuple: (was_handled, new_state, new_current_block)
    """
    match = _PATTERNS[pattern_key].match(line)
    if match:
        node = {'type': command_type, 'commands': []}
        ast.append(node)
        current_block = node['commands']
        state = "IN_STEP"  # Reuse IN_STEP state for these blocks
        return True, state, current_block
    return False, state, current_block
