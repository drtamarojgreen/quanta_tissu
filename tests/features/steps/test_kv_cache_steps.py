import numpy as np
import sys
import os

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.tokenizer import Tokenizer

def register_steps(runner):
    @runner.step(r'^Given a model and tokenizer$')
    def context(context):
        np.random.seed(42)
        model = QuantaTissu(model_config)
        tokenizer = Tokenizer()
        context['model'] = model
        context['tokenizer'] = tokenizer

    @runner.step(r'^When I generate text with a prompt "(.*)" and (\d+) new tokens using the KV cache$')
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

    @runner.step(r'^And I generate text with the same prompt and new tokens without the KV cache$')
    def generate_without_cache(context):
        prompt = context['prompt']
        n_new_tokens = context['n_new_tokens']
        prompt_tokens = context['tokenizer'].tokenize(prompt)
        non_cached_generated_ids = context['model'].generate(
            prompt_tokens,
            n_new_tokens=n_new_tokens,
            method="greedy",
            use_cache=False
        )
        context['non_cached_generated_ids'] = non_cached_generated_ids

    @runner.step(r'^Then the generated tokens should be the same$')
    def compare_results(context):
        assert context['cached_generated_ids'] == context['non_cached_generated_ids']
        return "Test passed!"
