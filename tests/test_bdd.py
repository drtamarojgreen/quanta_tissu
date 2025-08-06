import re
import sys
import os
from collections import defaultdict

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from quanta_tissu.model import QuantaTissu
from quanta_tissu.tokenizer import tokenize
from quanta_tissu.config import model_config

# --- BDD Test Runner ---

step_registry = defaultdict(list)
context = {}

def step(pattern):
    def decorator(func):
        step_registry[pattern].append(func)
        return func
    return decorator

def parse_feature_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    features = []
    current_feature = None
    current_scenario = None

    for line in content.split('\n'):
        line = line.strip()
        if line.startswith('Feature:'):
            if current_feature:
                features.append(current_feature)
            current_feature = {'name': line.replace('Feature:', '').strip(), 'scenarios': []}
            current_scenario = None
        elif line.startswith('Scenario:'):
            current_scenario = {'name': line.replace('Scenario:', '').strip(), 'steps': []}
            current_feature['scenarios'].append(current_scenario)
        elif line.startswith(('Given', 'When', 'Then', 'And')):
            if current_scenario:
                current_scenario['steps'].append(line)

    if current_feature:
        features.append(current_feature)

    return features

def run_tests(features):
    for feature in features:
        print(f"Feature: {feature['name']}")
        for scenario in feature['scenarios']:
            print(f"  Scenario: {scenario['name']}")
            context.clear()
            success = True
            for step_text in scenario['steps']:
                found_step = False
                for pattern, funcs in step_registry.items():
                    match = re.match(pattern, step_text)
                    if match:
                        for func in funcs:
                            try:
                                func(context, *match.groups())
                                print(f"    - {step_text} [PASSED]")
                                found_step = True
                                break
                            except Exception as e:
                                print(f"    - {step_text} [FAILED]")
                                print(f"      Error: {e}")
                                success = False
                                break
                    if found_step or not success:
                        break
                if not found_step:
                    print(f"    - {step_text} [SKIPPED] (No matching step implementation)")
                    success = False
                if not success:
                    break
            if success:
                print("  Scenario: PASSED")
            else:
                print("  Scenario: FAILED")
            print()

# --- Step Implementations ---

@step(r'Given a trained QuantaTissu model')
def given_a_model(context):
    context['model'] = QuantaTissu(model_config)

@step(r'And a prompt "(.*)"')
def and_a_prompt(context, prompt):
    context['prompt'] = prompt

@step(r'When I ask the model to predict the next token')
def when_predict_next_token(context):
    token_ids = tokenize(context['prompt'])
    next_token = context['model'].predict(token_ids)
    context['next_token'] = next_token

@step(r'Then the model should return a single token ID')
def then_return_single_token_id(context):
    assert isinstance(context['next_token'], int)

@step(r'When I ask the model to generate a sequence of (\d+) tokens')
def when_generate_sequence(context, num_tokens):
    token_ids = list(tokenize(context['prompt']))
    generated_tokens = []
    model = context['model']
    for _ in range(int(num_tokens)):
        next_token = model.predict(token_ids)
        generated_tokens.append(next_token)
        token_ids.append(next_token)
    context['generated_tokens'] = generated_tokens

@step(r'Then the model should return a sequence of (\d+) token IDs')
def then_return_sequence_of_token_ids(context, num_tokens):
    assert isinstance(context['generated_tokens'], list)
    assert len(context['generated_tokens']) == int(num_tokens)
    for token in context['generated_tokens']:
        assert isinstance(token, int)

@step(r'Given a QuantaTissu model with a fixed vocabulary')
def given_a_model_with_fixed_vocab(context):
    context['model'] = QuantaTissu(model_config)

@step(r'And a sentence "(.*)"')
def and_a_sentence(context, sentence):
    context['sentence'] = sentence

@step(r'When I tokenize the sentence')
def when_tokenize_sentence(context):
    context['tokens'] = tokenize(context['sentence'])

@step(r'Then the token "foobar" should be mapped to the "<unk>" token ID')
def then_foobar_is_unk(context):
    # The sentence is "this is a foobar test"
    # "foobar" is the 4th word, so the 4th token (index 3)
    assert context['tokens'][3] == model_config['vocab_size'] - 1 # <unk> is the last token
