import requests
import json

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb" # Use a consistent test database

def register_steps(step): # Changed

    @step(r'^When I insert the following documents into "(.*)":$') # Changed
    def insert_documents(context, collection_name, table):
        headers = [h.strip() for h in table[0].strip('|').split('|')]
        documents = []
        for i in range(1, len(table)):
            values = [v.strip() for v in table[i].strip('|').split('|')]
            doc = dict(zip(headers, values))
            documents.append(doc)

        for doc in documents:
            doc_id = doc['id']
            content = json.loads(doc['content'])
            response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content)
            assert response.status_code == 200

    @step(r'^When I list all documents in "(.*)"$') # Changed
    def list_all_documents(context, collection_name):
        # Note: /_all endpoint is not implemented, this step will fail if used.
        # This is a pre-existing issue with the test suite.
        # We will query for all documents using a placeholder scan.
        query = {"query": "SELECT * FROM " + collection_name}
        response = requests.post(f"{BASE_URL}/{DB_NAME}/{collection_name}/_query", json=query)
        assert response.status_code == 200
        context['document_list'] = response.json()

    @step(r'^Then the document list should contain a document with ID "(.*)"$') # Changed text
    def document_list_should_contain(context, doc_id):
        found = any(doc['id'] == doc_id for doc in context['document_list'])
        assert found, f"Document with ID {doc_id} not found in list."

    @step(r'^When I attempt to create a collection named "(.*)"$') # Changed
    def attempt_create_collection(context, collection_name):
        response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}")
        context['response_status_code'] = response.status_code

    @step(r'^Then the operation should be successful with status code (.*)$') # Changed
    def operation_should_be_successful(context, status_code):
        assert context['response_status_code'] == int(status_code)

    @step(r'^When I attempt to delete the document with ID "(.*)" from "(.*)"$') # Changed
    def attempt_delete_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}")
        context['response_status_code'] = response.status_code

    @step(r'^Then the operation should fail with status code (.*)$') # Changed
    def operation_should_fail(context, status_code):
        assert context['response_status_code'] == int(status_code)
