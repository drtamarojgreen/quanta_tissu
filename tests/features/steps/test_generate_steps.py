import numpy as np
from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.tokenizer import Tokenizer, vocab

def register_steps(runner):
    @runner.step(r'^Given a model and tokenizer$')
    def context(context):
        np.random.seed(42)
        model = QuantaTissu(model_config)
        tokenizer = Tokenizer(vocab)
        context['model'] = model
        context['tokenizer'] = tokenizer
        
    @runner.step(r'^When I generate (\d+) new tokens with the prompt "(.*)"$')
    def generate_tokens(context, n_new_tokens, prompt):
        n_new_tokens = int(n_new_tokens)
        prompt_tokens = context['tokenizer'].tokenize(prompt)
        generated_ids = context['model'].generate(
            prompt_tokens,
            n_new_tokens=n_new_tokens,
            method="greedy"
        )
        context['generated_ids'] = generated_ids

    @runner.step(r'^Then the generated tokens should be a list of (\d+) integers$')
    def check_generated_tokens(context, n_new_tokens):
        n_new_tokens = int(n_new_tokens)
        assert isinstance(context['generated_ids'], list)
        assert len(context['generated_ids']) == n_new_tokens
        for token_id in context['generated_ids']:
            assert isinstance(token_id, int)
        return "Test passed!"
