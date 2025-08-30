import requests
import json
import re

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb" # Use a consistent test database

def register_steps(runner):

    @runner.step(r'^a TissDB collection named "(.*)" is available for TissLM$')
    def given_tissdb_collection_is_available(context, collection_name):
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}")
        assert response.status_code in [201, 200, 409] # 409 Conflict is ok if it already exists
        context['collection_name'] = collection_name

    @runner.step(r'And the "(.*)" collection contains a document with ID "(.*)" and content (.*)')
    def and_collection_contains_document(context, collection_name, doc_id, content_str):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200

    @runner.step(r'When the TissLM receives a user prompt "(.*)"')
    def when_tisslm_receives_prompt(context, prompt):
        context['user_prompt'] = prompt

    @runner.step(r'And the TissLM KnowledgeBase formulates a TissQL query "(.*)"')
    def and_knowledgebase_formulates_query(context, query):
        context['tissql_query'] = query

    @runner.step(r'And the KnowledgeBase executes the query against the "(.*)" collection')
    def and_knowledgebase_executes_query(context, collection_name):
        data = {"query": context['tissql_query']}
        response = requests.post(f"{BASE_URL}/{DB_NAME}/{collection_name}/_query", json=data)
        assert response.status_code == 200
        context['query_result'] = response.json()

    @runner.step(r'Then the query result for the KnowledgeBase should contain "(.*)"')
    def then_query_result_should_contain(context, expected_text):
        found = False
        for item in context['query_result']:
            if any(expected_text in str(val) for val in item.values()):
                found = True
                break
        assert found, f"Expected text '{expected_text}' not found in query result: {context['query_result']}"

    @runner.step(r'And the query result for the KnowledgeBase should not contain "(.*)"')
    def and_query_result_should_not_contain(context, unexpected_text):
        found = False
        for item in context['query_result']:
            if any(unexpected_text in str(val) for val in item.values()):
                found = True
                break
        assert not found, f"Unexpected text '{unexpected_text}' found in query result: {context['query_result']}"

    @runner.step(r'Given a user prompt "(.*)"')
    def given_user_prompt(context, prompt):
        context['user_prompt'] = prompt

    @runner.step(r'And a retrieved context from TissDB: "(.*)"')
    def and_retrieved_context(context, retrieved_context):
        context['retrieved_context'] = retrieved_context

    @runner.step(r'When the TissLM augments the prompt with the retrieved context')
    def when_tisslm_augments_prompt(context):
        context['final_prompt'] = f"context: {{{context['retrieved_context']}}} question: {{{context['user_prompt']}}}"

    @runner.step(r'^Then the final prompt sent to the language model should be "(.*)"$')
    def then_final_prompt_should_be(context, expected_prompt):
        normalized_actual = re.sub(r'\s+', ' ', context['final_prompt']).strip()
        normalized_expected = re.sub(r'\s+', ' ', expected_prompt).strip()
        assert normalized_actual == normalized_expected, f"Expected '{normalized_expected}', but got '{normalized_actual}'"

    @runner.step(r'When a simulated Sinew client creates a document with ID "(.*)" and content (.*) in "(.*)"')
    def when_sinew_creates_document(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200

    @runner.step(r'When a simulated Sinew client deletes the document with ID "(.*)" from "(.*)"')
    def when_sinew_deletes_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}")
        assert response.status_code == 204
