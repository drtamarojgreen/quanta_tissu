import requests
import json
import re

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb"

def get_headers(context):
    headers = {}
    if 'transaction_id' in context:
        headers['X-Transaction-ID'] = str(context.transaction_id)
    return headers

def register_steps(runner):

    @runner.step(r'the document with ID "(.*)" in "(.*)" should have content (.*)')
    def document_should_have_content(context, doc_id, collection_name, expected_content_str):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 200, f"Failed to get document {doc_id}. Status: {response.status_code}, Body: {response.text}"

        actual_content = response.json()

        # Handle single-quoted JSON which is common in the feature files
        if expected_content_str.startswith("'") and expected_content_str.endswith("'"):
            expected_content_str = expected_content_str.replace("'", '"')

        try:
            expected_content = json.loads(expected_content_str)
        except json.JSONDecodeError as e:
            raise ValueError(f"Invalid JSON in step definition: {expected_content_str}") from e

        for key, value in expected_content.items():
            assert key in actual_content
            # Type-agnostic comparison for simplicity in tests
            assert str(actual_content[key]) == str(value), f"Mismatch for key '{key}' in doc '{doc_id}': expected '{value}', got '{actual_content[key]}'"

    @runner.step(r'the document with ID "(.*)" in "(.*)" should exist')
    def document_should_exist(context, doc_id, collection_name):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 200, f"Expected document '{doc_id}' to exist, but it does not (status code: {response.status_code})."

    @runner.step(r'the document with ID "(.*)" in "(.*)" should not exist')
    def document_should_not_exist(context, doc_id, collection_name):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 404, f"Expected document '{doc_id}' not to exist, but it does (status code: {response.status_code})."
