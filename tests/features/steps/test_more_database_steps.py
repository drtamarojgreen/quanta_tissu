import requests
import json

BASE_URL = "http://localhost:8080"

def register_steps(runner):

    @runner.step(r'^When I insert the following documents into "(.*)":$')
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
            response = requests.put(f"{BASE_URL}/{collection_name}/{doc_id}", json=content)
            assert response.status_code == 200

    @runner.step(r'^When I list all documents in "(.*)"$')
    def list_all_documents(context, collection_name):
        response = requests.get(f"{BASE_URL}/{collection_name}/_all")
        assert response.status_code == 200
        context['document_list'] = response.json()

    @runner.step(r'^Then the document list should contain "(.*)"$')
    def document_list_should_contain(context, doc_id):
        assert doc_id in context['document_list']

    @runner.step(r'^When I attempt to create a collection named "(.*)"$')
    def attempt_create_collection(context, collection_name):
        response = requests.put(f"{BASE_URL}/{collection_name}")
        context['response_status_code'] = response.status_code

    @runner.step(r'^Then the operation should be successful with status code (.*)$')
    def operation_should_be_successful(context, status_code):
        assert context['response_status_code'] == int(status_code)

    @runner.step(r'^When I attempt to delete the document with ID "(.*)" from "(.*)"$')
    def attempt_delete_document(context, doc_id, collection_name):
        response = requests.delete(f"{BASE_URL}/{collection_name}/{doc_id}")
        context['response_status_code'] = response.status_code

    @runner.step(r'^Then the operation should fail with status code (.*)$')
    def operation_should_fail(context, status_code):
        assert context['response_status_code'] == int(status_code)
