import numpy as np
from quanta_tissu.quanta_tissu.model import QuantaTissu
from quanta_tissu.quanta_tissu.config import model_config
from quanta_tissu.quanta_tissu.tokenizer import Tokenizer, vocab

def register_steps(runner):
    @runner.step(r'^Given a model and tokenizer$')
    def context(context):
        np.random.seed(42)
        model = QuantaTissu(model_config)
        tokenizer = Tokenizer(vocab)
        context['model'] = model
        context['tokenizer'] = tokenizer

    @runner.step(r'^When I predict the next token for the prompt "(.*)"$')
    def predict_next_token(context, prompt):
        prompt_tokens = context['tokenizer'].tokenize(prompt)
        token_ids_np = np.array(prompt_tokens)
        next_token_id = context['model'].predict(token_ids_np, method="greedy")
        context['next_token_id'] = next_token_id

    @runner.step(r'^Then the next token should be a valid token id$')
    def check_next_token(context):
        assert isinstance(context['next_token_id'], int)
        assert context['next_token_id'] >= 0
        assert context['next_token_id'] < len(vocab)
        return "Test passed!"
