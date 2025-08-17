import requests
import json
import re

BASE_URL = "http://localhost:8080"

def register_steps(runner):

    @runner.step(r'^a TissDB collection named "(.*)" is available for TissLM$')
    def collection_available_for_tisslm(context, collection_name):
        # This is effectively the same as creating it if it doesn't exist.
        response = requests.put(f"{BASE_URL}/{collection_name}")
        assert response.status_code in [201, 200]
        context.collection_name = collection_name

    @runner.step(r'^the "(.*)" collection contains a document with ID "(.*)" and content (.*)$')
    def collection_contains_document(context, collection_name, doc_id, content_str):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200

    @runner.step(r'^the TissLM receives a user prompt "(.*)"$')
    def tisslm_receives_prompt(context, prompt):
        context.user_prompt = prompt

    @runner.step(r'^the TissLM KnowledgeBase formulates a TissQL query "(.*)"$')
    def knowledgebase_formulates_query(context, query):
        context.tissql_query = query

    @runner.step(r'^the KnowledgeBase executes the query against the "(.*)" collection$')
    def knowledgebase_executes_query(context, collection_name):
        data = {"query": context.tissql_query}
        response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
        assert response.status_code == 200
        context.query_result = response.json()

    @runner.step(r'^the query result for the KnowledgeBase should contain "(.*)"$')
    def query_result_should_contain(context, expected_text):
        found = False
        for item in context.query_result:
            # Check if any value in the result item's dictionary contains the expected text
            if any(expected_text in str(val) for val in item.values()):
                found = True
                break
        assert found, f"Expected text '{expected_text}' not found in query result: {context.query_result}"

    @runner.step(r'^the query result for the KnowledgeBase should not contain "(.*)"$')
    def query_result_should_not_contain(context, unexpected_text):
        found = False
        for item in context.query_result:
            if any(unexpected_text in str(val) for val in item.values()):
                found = True
                break
        assert not found, f"Unexpected text '{unexpected_text}' found in query result: {context.query_result}"

    @runner.step(r'^a user prompt "(.*)"$')
    def given_user_prompt(context, prompt):
        context.user_prompt = prompt

    @runner.step(r'^a retrieved context from TissDB: "(.*)"$')
    def given_retrieved_context(context, retrieved_context):
        context.retrieved_context = retrieved_context

    @runner.step(r'^the TissLM augments the prompt with the retrieved context$')
    def tisslm_augments_prompt(context):
        context.final_prompt = f"context: {{{context.retrieved_context}}} question: {{{context.user_prompt}}}"

    @runner.step(r'the final prompt sent to the language model should be:\s*"""\s*([\s\S]*?)\s*"""')
    def final_prompt_should_be(context, expected_prompt):
        # Normalize whitespace and newlines for comparison
        normalized_actual = re.sub(r'\s+', ' ', context.final_prompt).strip()
        normalized_expected = re.sub(r'\s+', ' ', expected_prompt).strip()
        assert normalized_actual == normalized_expected

    @runner.step(r'^a simulated Sinew client creates a document with ID "(.*)" and content (.*) in "(.*)"$')
    def sinew_creates_document(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200

    @runner.step(r'^a simulated Sinew client deletes the document with ID "(.*)" from "(.*)"$')
    def sinew_deletes_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{collection_name}/{doc_id}")
        assert response.status_code == 204
