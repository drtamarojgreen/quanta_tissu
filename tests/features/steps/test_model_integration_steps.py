import numpy as np
from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.tokenizer import tokenize, detokenize
from quanta_tissu.tisslm.config import model_config

def register_steps(step): # Changed
    @step(r'Given a trained QuantaTissu model') # Changed
    def given_a_model(context):
        context['model'] = QuantaTissu(model_config)

    @step(r'And a prompt "(.*)"') # Changed
    def and_a_prompt(context, prompt):
        context['prompt'] = prompt

    @step(r'When I ask the model to predict the next token$') # Changed
    def when_predict_next_token(context):
        token_ids = tokenize(context['prompt'])
        next_token = context['model'].predict(token_ids)
        context['next_token'] = next_token

    @step(r'Then the model should return a single token ID') # Changed
    def then_return_single_token_id(context):
        assert isinstance(context['next_token'], int)

    @step(r'When I ask the model to predict the next token with top_k sampling and k=(\d+)') # Changed
    def when_predict_next_token_with_top_k(context, k):
        token_ids = tokenize(context['prompt'])
        next_token = context['model'].predict(token_ids, method='top_k', top_k=int(k))
        context['next_token'] = next_token

    # This step is also defined in knowledge_base_steps.py
    @step(r'And the knowledge base contains the document "(.*)"') # Changed
    def and_kb_contains_doc(context, doc):
        # Ensure knowledge_base exists
        if not hasattr(context['model'], 'knowledge_base') or context['model'].knowledge_base is None:
             context['model'].__init__(model_config, use_db=True)
        context['model'].knowledge_base.add_document(doc)

    @step(r'When I ask the model to predict the next token with the prompt "(.*)"') # Changed
    def when_predict_with_prompt(context, prompt):
        context['next_token'] = context['model'].generate_with_kb(prompt, 1)

    @step(r'When I ask the model to predict the next token with nucleus sampling and p=([0-9.]+)') # Changed
    def when_predict_next_token_with_nucleus(context, p):
        token_ids = tokenize(context['prompt'])
        next_token = context['model'].predict(token_ids, method='nucleus', top_p=float(p))
        context['next_token'] = next_token

    @step(r'When I ask the model to generate a sequence of (\d+) tokens') # Changed
    def when_generate_sequence(context, num_tokens):
        token_ids = list(tokenize(context['prompt']))
        generated_tokens = context['model'].generate(token_ids, int(num_tokens))
        context['generated_tokens'] = generated_tokens

    @step(r'Then the model should return a sequence of (\d+) token IDs') # Changed
    def then_return_sequence_of_token_ids(context, num_tokens):
        assert isinstance(context['generated_tokens'], list)
        assert len(context['generated_tokens']) == int(num_tokens)
        for token in context['generated_tokens']:
            assert isinstance(token, int)

    @step(r'Given a QuantaTissu model with a fixed vocabulary') # Changed
    def given_a_model_with_fixed_vocab(context):
        context['model'] = QuantaTissu(model_config, use_db=False)

    @step(r'And a sentence "(.*)"') # Changed
    def and_a_sentence(context, sentence):
        context['sentence'] = sentence

    @step(r'When I tokenize the sentence') # Changed
    def when_tokenize_sentence(context):
        context['tokens'] = tokenize(context['sentence'])

    @step(r'Given a QuantaTissu model with a knowledge base') # Changed
    def given_a_model_with_kb(context):
        context['model'] = QuantaTissu(model_config)

    @step(r'When I ask the model to generate an answer to "(.*)" using the knowledge base') # Changed
    def when_generate_answer_with_kb(context, prompt):
        generated_ids = context['model'].generate_with_kb(prompt, 10)
        context['generated_answer_tokens'] = generated_ids if generated_ids else []


    @step(r'Then the model should generate an answer containing the word "(.*)"') # Changed
    def then_answer_contains_word(context, word):
        generated_text = detokenize(context['generated_answer_tokens'])
        assert word in generated_text
