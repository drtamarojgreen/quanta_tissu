from .matcher import _PATTERNS

def handle_step_command(line: str, ast: list, state_stack: list, block_stack: list, line_number: int) -> bool:
    """
    Handles parsing of a STEP command, modifying stacks directly.

    Returns:
        A boolean indicating if the command was handled.
    """
    step_match = _PATTERNS['STEP'].match(line)
    if step_match:
        step_node = {'type': 'STEP', 'description': step_match.group(1), 'commands': []}
        # A STEP block is always at the top level
        ast.append(step_node)

        state_stack.append("IN_STEP")
        block_stack.append(step_node['commands'])
        return True
    return False
