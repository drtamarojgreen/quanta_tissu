def register_steps(runner):
    # Most steps are already registered in test_parser_steps.py.
    # Only block-specific steps or overrides should go here.

    @runner.step(r'And the (CHOOSE|ESTIMATE_COST) node should contain (\d+) commands')
    def and_the_block_node_should_contain_n_commands(context, node_type, num_commands):
        assert context.get('current_node') is not None, "No current node in context."
        node = context['current_node']
        assert node.get('type') == node_type, f"Current node is not of type {node_type}."
        commands = node.get('commands', [])
        assert len(commands) == int(num_commands), f"Expected {num_commands} commands, but found {len(commands)}."
        context['current_commands'] = commands
