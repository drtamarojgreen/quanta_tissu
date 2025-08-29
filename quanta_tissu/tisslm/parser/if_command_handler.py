import re
from .matcher import _PATTERNS
from .errors import TissLangParserError
from .value_parser import parse_value

def parse_expression(expression_str: str, line_number: int) -> dict:
    """Parses a simple TissLang expression."""
    boolean_match = _PATTERNS['BOOLEAN'].match(expression_str.strip())
    if boolean_match:
        return {'type': 'BOOLEAN', 'value': boolean_match.group(1) == 'true'}

    # For now, assume simple comparison: identifier OPERATOR value
    match = re.match(r'^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*(==|!=|<|>|<=|>=)\s*(.+?)\s*$', expression_str)
    if match:
        left = match.group(1)
        operator = match.group(2)
        right_str = match.group(3)
        right = parse_value(right_str) # Use parse_value
        return {'type': 'COMPARISON', 'left': left, 'operator': operator, 'right': right}

    raise TissLangParserError(f"Invalid expression: '{expression_str}'", line_number)


def handle_if_command(line: str, ast: list, current_block: list, state: str, line_number: int) -> tuple[bool, str, list]:
    """
    Handles parsing of IF and ELSE commands.

    Returns:
        A tuple: (was_handled, new_state, new_current_block)
    """
    if_match = _PATTERNS['IF'].match(line)
    if if_match:
        expression_str = if_match.group(1)
        expression_node = parse_expression(expression_str, line_number)
        if_node = {'type': 'IF', 'condition': expression_node, 'then_block': [], 'else_block': None}
        ast.append(if_node)
        current_block = if_node['then_block']
        state = "IN_STEP" # Reuse IN_STEP state for THEN block
        return True, state, current_block

    else_match = _PATTERNS['ELSE'].match(line)
    if else_match:
        # Ensure the previous node was an IF with a then_block
        if not ast or ast[-1]['type'] != 'IF' or ast[-1]['else_block'] is not None:
            raise TissLangParserError("ELSE without a preceding IF or ELSE already defined.", line_number)

        if_node = ast[-1]
        if_node['else_block'] = []
        current_block = if_node['else_block']
        state = "IN_STEP" # Reuse IN_STEP state for ELSE block
        return True, state, current_block

    return False, state, current_block
