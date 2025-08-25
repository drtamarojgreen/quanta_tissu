from .matcher import _PATTERNS

def handle_setup_command(line: str, ast: list, state_stack: list, block_stack: list, line_number: int) -> bool:
    """
    Handles parsing of a SETUP command, modifying stacks directly.

    Returns:
        A boolean indicating if the command was handled.
    """
    setup_match = _PATTERNS['SETUP'].match(line)
    if setup_match:
        setup_node = {'type': 'SETUP', 'commands': []}
        # A SETUP block is always at the top level
        ast.append(setup_node)

        state_stack.append("IN_STEP")
        block_stack.append(setup_node['commands'])
        return True
    return False
