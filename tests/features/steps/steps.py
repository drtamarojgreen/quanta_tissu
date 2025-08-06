from behave import given, when, then
import numpy as np
from quanta_tissu.quanta_tissu.model import QuantaTissu
from quanta_tissu.quanta_tissu.tokenizer import tokenize
from quanta_tissu.quanta_tissu.config import vocab

@given('a trained QuantaTissu model')
def step_given_a_trained_model(context):
    config = {
        "d_model": 16,
        "vocab_size": len(vocab),
        "num_heads": 4,
        "d_ff": 32,
        "n_layers": 2,
    }
    context.model = QuantaTissu(config)

@given('a prompt "{prompt}"')
def step_given_a_prompt(context, prompt):
    context.prompt = prompt

@when('I ask the model to predict the next token')
def step_when_predict_next_token(context):
    token_ids = tokenize(context.prompt)
    context.next_token = context.model.predict(token_ids)

@then('the model should return a single token ID')
def step_then_return_single_token_id(context):
    assert isinstance(context.next_token, np.integer)

@when('I ask the model to generate a sequence of {num_tokens} tokens')
def step_when_generate_sequence(context, num_tokens):
    num_tokens = int(num_tokens)
    token_ids = tokenize(context.prompt)
    generated_tokens = []
    for _ in range(num_tokens):
        next_token = context.model.predict(token_ids)
        generated_tokens.append(next_token)
        token_ids = np.append(token_ids, next_token)
    context.generated_tokens = generated_tokens

@then('the model should return a sequence of {num_tokens} token IDs')
def step_then_return_sequence(context, num_tokens):
    num_tokens = int(num_tokens)
    assert isinstance(context.generated_tokens, list)
    assert len(context.generated_tokens) == num_tokens
    for token in context.generated_tokens:
        assert isinstance(token, np.integer)

@given('a QuantaTissu model with a fixed vocabulary')
def step_given_a_model_with_fixed_vocab(context):
    # This is the same as the other given step, but we can make it more explicit if needed
    config = {
        "d_model": 16,
        "vocab_size": len(vocab),
        "num_heads": 4,
        "d_ff": 32,
        "n_layers": 2,
    }
    context.model = QuantaTissu(config)

@given('a sentence "{sentence}"')
def step_given_a_sentence(context, sentence):
    context.sentence = sentence

@when('I tokenize the sentence')
def step_when_tokenize_sentence(context):
    context.token_ids = tokenize(context.sentence)

@then('the token "{token}" should be mapped to the "<unk>" token ID')
def step_then_map_to_unk(context, token):
    # We need to find the position of the token in the original sentence
    words = context.sentence.lower().split()
    try:
        word_index = words.index(token)
        assert context.token_ids[word_index] == vocab["<unk>"]
    except ValueError:
        # The token was not found in the sentence, which is a problem with the test case itself.
        raise ValueError(f"Token '{token}' not in sentence '{context.sentence}'")
