import requests
import json
import time

BASE_URL = "http://localhost:9876"

def get_headers(context):
    headers = {}
    if 'transaction_id' in context:
        headers['X-Transaction-ID'] = str(context['transaction_id'])
    if 'auth_token' in context:
        headers['Authorization'] = f"Bearer {context['auth_token']}"
    print(f"DEBUG: Generated headers: {headers}")
    return headers

def register_steps(runner):

    @runner.step(r'a running TissDB instance')
    def running_tissdb_instance(context):
        context['db_name'] = "testdb"
        # Set a default admin token for all subsequent requests in the tests.
        # Security-specific tests can override or remove this.
        context['auth_token'] = "static_test_token"
        headers = get_headers(context)

        # Ensure a clean slate. These requests now require authentication.
        requests.delete(f"{BASE_URL}/{context['db_name']}", headers=headers)
        response = requests.put(f"{BASE_URL}/{context['db_name']}", headers=headers)
        assert response.status_code in [201, 200, 409], f"Failed to create database. Status: {response.status_code}, Body: {response.text}"

    @runner.step(r'^Given a TissDB instance$')
    def tissdb_instance(context):
        # This is an alias for the "a running TissDB instance" step.
        running_tissdb_instance(context)

    @runner.step(r'a collection named "(.*)"( exists)?')
    def collection_exists(context, collection_name, exists_word=None):
        context['collection_name'] = collection_name
        headers = get_headers(context)
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}", headers=headers)
        assert response.status_code in [201, 200, 409], f"Failed to create collection. Status: {response.status_code}, Body: {response.text}"

    @runner.step(r'I create a collection named "(.*)"')
    def create_collection(context, collection_name):
        collection_exists(context, collection_name)

    @runner.step(r'the collection "(.*)" should exist')
    def collection_should_exist(context, collection_name):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{context['db_name']}/_collections", headers=headers)
        if response.status_code != 200:
            print(f"DEBUG: GET /_collections failed with status {response.status_code}")
            print(f"DEBUG: Response body: {response.text}")
        assert response.status_code == 200
        assert collection_name in response.json()

    @runner.step(r'the collection "(.*)" should not exist')
    def collection_should_not_exist(context, collection_name):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{context['db_name']}/_collections", headers=headers)
        assert response.status_code == 200
        assert collection_name not in response.json()

    @runner.step(r'I delete the collection "(.*)"')
    def delete_collection(context, collection_name):
        headers = get_headers(context)
        response = requests.delete(f"{BASE_URL}/{context['db_name']}/{collection_name}", headers=headers)
        assert response.status_code in [204, 404]

    @runner.step(r'a document with ID "(.*)" and content (.*) in "(.*)"')
    def create_document_with_content(context, doc_id, content, collection_name):
        headers = get_headers(context)
        # The content can be single-quoted, double-quoted, or not quoted at all if it's a single token
        if content.startswith("'") and content.endswith("'"):
            content = content[1:-1]
        elif content.startswith('"') and content.endswith('"'):
            content = content[1:-1]

        try:
            payload = json.loads(content)
        except json.JSONDecodeError:
            payload = {"content": content}

        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", json=payload, headers=headers)
        assert response.status_code == 200, f"Failed to create document. Status: {response.status_code}, Body: {response.text}"
        context['doc_id'] = doc_id

    @runner.step(r'I create a document with ID "(.*)" and content (.*) in "(.*)"')
    def i_create_document_with_content(context, doc_id, content, collection_name):
        create_document_with_content(context, doc_id, content, collection_name)

    @runner.step(r'I update the document with ID "(.*)" with content (.*) in "(.*)"')
    def update_document_with_content(context, doc_id, content, collection_name):
        create_document_with_content(context, doc_id, content, collection_name)

    @runner.step(r'the document with ID "(.*)" in "(.*)" should have content (.*)')
    def document_should_have_content(context, doc_id, collection_name, expected_content_str):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 200
        actual_content = response.json()

        if expected_content_str.startswith("'") and expected_content_str.endswith("'"):
            expected_content_str = expected_content_str[1:-1]
        elif expected_content_str.startswith('"') and expected_content_str.endswith('"'):
            expected_content_str = expected_content_str[1:-1]

        expected_content = json.loads(expected_content_str)

        for key, value in expected_content.items():
            assert key in actual_content
            assert actual_content[key] == value, f"Mismatch for key '{key}': expected '{value}', got '{actual_content[key]}'"

    @runner.step(r'I delete the document with ID "(.*)" from "(.*)"')
    def delete_document(context, doc_id, collection_name):
        headers = get_headers(context)
        response = requests.delete(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 204

    @runner.step(r'the document with ID "(.*)" in "(.*)" should not exist')
    def document_should_not_exist(context, doc_id, collection_name):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 404

    @runner.step(r'the document with ID "(.*)" in "(.*)" should exist')
    def document_should_exist(context, doc_id, collection_name):
        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", headers=headers)
        assert response.status_code == 200, f"Document {doc_id} does not exist. Status: {response.status_code}"

    @runner.step(r'I execute the TissQL query "(.*)" on "(.*)"')
    def execute_tissql_query(context, query_string, collection_name):
        data = {"query": query_string}
        headers = get_headers(context)
        # Use the /_query endpoint
        response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data, headers=headers)
        context['response'] = response
        if response.ok:
            context['query_result'] = response.json()
        else:
            context['query_result'] = []

    @runner.step(r'the query result should contain "(.*)"')
    def query_result_should_contain(context, expected_value):
        assert 'query_result' in context, "No query result found in context"
        found = any(expected_value in (str(v) for v in item.values()) for item in context['query_result'])
        assert found, f"Expected value '{expected_value}' not found in query result: {context['query_result']}"

    @runner.step(r'the query result should not contain "(.*)"')
    def query_result_should_not_contain(context, unexpected_value):
        assert 'query_result' in context, "No query result found in context"
        found = any(unexpected_value in (str(v) for v in item.values()) for item in context['query_result'])
        assert not found, f"Unexpected value '{unexpected_value}' found in query result: {context['query_result']}"

    @runner.step(r'the query result should be empty')
    def query_result_should_be_empty(context):
        assert 'query_result' in context, "No query result found in context"
        assert len(context['query_result']) == 0, f"Expected empty result, but got: {context['query_result']}"

    @runner.step(r'the query should succeed')
    def query_should_succeed(context):
        assert 'response' in context, "No response found in context"
        assert context['response'].ok, f"Query failed with status {context['response'].status_code}: {context['response'].text}"

    @runner.step(r'I begin a transaction')
    def begin_transaction(context):
        headers = get_headers(context)
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_begin", headers=headers)
        assert response.status_code == 200
        context['transaction_id'] = response.json()['transaction_id']

    @runner.step(r'I commit the transaction')
    def commit_transaction(context):
        headers = get_headers(context)
        data = {'transaction_id': context['transaction_id']}
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_commit", json=data, headers=headers)
        assert response.status_code in [200, 204]
        del context['transaction_id']

    @runner.step(r'I rollback the transaction')
    def rollback_transaction(context):
        headers = get_headers(context)
        data = {'transaction_id': context['transaction_id']}
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_rollback", json=data, headers=headers)
        assert response.status_code in [200, 204]
        del context['transaction_id']
