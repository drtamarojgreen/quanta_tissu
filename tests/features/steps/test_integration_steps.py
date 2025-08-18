import requests
import json
import re

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb" # Use a consistent test database

def register_steps(step): # Changed

    @step(r'a TissDB collection named "(.*)" is available for TissLM$') # Changed
    def collection_available_for_tisslm(context, collection_name):
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}")
        assert response.status_code in [201, 200]
        context.collection_name = collection_name

    @step(r'the "(.*)" collection contains a document with ID "(.*)" and content (.*)$') # Changed
    def collection_contains_document(context, collection_name, doc_id, content_str):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200

    @step(r'the TissLM receives a user prompt "(.*)"$') # Changed
    def tisslm_receives_prompt(context, prompt):
        context.user_prompt = prompt

    @step(r'the TissLM KnowledgeBase formulates a TissQL query "(.*)"$') # Changed
    def knowledgebase_formulates_query(context, query):
        context.tissql_query = query

    @step(r'the KnowledgeBase executes the query against the "(.*)" collection$') # Changed
    def knowledgebase_executes_query(context, collection_name):
        data = {"query": context.tissql_query}
        response = requests.post(f"{BASE_URL}/{DB_NAME}/{collection_name}/_query", json=data)
        assert response.status_code == 200
        context.query_result = response.json()

    @step(r'the query result for the KnowledgeBase should contain "(.*)"$') # Changed
    def query_result_should_contain(context, expected_text):
        found = False
        for item in context.query_result:
            if any(expected_text in str(val) for val in item.values()):
                found = True
                break
        assert found, f"Expected text '{expected_text}' not found in query result: {context.query_result}"

    @step(r'the query result for the KnowledgeBase should not contain "(.*)"$') # Changed
    def query_result_should_not_contain(context, unexpected_text):
        found = False
        for item in context.query_result:
            if any(unexpected_text in str(val) for val in item.values()):
                found = True
                break
        assert not found, f"Unexpected text '{unexpected_text}' found in query result: {context.query_result}"

    @step(r'a user prompt "(.*)"$') # Changed
    def given_user_prompt(context, prompt):
        context.user_prompt = prompt

    @step(r'a retrieved context from TissDB: "(.*)"$') # Changed
    def given_retrieved_context(context, retrieved_context):
        context.retrieved_context = retrieved_context

    @step(r'the TissLM augments the prompt with the retrieved context$') # Changed
    def tisslm_augments_prompt(context):
        context.final_prompt = f"context: {{{context.retrieved_context}}} question: {{{context.user_prompt}}}"

    @step(r'the final prompt sent to the language model should be:\s*"""\s*([\s\S]*?)\s*"""') # Changed
    def final_prompt_should_be(context, expected_prompt):
        normalized_actual = re.sub(r'\s+', ' ', context.final_prompt).strip()
        normalized_expected = re.sub(r'\s+', ' ', expected_prompt).strip()
        assert normalized_actual == normalized_expected

    @step(r'a simulated Sinew client creates a document with ID "(.*)" and content (.*) in "(.*)"$') # Changed
    def sinew_creates_document(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200

    @step(r'a simulated Sinew client deletes the document with ID "(.*)" from "(.*)"$') # Changed
    def sinew_deletes_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}")
        assert response.status_code == 204
