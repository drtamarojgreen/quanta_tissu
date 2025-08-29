import sys
import os
from behave import * # Assuming behave is the underlying framework

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser, TissLangParserError

# Re-initialize the parser for each scenario
@before_scenario
def before_scenario(context, scenario):
    context.parser = TissLangParser()
    context.ast = None
    context.error = None

@given('I have a TissLang script with content:')
def step_impl(context):
    context.script_content = context.text

@when('I parse the script')
def step_impl(context):
    try:
        context.ast = context.parser.parse(context.script_content)
    except TissLangParserError as e:
        context.error = e

@then('the parser should produce an Abstract Syntax Tree (AST)')
def step_impl(context):
    assert context.error is None, f"Parsing failed with error: {context.error}"
    assert context.ast is not None, "AST was not generated."
    assert isinstance(context.ast, list), "AST is not a list."

@then('the AST should contain a {node_type} node at index {index}')
def step_impl(context, node_type, index):
    index = int(index)
    assert len(context.ast) > index, f"AST does not have an element at index {index}"
    node = context.ast[index]
    assert node['type'] == node_type, f"Expected node type {node_type}, but got {node['type']}"
    context.current_node = node # Store for subsequent checks

@then('the {node_type} node should have a description "{description}"')
def step_impl(context, node_type, description):
    assert context.current_node['type'] == node_type
    assert context.current_node['description'] == description

@then('the {node_type} node should contain {count} commands')
def step_impl(context, node_type, count):
    count = int(count)
    assert context.current_node['type'] == node_type
    assert 'commands' in context.current_node
    assert len(context.current_node['commands']) == count
    context.current_commands = context.current_node['commands'] # Store for subsequent checks

@then('the command at index {index} should be of type {command_type}')
def step_impl(context, index, command_type):
    index = int(index)
    assert len(context.current_commands) > index
    command = context.current_commands[index]
    assert command['type'] == command_type
    context.current_command = command # Store for subsequent checks

@then('the {command_type} command should have {key} "{value}"')
def step_impl(context, command_type, key, value):
    assert context.current_command['type'] == command_type
    assert context.current_command[key] == value

@then('the parser should raise a TissLangParserError')
def step_impl(context):
    assert context.error is not None, "Parser did not raise an error."
    assert isinstance(context.error, TissLangParserError), "Parser raised an unexpected error type."

@then('the error message should contain "{message_part}"')
def step_impl(context, message_part):
    assert message_part in str(context.error)

@then('the AST should contain a directive named "{name}" with value "{value}"')
def step_impl(context, name, value):
    found = False
    if hasattr(context.parser, 'metadata') and 'directives' in context.parser.metadata:
        for directive in context.parser.metadata['directives']:
            if directive['name'] == name and directive['value'] == value:
                found = True
                break
    assert found, f"Directive named '{name}' with value '{value}' not found in AST metadata."

@then('the {node_type} node should have a condition of type {condition_type}')
def step_impl(context, node_type, condition_type):
    assert context.current_node['type'] == node_type
    assert 'condition' in context.current_node
    assert context.current_node['condition']['type'] == condition_type
    context.current_condition = context.current_node['condition']

@then('the comparison condition should have left "{left}", operator "{operator}", and right "{right}"')
def step_impl(context, left, operator, right):
    assert context.current_condition['type'] == 'COMPARISON'
    assert context.current_condition['left'] == left
    assert context.current_condition['operator'] == operator
    # Convert right to appropriate type for comparison
    if right.lower() == 'true':
        right = True
    elif right.lower() == 'false':
        right = False
    else:
        try:
            right = int(right)
        except ValueError:
            try:
                right = float(right)
            except ValueError:
                pass # Keep as string
    assert context.current_condition['right'] == right

@then('the boolean condition should be {value}')
def step_impl(context, value):
    assert context.current_condition['type'] == 'BOOLEAN'
    expected_value = (value.lower() == 'true')
    assert context.current_condition['value'] == expected_value

@then('the {node_type} node should have a then_block with {count} commands')
def step_impl(context, node_type, count):
    count = int(count)
    assert context.current_node['type'] == node_type
    assert 'then_block' in context.current_node
    assert len(context.current_node['then_block']) == count
    context.current_commands = context.current_node['then_block']

@then('the {node_type} node should have an else_block with {count} commands')
def step_impl(context, node_type, count):
    count = int(count)
    assert context.current_node['type'] == node_type
    assert 'else_block' in context.current_node
    assert len(context.current_node['else_block']) == count
    context.current_commands = context.current_node['else_block']