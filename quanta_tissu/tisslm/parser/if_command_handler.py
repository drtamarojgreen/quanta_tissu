import re
from typing import Optional
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


def handle_if_command(line: str, current_block: list, line_number: int) -> Optional[dict]:
    """
    Checks if a line is an IF command and returns the parsed node if it is.
    Does not handle state changes.
    """
    if_match = _PATTERNS['IF'].match(line)
    if if_match:
        expression_str = if_match.group(1)
        expression_node = parse_expression(expression_str, line_number)
        if_node = {'type': 'IF', 'condition': expression_node, 'then_block': [], 'else_block': None}
        current_block.append(if_node)
        return if_node
    return None


def handle_else_command(line: str, current_block: list, line_number: int) -> Optional[dict]:
    """
    Checks if a line is an ELSE command and returns the corresponding IF node if it is.
    """
    else_match = _PATTERNS['ELSE'].match(line)
    if else_match:
        # Find the last IF node in the current block that doesn't have an else_block yet
        last_if_node = None
        for node in reversed(current_block):
            if node.get('type') == 'IF' and node.get('else_block') is None:
                last_if_node = node
                break

        if last_if_node is None:
            raise TissLangParserError("ELSE without a preceding IF.", line_number)

        last_if_node['else_block'] = []
        return last_if_node
    return None
