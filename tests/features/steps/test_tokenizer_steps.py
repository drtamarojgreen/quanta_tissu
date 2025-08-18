from quanta_tissu.tisslm.tokenizer import Tokenizer

def register_steps(step): # Changed
    @step(r'^Given a tokenizer$') # Changed
    def context(context):
        context['tokenizer'] = Tokenizer()

    @step(r'^When I tokenize the string "(.*)"$') # Changed
    def tokenize_string(context, string):
        context['tokens'] = context['tokenizer'].tokenize(string)

    @step(r'^And I detokenize the tokens$') # Changed
    def detokenize_string(context):
        context['detokenized_string'] = context['tokenizer'].detokenize(context['tokens'])

    @step(r'^Then the resulting string should be "(.*)"$') # Changed
    def compare_strings(context, expected_string):
        assert context['detokenized_string'] == expected_string
        return "Test passed!"

    @step(r'^Then the result should be an empty list of tokens$') # Changed
    def check_empty_list(context):
        # The BPE tokenizer will not produce an empty list for a non-empty string.
        # This test might be for a different tokenizer logic.
        # However, for an empty string, it should be empty.
        assert len(context['tokens']) == 0
        return "Test passed!"
