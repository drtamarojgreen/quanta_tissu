import json
from quanta_tissu.tisslm.parser.tisslang_parser import TissLangParser, TissLangParserError

def register_steps(runner):
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
