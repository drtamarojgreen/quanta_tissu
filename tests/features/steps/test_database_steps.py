import requests
import json
import time

BASE_URL = "http://localhost:8080"

# The 'step' decorator is passed in by the runner in test_bdd.py
def register_steps(step):

    @step(r'Given a running TissDB instance')
    def running_tissdb_instance(context):
        context['db_name'] = "testdb"
        # Clean up any old test database and create a fresh one
        requests.delete(f"{BASE_URL}/{context['db_name']}")
        response = requests.put(f"{BASE_URL}/{context['db_name']}")
        assert response.status_code == 201, f"Failed to create test database: {response.text}"

        # Check health
        try:
            response = requests.get(f"{BASE_URL}/_health")
            response.raise_for_status()
        except requests.exceptions.RequestException as e:
            raise Exception(f"TissDB instance is not responsive: {e}")

    @step(r'When I create a collection named "(.*)"')
    def create_collection(context, collection_name):
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}")
        assert response.status_code == 201
        context['collection_name'] = collection_name

    @step(r'Then the collection "(.*)" should exist')
    def collection_should_exist(context, collection_name):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/_collections")
        assert response.status_code == 200
        collections = response.json()
        assert collection_name in collections

    @step(r'When I delete the collection "(.*)"')
    def delete_collection(context, collection_name):
        response = requests.delete(f"{BASE_URL}/{context['db_name']}/{collection_name}")
        assert response.status_code == 204

    @step(r'Then the collection "(.*)" should not exist')
    def collection_should_not_exist(context, collection_name):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/_collections")
        assert response.status_code == 200
        collections = response.json()
        assert collection_name not in collections

    @step(r'And a collection named "(.*)" exists')
    def collection_exists(context, collection_name):
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}")
        assert response.status_code in [201, 200]
        context['collection_name'] = collection_name

    @step(r'When I create a document with ID "(.*)" and content (.*) in "(.*)"')
    def create_document_with_id(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200
        context['doc_id'] = doc_id
        context['doc_content'] = content

    @step(r'Then the document with ID "(.*)" in "(.*)" should have content (.*)')
    def document_should_have_content(context, doc_id, collection_name, expected_content_str):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
        assert response.status_code == 200
        actual_content = response.json()
        expected_content = json.loads(expected_content_str)
        for key, value in expected_content.items():
            assert key in actual_content
            assert actual_content[key] == value

    @step(r'When I update the document with ID "(.*)" with content (.*) in "(.*)"')
    def update_document(context, doc_id, content_str, collection_name):
        content = json.loads(content_str)
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", json=content)
        assert response.status_code == 200
        context['doc_id'] = doc_id
        context['doc_content'] = content

    @step(r'When I delete the document with ID "(.*)" from "(.*)"$')
    def delete_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
        assert response.status_code == 204

    @step(r'Then the document with ID "(.*)" in "(.*)" should not exist$')
    def document_should_not_exist(context, doc_id, collection_name):
        response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
        assert response.status_code == 404

    @step(r'Given a document with ID "(.*)" exists in TissDB')
    def given_document_exists(context, doc_id):
        context['collection_name'] = "documents"
        collection_exists(context, context['collection_name'])
        content = {"data": f"This is document {doc_id}"}
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{context['collection_name']}/{doc_id}", json=content)
        assert response.status_code == 200
        context['doc_id'] = doc_id

    @step(r'When I execute the TissQL query "(.*)" on "(.*)"$')
    def execute_tissql_query(context, query_string, collection_name):
        data = {"query": query_string}
        response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
        assert response.status_code == 200
        context['query_result'] = response.json()

    @step(r'Then the query result should contain "(.*)"$')
    def query_result_should_contain(context, expected_value):
        found = False
        for item in context['query_result']:
            if expected_value in item.values():
                found = True
                break
        assert found

    @step(r'And the query result should not contain "(.*)"$')
    def query_result_should_not_contain(context, unexpected_value):
        found = False
        for item in context['query_result']:
            if unexpected_value in item.values():
                found = True
                break
        assert not found

    @step(r'When I begin a transaction$')
    def begin_transaction(context):
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_begin")
        assert response.status_code == 200

    @step(r'And I commit the transaction$')
    def commit_transaction(context):
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_commit")
        assert response.status_code == 200

    @step(r'And I rollback the transaction$')
    def rollback_transaction(context):
        response = requests.post(f"{BASE_URL}/{context['db_name']}/_rollback")
        assert response.status_code == 200
