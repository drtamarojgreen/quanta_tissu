import numpy as np
import sys
import os

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

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

    @step(r'^When I generate text with a prompt "(.*)" and (\d+) new tokens using the KV cache$') # Changed
    def generate_with_cache(context, prompt, n_new_tokens):
        n_new_tokens = int(n_new_tokens)
        prompt_tokens = context['tokenizer'].tokenize(prompt)
        cached_generated_ids = context['model'].generate(
            prompt_tokens,
            n_new_tokens=n_new_tokens,
            method="greedy"
        )
        context['cached_generated_ids'] = cached_generated_ids
        context['prompt'] = prompt
        context['n_new_tokens'] = n_new_tokens

    @step(r'^And I generate text with the same prompt and new tokens without the KV cache$') # Changed
    def generate_without_cache(context):
        prompt = context['prompt']
        n_new_tokens = context['n_new_tokens']
        prompt_tokens = context['tokenizer'].tokenize(prompt)
        non_cached_generated_ids = []
        current_tokens = list(prompt_tokens)
        for _ in range(n_new_tokens):
            token_ids_np = np.array(current_tokens)
            next_token_id = context['model'].predict(token_ids_np, method="greedy")
            non_cached_generated_ids.append(next_token_id)
            current_tokens.append(next_token_id)
        context['non_cached_generated_ids'] = non_cached_generated_ids

    @step(r'^Then the generated tokens should be the same$') # Changed
    def compare_results(context):
        assert context['cached_generated_ids'] == context['non_cached_generated_ids']
        return "Test passed!"
