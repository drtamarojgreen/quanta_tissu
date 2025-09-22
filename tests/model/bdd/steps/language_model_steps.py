import os
import sys
import time
import numpy as np
import requests

# --- Path Setup ---
# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..', '..', '..'))
sys.path.insert(0, project_root)

# --- Imports from project ---
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.core.layers import softmax

# --- Configuration ---
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
FINAL_CHECKPOINT_PATH = os.path.join(TEST_MODEL_DIR, "checkpoint_step_50000.npz")
CORPUS_SENTIMENTS_PATH = os.path.join(project_root, "data", "corpus_sentiments.cat")

DB_PORT = 9876
DB_TOKEN = "static_test_token"
DB_URL = f"http://localhost:{DB_PORT}"

# --- Helper Functions ---
def generate_text_helper(model, tokenizer, prompt, length, method="greedy", **kwargs):
    prompt_tokens = tokenizer.tokenize(prompt).tolist()
    generated_tokens = model.sample(
        prompt_tokens=prompt_tokens,
        n_new_tokens=length,
        method=method,
        tokenizer=tokenizer,
        **kwargs
    )
    return tokenizer.detokenize(np.array(generated_tokens))

def register_steps(runner):

    # --- Given Steps ---

    @runner.step(r"^a trained language model and tokenizer$")
    def given_model(context):
        try:
            tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
            model_config["vocab_size"] = tokenizer.get_vocab_size()
            model = QuantaTissu(model_config)
            if os.path.exists(FINAL_CHECKPOINT_PATH):
                model.load_weights(FINAL_CHECKPOINT_PATH)
            else:
                print(f"Warning: Model checkpoint not found at {FINAL_CHECKPOINT_PATH}. Using random weights.")
            context['model'] = model
            context['tokenizer'] = tokenizer
        except Exception as e:
            raise Exception(f"Failed to load model or tokenizer: {e}")

    @runner.step(r'^the prompt "([^"]*)"$')
    def given_prompt(context, prompt):
        context['prompt'] = prompt

    # --- When Steps ---

    @runner.step(r"^I generate (\d+) tokens without the KV cache$")
    def when_generate_no_cache(context, count):
        model, tokenizer = context['model'], context['tokenizer']
        prompt = context['prompt']
        prompt_tokens = tokenizer.tokenize(prompt).tolist()
        generated_tokens = []
        current_tokens = list(prompt_tokens)

        start_time = time.time()
        for i in range(count):
            logits, _ = model.forward(np.array([current_tokens]))
            next_token = np.argmax(softmax(logits[:, -1, :]))
            generated_tokens.append(next_token)
            current_tokens.append(next_token)
        context['no_cache_time'] = time.time() - start_time
        context['no_cache_text'] = tokenizer.detokenize(np.array(generated_tokens))

    @runner.step(r"^I generate (\d+) tokens with the KV cache$")
    def when_generate_with_cache(context, count):
        model, tokenizer = context['model'], context['tokenizer']
        prompt = context['prompt']
        start_time = time.time()
        cache_text = generate_text_helper(model, tokenizer, prompt, count, method="greedy")
        context['cache_time'] = time.time() - start_time
        context['cache_text'] = cache_text

    # --- Then Steps ---

    @runner.step(r"^the generated text with and without the cache should be identical$")
    def then_cache_correctness(context):
        assert context['no_cache_text'] == context['cache_text'], "Generated text with and without KV cache should be identical"

    @runner.step(r"^the generation with the KV cache should be faster than without it$")
    def then_cache_performance(context):
        assert context['cache_time'] < context['no_cache_time'], "Generation with KV cache should be faster than without it"
