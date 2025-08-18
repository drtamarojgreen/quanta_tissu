import numpy as np
from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.tokenizer import Tokenizer

def register_steps(step): # Changed
    @step(r'^Given a model and tokenizer$') # Changed
    def context(context):
        np.random.seed(42)
        model = QuantaTissu(model_config)
        tokenizer = Tokenizer()
        context['model'] = model
        context['tokenizer'] = tokenizer

    @step(r'^When I predict the next token for the prompt "(.*)"$') # Changed
    def predict_next_token(context, prompt):
        prompt_tokens = context['tokenizer'].tokenize(prompt)
        token_ids_np = np.array(prompt_tokens)
        next_token_id = context['model'].predict(token_ids_np, method="greedy")
        context['next_token_id'] = next_token_id

    @step(r'^Then the next token should be a valid token id$') # Changed
    def check_next_token(context):
        assert isinstance(context['next_token_id'], int)
        assert context['next_token_id'] >= 0
        assert context['next_token_id'] < context['tokenizer'].get_vocab_size()
        return "Test passed!"
