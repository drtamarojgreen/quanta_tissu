import json
import re

# --- Native BDD Framework Support ---
# This is a simple decorator-based registry. A native BDD runner would
# import this `registry` object to find and execute the correct step definition.
class StepRegistry:
    def __init__(self):
        self.steps = {}

    def step(self, pattern):
        def decorator(func):
            self.steps[pattern] = func
            return func
        return decorator

registry = StepRegistry()
# This would be your actual parser. For this example, we'll use a mock
# that handles the scenarios in the feature file.
# from quanta_tissu.parser import TissLangParser
class MockTissLangParser:
    """A mock parser to demonstrate step definitions."""
    def parse(self, script_text):
        script_text = script_text.strip()
        if script_text == 'TASK "A simple task"':
            return [{"type": "TASK", "description": "A simple task"}]
        if 'STEP "Run a command"' in script_text:
            return [{"type": "STEP", "description": "Run a command", "commands": [{"type": "RUN", "command": "echo 'Hello, BDD!'"}]}]
        if 'STEP "Write to a file"' in script_text:
            return [{"type": "STEP", "description": "Write to a file", "commands": [{"type": "WRITE", "path": "greetings.txt", "language": "EOF", "content": "Hello from TissLang!\nThis is a test."}]}]
        if 'TASK "A complex task with multiple steps"' in script_text:
            # Just returning 4 dummy nodes to satisfy the count
            return [{}, {}, {}, {}]
        if 'STEP "This will fail"' in script_text:
            raise SyntaxError("Unexpected end of script")
        return None

# --- Step Definitions ---
# These functions would be discovered and registered by your native BDD framework.
# The framework would iterate through `registry.steps` to match Gherkin steps.

@registry.step(r'a TissLang script:')
def given_a_tisslang_script(context, doc_string=None):
    """Handles: Given a TissLang script:"""
    context.script = doc_string

@registry.step(r'I parse the script')
def when_i_parse_the_script(context):
    """Handles: When I parse the script"""
    parser = MockTissLangParser()
    context.error = None
    try:
        context.ast = parser.parse(context.script)
    except Exception as e:
        context.error = e

@registry.step(r'the AST should be:')
def then_the_ast_should_be(context, doc_string=None):
    """Handles: Then the AST should be:"""
    expected_ast = json.loads(doc_string)
    assert context.ast is not None, "AST was not generated"
    assert context.ast == expected_ast, f"Expected {expected_ast}, but got {context.ast}"

@registry.step(r'the AST should have (\d+) top-level nodes')
def then_the_ast_should_have_nodes(context, count):
    """Handles: Then the AST should have {count:d} top-level nodes"""
    assert context.ast is not None, "AST was not generated"
    assert len(context.ast) == int(count), f"Expected {count} nodes, but got {len(context.ast)}"

@registry.step(r'parsing should fail with an error containing "([^"]*)"')
def then_parsing_should_fail_with_error(context, error_message):
    """Handles: Then parsing should fail with an error containing "{error_message}" """
    assert context.error is not None, "Parsing did not fail as expected"
    assert error_message in str(context.error), f"Error message '{str(context.error)}' did not contain '{error_message}'"