def register_steps(runner):
    @runner.step(r'Then the parser should produce an Abstract Syntax Tree \(AST\)')
    def then_the_parser_should_produce_ast(context):
        assert context.get('ast') is not None, "Parser did not produce an AST."
        assert isinstance(context['ast'], list), "AST is not a list."

    @runner.step(r'And the AST should contain a (\w+) node at index (\d+)')
    def and_the_ast_should_contain_node_at_index(context, node_type, index):
        index = int(index)
        assert len(context['ast']) > index, f"AST does not have an element at index {index}."
        node = context['ast'][index]
        assert node.get('type') == node_type, f"Node at index {index} is not of type {node_type}."
        context['current_node'] = node

    @runner.step(r'And the (\w+) node should contain (\d+) commands')
    def and_the_node_should_contain_n_commands(context, node_type, num_commands):
        assert context.get('current_node') is not None, "No current node in context."
        node = context['current_node']
        assert node.get('type') == node_type, f"Current node is not of type {node_type}."
        commands = node.get('commands', [])
        assert len(commands) == int(num_commands), f"Expected {num_commands} commands, but found {len(commands)}."
        context['current_commands'] = commands

    @runner.step(r'And the command at index (\d+) should be of type (\w+)')
    def and_the_command_at_index_should_be_of_type(context, index, command_type):
        index = int(index)
        commands = context.get('current_commands', [])
        assert len(commands) > index, f"No command at index {index}."
        command = commands[index]
        assert command.get('type') == command_type, f"Command at index {index} is not of type {command_type}."
        context['current_command'] = command

    @runner.step(r'And the RUN command should have command "(.*)"')
    def and_the_run_command_should_have_command(context, command_text):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'RUN', "Current command is not of type RUN."
        assert command.get('command') == command_text, f"Expected command to be '{command_text}', but got '{command.get('command')}'."

    @runner.step(r'And the STEP node should have a description "(.*)"')
    def and_the_step_node_should_have_a_description(context, description):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'STEP', "Current command is not of type STEP."
        assert command.get('description') == description, f"Expected description to be '{description}', but got '{command.get('description')}'."

    @runner.step(r'And the PROMPT_AGENT command should have prompt "(.*)"')
    def and_the_prompt_agent_command_should_have_prompt(context, prompt):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'PROMPT_AGENT', "Current command is not of type PROMPT_AGENT."
        assert command.get('prompt') == prompt, f"Expected prompt to be '{prompt}', but got '{command.get('prompt')}'."
