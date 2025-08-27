import sys
import os

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

from quanta_tissu.tisslm.parser.parser import TissLangParser, TissLangParserError

def register_steps(runner):

    @runner.step(r'I have a TissLM script with content:')
    def i_have_a_tisslm_script_with_content(context):
        context.script_content = context.text

    @runner.step(r'I parse the script')
    def i_parse_the_script(context):
        parser = TissLangParser()
        try:
            # The current parser expects STEP/END blocks. The feature file does not have them.
            # This will likely fail. This is intended, as it reflects the current state of the code.
            context.ast = parser.parse(context.script_content)
            context.error = None
        except TissLangParserError as e:
            context.ast = None
            context.error = e

    @runner.step(r'the parser should produce an Abstract Syntax Tree \(AST\)')
    def the_parser_should_produce_an_ast(context):
        # This step will likely fail because the parser will throw an error.
        assert context.error is None, f"Parser failed with error: {context.error}"
        assert context.ast is not None, "AST was not generated."
        assert isinstance(context.ast, list), "AST is not a list."

    @runner.step(r'the AST should contain a print statement with the text "(.*)"')
    def the_ast_should_contain_a_print_statement(context, text):
        # This step assumes the previous steps passed.
        # The parser does not support a 'PRINT' statement, so this will fail.
        # This is an accurate reflection of the current implementation.
        found = False
        if hasattr(context, 'ast') and context.ast:
            for node in context.ast:
                # This structure is hypothetical, as the parser doesn't create PRINT nodes.
                if node.get('type') == 'PRINT' and node.get('text') == text:
                    found = True
                    break
        assert found, f"Could not find a PRINT statement for '{text}' in the AST."
