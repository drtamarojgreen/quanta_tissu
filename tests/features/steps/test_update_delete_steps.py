import requests
import json
import re

BASE_URL = "http://localhost:8080"

def register_steps(runner):
    # This step is designed to handle the "And I insert the following documents" step
    # in the background of the update_delete_queries.feature file.
    @runner.step(r'^And I insert the following documents into "(.*)":$')
    def insert_documents_from_table(context, collection_name, table_lines):
        # This is a bit of a hack due to the custom BDD runner.
        # It seems the runner passes the table as a list of raw string lines.
        headers_line = table_lines[0]
        headers = [h.strip() for h in headers_line.strip().strip('|').split('|')]

        for i in range(1, len(table_lines)):
            row_line = table_lines[i]
            values = [v.strip() for v in row_line.strip().strip('|').split('|')]
            doc = dict(zip(headers, values))

            doc_id = doc['id']
            content = json.loads(doc['content'])

            response = requests.put(f"{BASE_URL}/{collection_name}/{doc_id}", json=content)
            assert response.status_code == 200, f"Failed to insert doc {doc_id}. Status: {response.status_code}"

    @runner.step(r'^Then the document with ID "(.*)" in "(.*)" should exist$')
    def document_should_exist(context, doc_id, collection_name):
        response = requests.get(f"{BASE_URL}/{collection_name}/{doc_id}")
        assert response.status_code == 200, f"Expected document '{doc_id}' to exist, but it does not (status code: {response.status_code})."

    # The following step is defined in `test_database_steps.py`
    # When I execute the TissQL query "(.*)" on "(.*)"
    # We will rely on the runner to have it registered.

    # The following step is also in `test_database_steps.py`
    # Then the document with ID "(.*)" in "(.*)" should have content (.*)
    # We will rely on the runner to have it registered.

    # Also from `test_database_steps.py`
    # Then the document with ID "(.*)" in "(.*)" should not exist
    # We will rely on the runner to have it registered.
