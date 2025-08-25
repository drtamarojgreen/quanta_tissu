from matcher import _PATTERNS
from tisslang_parser import TissLangParserError

def handle_step_command(line: str, ast: list, current_block: list, state: str, line_number: int) -> tuple[bool, str, list]:
    """
    Handles parsing of a STEP command.

    Returns:
        A tuple: (was_handled, new_state, new_current_block)
    """
    step_match = _PATTERNS['STEP'].match(line)
    if step_match:
        step_node = {'type': 'STEP', 'description': step_match.group(1), 'commands': []}
        ast.append(step_node)
        current_block = step_node['commands']
        state = "IN_STEP"
        return True, state, current_block
    return False, state, current_block
