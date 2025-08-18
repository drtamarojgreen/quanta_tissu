import requests
import json
import re

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb" # Use a consistent test database

def register_steps(step): # Changed from runner
    @step(r'And I insert the following documents into "(.*)":$') # Changed from runner.step
    def insert_documents_from_table(context, collection_name, table_lines):
        headers_line = table_lines[0]
        headers = [h.strip() for h in headers_line.strip().strip('|').split('|')]

        for i in range(1, len(table_lines)):
            row_line = table_lines[i]
            values = [v.strip() for v in row_line.strip().strip('|').split('|')]
            doc = dict(zip(headers, values))

            doc_id = doc['id']
            # The content might be a JSON string, so we parse it.
            try:
                content = json.loads(doc['content'])
            except json.JSONDecodeError:
                # If it's not a valid JSON, treat it as a plain string.
                content = {'value': doc['content']}


            response = requests.put(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}", json=content)
            assert response.status_code == 200, f"Failed to insert doc {doc_id}. Status: {response.status_code}, Text: {response.text}"

    @step(r'Then the document with ID "(.*)" in "(.*)" should exist$') # Changed from runner.step
    def document_should_exist(context, doc_id, collection_name):
        response = requests.get(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}")
        assert response.status_code == 200, f"Expected document '{doc_id}' to exist, but it does not (status code: {response.status_code})."
