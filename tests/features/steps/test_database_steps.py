import requests
import json
import time

BASE_URL = "http://localhost:8080"

def register_steps(runner):

    @runner.step(r'^Given a running TissDB instance$')
    def running_tissdb_instance(context):
        context['db_name'] = "testdb"
        # Try to delete the database to ensure a clean state, ignore errors if it doesn't exist
        requests.delete(f"{BASE_URL}/{context['db_name']}")

        # Create the database for the test
        response = requests.put(f"{BASE_URL}/{context['db_name']}")
        if response.status_code == 500 and 'already exists' in response.text:
            # This is fine, it means the previous delete failed but the DB is there.
            pass
        else:
            # The server should return 201 Created
            assert response.status_code == 201, f"Failed to create database. Status: {response.status_code}, Body: {response.text}"

        # Check if the server is healthy
        try:
            response = requests.get(f"{BASE_URL}/_health")
            response.raise_for_status()
        except requests.exceptions.RequestException as e:
            raise Exception(f"TissDB instance is not responsive: {e}")

    @runner.step(r'^When I create a collection named "(.*)"$')
    def create_collection(context, collection_name):
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}")
        assert response.status_code == 201
        context['collection_name'] = collection_name

    @runner.step(r'^Then the collection "(.*)" should exist$')
    def collection_should_exist(context, collection_name):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/_collections")
        assert response.status_code == 200
        collections = response.json()
        assert collection_name in collections

    @runner.step(r'^When I delete the collection "(.*)"$')
    def delete_collection(context, collection_name):
        response = requests.delete(f"{BASE_URL}/{context['db_name']}/{collection_name}")
        assert response.status_code == 204

    @runner.step(r'a TissDB collection named "(.*)" is available for TissLM')
    def collection_is_available(context, collection_name):
        db_name = context.get('db_name', 'testdb')
        response = requests.put(f"{BASE_URL}/{db_name}/{collection_name}")
        # This can be 200 (created) or 500/409 (if it exists).
        # We just want to make sure it exists for the test.
        assert response.status_code in [200, 500, 409]

    @runner.step(r'a document with ID "(.*)" and content (.*) in "(.*)"')
    def create_document_with_content(context, doc_id, content, collection_name):
        db_name = context.get('db_name', 'testdb')
        url = f"{BASE_URL}/{db_name}/{collection_name}/{doc_id}"
        response = requests.put(url, json=json.loads(content))
        response.raise_for_status()

    @runner.step(r'a simulated Sinew client creates a document with ID "(.*)" and content (.*) in "(.*)"')
    def sinew_create_document(context, doc_id, content, collection_name):
        # This is the same as the step above, just with a different sentence.
        create_document_with_content(context, doc_id, content, collection_name)

    @runner.step(r'^Then the collection "(.*)" should not exist$')
    def collection_should_not_exist(context, collection_name):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/_collections")
        assert response.status_code == 200
        collections = response.json()
        assert collection_name not in collections

    @runner.step(r'^And a collection named "(.*)" exists$')
    def collection_exists(context, collection_name):
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}")
        assert response.status_code in [201, 200]
        context['collection_name'] = collection_name

    @runner.step(r'^When I create a document with ID "(.*)" and content (.*) in "(.*)"$')
    def create_document_with_id(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200
        context['doc_id'] = doc_id
        context['doc_content'] = content

    @runner.step(r'^Then the document with ID "(.*)" in "(.*)" should have content (.*)$')
    def document_should_have_content(context, doc_id, collection_name, expected_content_str):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
        assert response.status_code == 200
        actual_content = response.json()
        expected_content = json.loads(expected_content_str)
        for key, value in expected_content.items():
            assert key in actual_content
            assert actual_content[key] == value, f"Mismatch for key '{key}': expected '{value}', got '{actual_content[key]}'"

    @runner.step(r'^When I update the document with ID "(.*)" with content (.*) in "(.*)"$')
    def update_document(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200
        context['doc_id'] = doc_id
        context['doc_content'] = content

    @runner.step(r'^When I delete the document with ID "(.*)" from "(.*)"$')
    def delete_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
        assert response.status_code == 204

    @runner.step(r'^Then the document with ID "(.*)" in "(.*)" should not exist$')
    def document_should_not_exist(context, doc_id, collection_name):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
        assert response.status_code == 404

    @runner.step(r'^Given a document with ID "(.*)" exists in TissDB$')
    def given_document_exists(context, doc_id):
        context['collection_name'] = "documents"
        collection_exists(context, context['collection_name'])
        content = {"data": f"This is document {doc_id}"}
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{context['collection_name']}/{doc_id}", json=content)
        assert response.status_code == 200
        context['doc_id'] = doc_id

    @runner.step(r'^When I execute the TissQL query "(.*)" on "(.*)"$')
    def execute_tissql_query(context, query_string, collection_name):
        data = {"query": query_string}
        response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
        assert response.status_code == 200
        context['query_result'] = response.json()

    @runner.step(r'^Then the query result should contain "(.*)"$')
    def query_result_should_contain(context, expected_value):
        found = False
        for item in context['query_result']:
            if expected_value in item.values():
                found = True
                break
        assert found

    @runner.step(r'^And the query result should not contain "(.*)"$')
    def query_result_should_not_contain(context, unexpected_value):
        found = False
        for item in context['query_result']:
            if unexpected_value in item.values():
                found = True
                break
        assert not found

    @runner.step(r'^When I begin a transaction$')
    def begin_transaction(context):
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_begin")
        assert response.status_code == 200

    @runner.step(r'^And I commit the transaction$')
    def commit_transaction(context):
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_commit")
        assert response.status_code in [200, 204]

    @runner.step(r'^And I rollback the transaction$')
    def rollback_transaction(context):
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_rollback")
        assert response.status_code in [200, 204]

    @runner.step(r'^And I delete the collection "(.*)"$')
    def and_delete_collection(context, collection_name):
        delete_collection(context, collection_name)

    @runner.step(r'^And I create a document with ID "(.*)" and content (.*) in "(.*)"$')
    def and_create_document_with_id(context, doc_id, content_str, collection_name):
        create_document_with_id(context, doc_id, content_str, collection_name)

    @runner.step(r'^And the query result should contain "(.*)"$')
    def and_query_result_should_contain(context, expected_value):
        query_result_should_contain(context, expected_value)

    @runner.step(r'^And the document with ID "(.*)" in "(.*)" should have content (.*)$')
    def and_document_should_have_content(context, doc_id, collection_name, expected_content_str):
        document_should_have_content(context, doc_id, collection_name, expected_content_str)
