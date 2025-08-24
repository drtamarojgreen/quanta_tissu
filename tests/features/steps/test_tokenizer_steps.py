from quanta_tissu.tisslm.core.tokenizer import Tokenizer

def register_steps(runner):
    @runner.step(r'^Given a tokenizer$')
    def context(context):
        context['tokenizer'] = Tokenizer()

    @runner.step(r'^When I tokenize the string "(.*)"$')
    def tokenize_string(context, string):
        context['tokens'] = context['tokenizer'].tokenize(string)

    @runner.step(r'^And I detokenize the tokens$')
    def detokenize_string(context):
        context['detokenized_string'] = context['tokenizer'].detokenize(context['tokens'])

    @runner.step(r'^Then the resulting string should be "(.*)"$')
    def compare_strings(context, expected_string):
        assert context['detokenized_string'] == expected_string
        return "Test passed!"

    @runner.step(r'^Then the result should be an empty list of tokens$')
    def check_empty_list(context):
        assert context['tokens'].size == 0
        return "Test passed!"
