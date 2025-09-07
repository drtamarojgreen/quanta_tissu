import requests
import json
import time
from tests.features.steps.test_database_steps import BASE_URL, get_headers

def register_steps(runner):

    @runner.step(r'the collection "(.*)" is empty')
    def collection_is_empty(context, collection_name):
        headers = get_headers(context)
        # It's easier to just delete and recreate the collection to ensure it's empty
        requests.delete(f"{BASE_URL}/{context.db_name}/{collection_name}", headers=headers)
        response = requests.put(f"{BASE_URL}/{context.db_name}/{collection_name}", headers=headers)
        assert response.status_code in [201, 200, 409], f"Failed to create collection. Status: {response.status_code}, Body: {response.text}"

    @runner.step(r'I execute the TissQL query "(.*)"')
    def execute_tissql_query(context, query_string):
        data = {"query": query_string}
        headers = get_headers(context)
        # The general /_query endpoint doesn't need a collection name in the URL
        response = requests.post(f"{BASE_URL}/{context.db_name}/_query", json=data, headers=headers)
        context.response = response
        if response.ok:
            context.query_result = response.json()
        else:
            context.query_result = []

    @runner.step(r'the collection "(.*)" should contain a document with (.*)')
    def collection_should_contain_doc_with_data(context, collection_name, data_string):
        headers = get_headers(context)
        # To verify the insert, we need to find the document. Since we don't know the ID,
        # we have to query for it. This assumes SELECT works.

        # Parse the data_string: "name" = "Alice", "age" = 30
        assignments = [a.strip() for a in data_string.split(',')]
        filters = []
        for assign in assignments:
            key, value = [v.strip() for v in assign.split('=')]
            # Remove quotes from keys if they exist
            if key.startswith('"') and key.endswith('"'):
                key = key[1:-1]
            filters.append(f"{key} = {value}")

        where_clause = " AND ".join(filters)
        query = f"SELECT * FROM {collection_name} WHERE {where_clause}"

        data = {"query": query}
        response = requests.post(f"{BASE_URL}/{context.db_name}/_query", json=data, headers=headers)

        assert response.ok, f"Failed to query for document. Status: {response.status_code}, Body: {response.text}"
        results = response.json()
        assert len(results) > 0, f"No document found in '{collection_name}' with data: {data_string}"

    @runner.step(r'the query should fail with a syntax error')
    def query_should_fail_with_syntax_error(context):
        assert not context.response.ok
        # This is a guess at the error message. May need to be adjusted.
        assert "syntax error" in context.response.text.lower()

    @runner.step(r'the query should fail with a mismatched column/value count error')
    def query_should_fail_with_mismatched_count_error(context):
        assert not context.response.ok
        # This is a guess at the error message. May need to be adjusted.
        assert "column count does not match value count" in context.response.text.lower()

    @runner.step(r'the query should succeed and create the collection')
    def query_succeeds_and_creates_collection(context):
        assert context.response.ok
        # Extract collection name from the query
        query = context.response.request.body.decode('utf-8')
        query_json = json.loads(query)
        tissql = query_json['query']
        parts = tissql.split()
        collection_name = parts[2]

        headers = get_headers(context)
        response = requests.get(f"{BASE_URL}/{context.db_name}/_collections", headers=headers)
        assert response.ok
        assert collection_name in response.json()
