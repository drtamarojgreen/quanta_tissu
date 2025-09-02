import json
from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser, TissLangParserError

def register_steps(runner):
    @runner.step(r'Given I have a TissLang script with content:(.*)')
    def given_a_tisslang_script_with_content(context, script_content):
        # This handles scripts provided on a single line, often with quotes
        context['script'] = script_content.strip().strip("'\"")

    @runner.step(r'Given a TissLang script:\s*"""\s*([\s\S]*?)"""')
    def given_a_tisslang_script(context, script):
        context['script'] = script.strip()

    @runner.step(r'When I parse the script')
    def when_i_parse_the_script(context):
        parser = TissLangParser()
        try:
            context['ast'] = parser.parse(context['script'])
            context['error'] = None
        except TissLangParserError as e:
            context['ast'] = None
            context['error'] = e

    @runner.step(r'Then the AST should be:\s*"""\s*([\s\S]*?)\s*"""')
    def then_the_ast_should_be(context, expected_ast_json):
        expected_ast = json.loads(expected_ast_json)
        assert context['ast'] is not None, "AST is None, an error probably occurred"
        assert context['ast'] == expected_ast, f"AST mismatch:\nExpected: {expected_ast}\nGot: {context['ast']}"

    @runner.step(r'Then the AST should have (\d+) top-level nodes')
    def then_the_ast_should_have_n_nodes(context, num_nodes):
        assert context['ast'] is not None, "AST is None, an error probably occurred"
        assert len(context['ast']) == int(num_nodes), f"Expected {num_nodes} top-level nodes, but got {len(context['ast'])}"

    @runner.step(r'Then parsing should fail with an error containing "(.*)"')
    def then_parsing_should_fail(context, error_message):
        assert context['error'] is not None, "Expected a parsing error, but none occurred"
        assert error_message in str(context['error']), f"Expected error message to contain '{error_message}', but got '{context['error']}'"

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

    @runner.step(r'And the AST should contain a directive named "(.*)" with value "(.*)"')
    def and_the_ast_should_contain_directive(context, name, value):
        assert context.get('ast') is not None, "AST is not in context."
        for node in context['ast']:
            if node.get('type') == 'DIRECTIVE' and node.get('name') == name and node.get('value') == value:
                return
        raise AssertionError(f"Directive '{name}' with value '{value}' not found in AST.")

    @runner.step(r'And the DIRECTIVE command should have name "(.*)"')
    def and_the_directive_command_should_have_name(context, name):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'DIRECTIVE', "Current command is not of type DIRECTIVE."
        assert command.get('name') == name, f"Expected directive name to be '{name}', but got '{command.get('name')}'."

    @runner.step(r'And the DIRECTIVE command should have value "(.*)"')
    def and_the_directive_command_should_have_value(context, value):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'DIRECTIVE', "Current command is not of type DIRECTIVE."
        assert command.get('value') == value, f"Expected directive value to be '{value}', but got '{command.get('value')}'."

    @runner.step(r'And the IF node should have a condition of type (\w+)')
    def and_the_if_node_should_have_condition_of_type(context, condition_type):
        assert context.get('current_node') is not None, "No current node in context."
        node = context['current_node']
        assert node.get('type') == 'IF', "Current node is not of type IF."
        condition = node.get('condition', {})
        assert condition.get('type') == condition_type, f"Expected condition type to be '{condition_type}', but got '{condition.get('type')}'."
        context['current_condition'] = condition

    @runner.step(r'And the boolean condition should be (true|false)')
    def and_the_boolean_condition_should_be(context, value):
        assert context.get('current_condition') is not None, "No current condition in context."
        condition = context['current_condition']
        assert condition.get('type') == 'BOOLEAN', "Current condition is not of type BOOLEAN."
        expected_value = value == 'true'
        assert condition.get('value') == expected_value, f"Expected boolean condition to be '{expected_value}', but got '{condition.get('value')}'."

    @runner.step(r'And the IF node should have a then_block with (\d+) commands')
    def and_the_if_node_should_have_then_block_with_n_commands(context, num_commands):
        assert context.get('current_node') is not None, "No current node in context."
        node = context['current_node']
        assert node.get('type') == 'IF', "Current node is not of type IF."
        then_block = node.get('then_block', [])
        assert len(then_block) == int(num_commands), f"Expected {num_commands} commands in then_block, but got {len(then_block)}."
        context['current_commands'] = then_block

    @runner.step(r'And the LOG command should have message "(.*)"')
    def and_the_log_command_should_have_message(context, message):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'LOG', "Current command is not of type LOG."
        assert command.get('message') == message, f"Expected log message to be '{message}', but got '{command.get('message')}'."

    @runner.step(r'And the comparison condition should have left "(.*)", operator "(.*)", and right "(.*)"')
    def and_the_comparison_condition_should_have(context, left, operator, right):
        assert context.get('current_condition') is not None, "No current condition in context."
        condition = context['current_condition']
        assert condition.get('type') == 'COMPARISON', "Current condition is not of type COMPARISON."
        assert condition.get('left') == left, f"Expected left to be '{left}', but got '{condition.get('left')}'."
        assert condition.get('operator') == operator, f"Expected operator to be '{operator}', but got '{condition.get('operator')}'."
        assert condition.get('right') == right, f"Expected right to be '{right}', but got '{condition.get('right')}'."

    @runner.step(r'And the IF node should have an else_block with (\d+) commands')
    def and_the_if_node_should_have_else_block_with_n_commands(context, num_commands):
        assert context.get('current_node') is not None, "No current node in context."
        node = context['current_node']
        assert node.get('type') == 'IF', "Current node is not of type IF."
        else_block = node.get('else_block', [])
        assert len(else_block) == int(num_commands), f"Expected {num_commands} commands in else_block, but got {len(else_block)}."
        context['current_commands'] = else_block

    @runner.step(r'And the REQUEST_REVIEW command should have message "(.*)"')
    def and_the_request_review_command_should_have_message(context, message):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'REQUEST_REVIEW', "Current command is not of type REQUEST_REVIEW."
        assert command.get('message') == message, f"Expected message to be '{message}', but got '{command.get('message')}'."

    @runner.step(r'And the SET_BUDGET command should have variable "(.*)"')
    def and_the_set_budget_command_should_have_variable(context, variable):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'SET_BUDGET', "Current command is not of type SET_BUDGET."
        assert command.get('variable') == variable, f"Expected variable to be '{variable}', but got '{command.get('variable')}'."

    @runner.step(r'And the SET_BUDGET command should have value "(.*)"')
    def and_the_set_budget_command_should_have_value(context, value):
        assert context.get('current_command') is not None, "No current command in context."
        command = context['current_command']
        assert command.get('type') == 'SET_BUDGET', "Current command is not of type SET_BUDGET."
        assert str(command.get('value')) == value, f"Expected value to be '{value}', but got '{command.get('value')}'."

    @runner.step(r'Given I have a TissLM script with content:')
    def given_a_tisslm_script_with_content(context, script_content):
        context['script'] = script_content.strip().strip("'\"")

    @runner.step(r'And the AST should contain a print statement with the text "(.*)"')
    def and_the_ast_should_contain_print_statement(context, text):
        assert context.get('ast') is not None, "AST is not in context."
        for node in context['ast']:
            if node.get('type') == 'PRINT' and node.get('text') == text:
                return
        raise AssertionError(f"Print statement with text '{text}' not found in AST.")
