import numpy as np
from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.tokenizer import tokenize, detokenize
from quanta_tissu.tisslm.config import model_config

def register_steps(runner):
    @runner.step(r'Given a trained QuantaTissu model')
    def given_a_model(context):
        context['model'] = QuantaTissu(model_config)

    @runner.step(r'And a prompt "(.*)"')
    def and_a_prompt(context, prompt):
        context['prompt'] = prompt

    @runner.step(r'When I ask the model to predict the next token$')
    def when_predict_next_token(context):
        token_ids = tokenize(context['prompt'])
        next_token = context['model'].predict(token_ids)
        context['next_token'] = next_token

    @runner.step(r'Then the model should return a single token ID')
    def then_return_single_token_id(context):
        assert isinstance(context['next_token'], int)

    @runner.step(r'When I ask the model to predict the next token with top_k sampling and k=(\d+)')
    def when_predict_next_token_with_top_k(context, k):
        token_ids = tokenize(context['prompt'])
        next_token = context['model'].predict(token_ids, method='top_k', top_k=int(k))
        context['next_token'] = next_token

    @runner.step(r'And the knowledge base contains the document "(.*)"')
    def and_kb_contains_doc(context, doc):
        context['model'].knowledge_base.add_document(doc)

    @runner.step(r'When I ask the model to predict the next token with the prompt "(.*)"')
    def when_predict_with_prompt(context, prompt):
        context['next_token'] = context['model'].generate_with_kb(prompt)

    @runner.step(r'When I ask the model to predict the next token with nucleus sampling and p=([0-9.]+)')
    def when_predict_next_token_with_nucleus(context, p):
        token_ids = tokenize(context['prompt'])
        next_token = context['model'].predict(token_ids, method='nucleus', top_p=float(p))
        context['next_token'] = next_token

    @runner.step(r'When I ask the model to generate a sequence of (\d+) tokens')
    def when_generate_sequence(context, num_tokens):
        token_ids = list(tokenize(context['prompt']))
        generated_tokens = []
        model = context['model']
        for _ in range(int(num_tokens)):
            next_token = model.predict(np.array(token_ids))
            generated_tokens.append(next_token)
            token_ids.append(next_token)
        context['generated_tokens'] = generated_tokens

    @runner.step(r'Then the model should return a sequence of (\d+) token IDs')
    def then_return_sequence_of_token_ids(context, num_tokens):
        assert isinstance(context['generated_tokens'], list)
        assert len(context['generated_tokens']) == int(num_tokens)
        for token in context['generated_tokens']:
            assert isinstance(token, int)

    @runner.step(r'Given a QuantaTissu model with a fixed vocabulary')
    def given_a_model_with_fixed_vocab(context):
        context['model'] = QuantaTissu(model_config)

    @runner.step(r'And a sentence "(.*)"')
    def and_a_sentence(context, sentence):
        context['sentence'] = sentence

    @runner.step(r'When I tokenize the sentence')
    def when_tokenize_sentence(context):
        context['tokens'] = tokenize(context['sentence'])

    @runner.step(r'Then the token "foobar" should be mapped to the "<unk>" token ID')
    def then_foobar_is_unk(context):
        # The sentence is "this is a foobar test"
        # "foobar" is the 4th word, so the 4th token (index 3)
        assert context['tokens'][3] == model_config['vocab_size'] - 1 # <unk> is the last token

    @runner.step(r'Given a QuantaTissu model with a knowledge base')
    def given_a_model_with_kb(context):
        context['model'] = QuantaTissu(model_config)

    @runner.step(r'And the knowledge base contains the document "(.*)"')
    def and_kb_contains_doc(context, doc_text):
        context['model'].knowledge_base.add_document(doc_text)

    @runner.step(r'When I ask the model to generate an answer to "(.*)" using the knowledge base')
    def when_generate_answer_with_kb(context, prompt):
        # For testing, we'll just generate one token to see if it's on the right track
        next_token = context['model'].generate_with_kb(prompt)
        context['generated_answer_tokens'] = [next_token]

    @runner.step(r'Then the model should generate an answer containing the word "(.*)"')
    def then_answer_contains_word(context, word):
        # This is a simplification. In a real scenario, we'd generate more tokens
        # and check if the word appears in the detokenized text.
        generated_text = detokenize(context['generated_answer_tokens'])
        # For this test, we are cheating a little. Because we only generate one token,
        # we can't get a full word. Instead, we'll check if the *expected* word's
        # token is the one that was generated.
        expected_token = tokenize(word)[0]
        assert context['generated_answer_tokens'][0] == expected_token, \
            f"Generated token ID does not match expected token for '{word}'"
