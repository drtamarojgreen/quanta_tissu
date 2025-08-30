import requests
import json

BASE_URL = "http://localhost:9876"
DB_NAME = "testdb" # Use a consistent test database

def get_headers(context):
    headers = {}
    if 'auth_token' in context:
        headers['Authorization'] = f"Bearer {context['auth_token']}"
    return headers

def register_steps(runner):

    @runner.step(r'I insert the following documents into "(.*)":')
    def insert_documents(context, collection_name):
        headers = get_headers(context)
        for row in context.table:
            doc_id = row['id']
            content_str = row['content']
            try:
                content = json.loads(content_str)
            except json.JSONDecodeError:
                content = json.loads(content_str.replace("'", '"'))

            response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content, headers=headers)
            assert response.status_code == 200, f"Failed to insert doc {doc_id}. Status: {response.status_code}, Body: {response.text}"

    @runner.step(r'^When I list all documents in "(.*)"$')
    def list_all_documents(context, collection_name):
        headers = get_headers(context)
        query = {"query": "SELECT * FROM " + collection_name}
        response = requests.post(f"{BASE_URL}/{DB_NAME}/{collection_name}/_query", json=query, headers=headers)
        assert response.status_code == 200
        context['document_list'] = response.json()

    @runner.step(r'^Then the document list should contain a document with ID "(.*)"$')
    def document_list_should_contain(context, doc_id):
        found = any(doc.get('_id') == doc_id for doc in context.get('document_list', []))
        assert found, f"Document with ID {doc_id} not found in list."

    @runner.step(r'^When I attempt to create a collection named "(.*)"$')
    def attempt_create_collection(context, collection_name):
        headers = get_headers(context)
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}", headers=headers)
        context['response_status_code'] = response.status_code

    @runner.step(r'^Then the operation should be successful with status code (.*)$')
    def operation_should_be_successful(context, status_code):
        assert context['response_status_code'] in [200, 201, 204]

    @runner.step(r'^When I attempt to delete the document with ID "(.*)" from "(.*)"$')
    def attempt_delete_document(context, doc_id, collection_name):
        headers = get_headers(context)
        response = requests.delete(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", headers=headers)
        context['response_status_code'] = response.status_code

    @runner.step(r'^Then the operation should fail with status code (.*)$')
    def operation_should_fail(context, status_code):
        assert context['response_status_code'] == int(status_code) and context['response_status_code'] not in [200, 201, 204]

    @runner.step(r'^Then the document list should contain "(.*)"$')
    def then_document_list_should_contain(context, doc_id):
        found = False
        for doc in context.get('document_list', []):
            if doc.get('_id') == doc_id:
                found = True
                break
        assert found, f"Document with ID {doc_id} not found in list."

    @runner.step(r'^And the document list should contain "(.*)"$')
    def and_document_list_should_contain(context, doc_id):
        then_document_list_should_contain(context, doc_id)
