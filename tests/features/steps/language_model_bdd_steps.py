import os
import sys
import time
import numpy as np
import requests
import re

# --- Path Setup ---
# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust path to go up to quanta_tissu
sys.path.insert(0, project_root)

# --- Imports from project ---
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.db.client import TissDBClient
from quanta_tissu.tisslm.core.rules.enforcer import RuleEnforcer
from quanta_tissu.tisslm.core.sentiment import SentimentAnalyzer
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

    @runner.step(r"^a running TissDB instance for the knowledge base$")
    def given_db_instance(context):
        try:
            response = requests.get(DB_URL)
            assert response.status_code == 200
            context['db_url'] = DB_URL
            context['db_token'] = DB_TOKEN
        except requests.exceptions.ConnectionError:
            raise Exception(f"TissDB instance is not running on {DB_URL}")

    @runner.step(r'^an empty knowledge base collection named "([^"]*)"$')
    def given_empty_collection(context, collection_name):
        context['collection_name'] = collection_name
        db_name = f"bdd_db_{collection_name}"
        context['db_name'] = db_name
        headers = {"Authorization": f"Bearer {context['db_token']}"}

        # Clean up any previous runs
        requests.delete(f"{context['db_url']}/{db_name}", headers=headers)

        # Create new DB and collection
        requests.put(f"{context['db_url']}/{db_name}", headers=headers, json={})
        response = requests.put(f"{context['db_url']}/{db_name}/{collection_name}", headers=headers, json={})
        assert response.status_code in [200, 201]

    @runner.step(r'^the knowledge base contains a document with id "([^"]*)" and content "([^"]*)"$')
    def given_kb_contains_doc(context, doc_id, content):
        db_name = context['db_name']
        collection_name = context['collection_name']
        headers = {"Authorization": f"Bearer {context['db_token']}"}
        document = {"text": content, "source": "test_setup"}
        response = requests.put(f"{context['db_url']}/{db_name}/{collection_name}/{doc_id}", json=document, headers=headers)
        assert response.status_code in [200, 201]

    @runner.step(r"^a sentiment analyzer$")
    def given_sentiment_analyzer(context):
        if not os.path.exists(CORPUS_SENTIMENTS_PATH):
            raise Exception(f"Sentiment lexicon not found at {CORPUS_SENTIMENTS_PATH}. Run the evaluation script to generate it.")
        context['sentiment_analyzer'] = SentimentAnalyzer(tokenizer=context['tokenizer'], sentiment_lexicon_path=CORPUS_SENTIMENTS_PATH)


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

    @runner.step(r'^I generate (\d+) tokens from the prompt "([^"]*)" using the "([^"]*)" sampling method$')
    def when_generate_standard(context, length, prompt, method):
        model, tokenizer = context['model'], context['tokenizer']
        context['generated_text'] = generate_text_helper(model, tokenizer, prompt, length, method=method)

    @runner.step(r"^I generate 50 tokens from the prompt$")
    def when_generate_for_rules(context):
        model, tokenizer = context['model'], context['tokenizer']
        context['raw_text'] = generate_text_helper(model, tokenizer, context['prompt'], 50, method="nucleus")

    @runner.step(r"^I apply the RuleEnforcer to the generated text$")
    def when_apply_rules(context):
        enforcer = RuleEnforcer(strictness=1.0)
        context['cleaned_text'] = enforcer.apply_rules(context['raw_text'])

    @runner.step(r'^I ask the model the question "([^"]*)"$')
    def when_ask_question(context, question):
        model, tokenizer = context['model'], context['tokenizer']
        db_name, collection_name = context['db_name'], context['collection_name']

        response = requests.get(f"{context['db_url']}/{db_name}/{collection_name}/mars_mission", headers={"Authorization": f"Bearer {context['db_token']}"})
        retrieved_text = response.json()['text'] if response.status_code == 200 else ""

        final_prompt = f"Based on this context: '{retrieved_text}', answer the question: '{question}'"
        context['generated_answer'] = generate_text_helper(model, tokenizer, final_prompt, 30)

    @runner.step(r'^the model generates the answer "([^"]*)"$')
    def when_model_generates_answer(context, answer):
        context['generated_answer'] = answer

    @runner.step(r"^the model stores this interaction in the knowledge base$")
    def when_store_interaction(context):
        db_name, collection_name = context['db_name'], context['collection_name']
        question = "What is the capital of the fictional country Eldoria?"
        answer = context['generated_answer']
        doc_id = "eldoria_interaction"
        doc_content = f"Question: {question} Answer: {answer}"
        document = {"text": doc_content, "source": "bdd_test"}
        response = requests.put(f"{context['db_url']}/{db_name}/{collection_name}/{doc_id}", json=document, headers={"Authorization": f"Bearer {context['db_token']}"})
        assert response.status_code in [200, 201]

    @runner.step(r'^I generate (\d+) tokens from the prompt "([^"]*)" with a "([^"]*)" sentiment of strength ([\d\.]+)
)
    def when_generate_with_sentiment(context, length, prompt, sentiment, strength):
        model, tokenizer, analyzer = context['model'], context['tokenizer'], context['sentiment_analyzer']
        context['generated_text'] = generate_text_helper(
            model, tokenizer, prompt, length,
            method="adaptive_sentiment",
            sentiment_analyzer=analyzer,
            target_sentiment=sentiment,
            target_strength=strength
        )

    # --- Then Steps ---

    @runner.step(r"^the generated text with and without the cache should be identical$")
    def then_cache_correctness(context):
        assert context['no_cache_text'] == context['cache_text'], "Generated text with and without KV cache should be identical"

    @runner.step(r"^the generation with the KV cache should be faster than without it$")
    def then_cache_performance(context):
        assert context['cache_time'] < context['no_cache_time'], "Generation with KV cache should be faster than without it"

    @runner.step(r"^the generated text should not be empty$")
    def then_text_not_empty(context):
        assert context['generated_text'] is not None
        assert len(context['generated_text'].strip()) > 0

    @runner.step(r"^the generated text should contain a plausible number of words based on the token length$")
    def then_text_plausible_length(context):
        word_count = len(context['generated_text'].split())
        assert word_count > 5

    @runner.step(r"^the cleaned text should be different from the raw generated text$")
    def then_text_is_cleaned(context):
        assert context['raw_text'] != context['cleaned_text']

    @runner.step(r'^the cleaned text should not contain "([^"]*)"$')
    def then_cleaned_text_obeys_rules(context, prohibited_phrase):
        assert prohibited_phrase not in context['cleaned_text']

    @runner.step(r'^the model should generate an answer containing "([^"]*)"$')
    def then_answer_contains(context, expected_answer):
        assert expected_answer in context['generated_answer']

    @runner.step(r'^the knowledge base should contain a document about "([^"]*)"$')
    def then_kb_contains_doc_about(context, topic):
        db_name, collection_name = context['db_name'], context['collection_name']
        response = requests.get(f"{context['db_url']}/{db_name}/{collection_name}/eldoria_interaction", headers={"Authorization": f"Bearer {context['db_token']}"})
        assert response.status_code == 200
        doc_text = response.json()['text']
        assert topic in doc_text

    @runner.step(r'^the generated text should have a "([^"]*)" sentiment$')
    def then_text_has_sentiment(context, sentiment):
        analyzer = context['sentiment_analyzer']
        analysis = analyzer.analyze_sentiment_of_text(context['generated_text'])
        dominant_sentiment = max(analysis, key=analysis.get)
        assert dominant_sentiment == sentiment

)
    def when_generate_with_sentiment(context, length, prompt, sentiment, strength):
        model, tokenizer, analyzer = context['model'], context['tokenizer'], context['sentiment_analyzer']
        context['generated_text'] = generate_text_helper(
            model, tokenizer, prompt, length,
            method="adaptive_sentiment",
            sentiment_analyzer=analyzer,
            target_sentiment=sentiment,
            target_strength=strength
        )

    # --- Then Steps ---

    @runner.step(r"^the generated text with and without the cache should be identical$")
    def then_cache_correctness(context):
        assert context['no_cache_text'] == context['cache_text'], "Generated text with and without KV cache should be identical"

    @runner.step(r"^the generation with the KV cache should be faster than without it$")
    def then_cache_performance(context):
        assert context['cache_time'] < context['no_cache_time'], "Generation with KV cache should be faster than without it"

    @runner.step(r"^the generated text should not be empty$")
    def then_text_not_empty(context):
        assert context['generated_text'] is not None
        assert len(context['generated_text'].strip()) > 0

    @runner.step(r"^the generated text should contain a plausible number of words based on the token length$")
    def then_text_plausible_length(context):
        word_count = len(context['generated_text'].split())
        assert word_count > 5

    @runner.step(r"^the cleaned text should be different from the raw generated text$")
    def then_text_is_cleaned(context):
        assert context['raw_text'] != context['cleaned_text']

    @runner.step(r'^the cleaned text should not contain "([^"]*)"$')
    def then_cleaned_text_obeys_rules(context, prohibited_phrase):
        assert prohibited_phrase not in context['cleaned_text']

    @runner.step(r'^the model should generate an answer containing "([^"]*)"$')
    def then_answer_contains(context, expected_answer):
        assert expected_answer in context['generated_answer']

    @runner.step(r'^the knowledge base should contain a document about "([^"]*)"$')
    def then_kb_contains_doc_about(context, topic):
        db_name, collection_name = context['db_name'], context['collection_name']
        response = requests.get(f"{context['db_url']}/{db_name}/{collection_name}/eldoria_interaction", headers={"Authorization": f"Bearer {context['db_token']}"})
        assert response.status_code == 200
        doc_text = response.json()['text']
        assert topic in doc_text

    @runner.step(r'^the generated text should have a "([^"]*)" sentiment$')
    def then_text_has_sentiment(context, sentiment):
        analyzer = context['sentiment_analyzer']
        analysis = analyzer.analyze_sentiment_of_text(context['generated_text'])
        dominant_sentiment = max(analysis, key=analysis.get)
        assert dominant_sentiment == sentiment
