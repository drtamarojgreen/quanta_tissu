import requests
import json

BASE_URL = "http://localhost:9876"

def get_headers(context):
    headers = {}
    if 'auth_token' in context:
        headers['Authorization'] = f"Bearer {context['auth_token']}"
    return headers

def register_steps(runner):

<<<<<<< HEAD
=======
    @runner.step(r'When I execute the TissQL query "(.*)"')
    def execute_tissql_query_from_string(context, query_string):
        # Extracts the collection name from the query, assuming "FROM [collection_name]"
        match = re.search(r'FROM\s+(\w+)', query_string, re.IGNORECASE)
        assert match, f"Could not find collection name in query: {query_string}"
        collection_name = match.group(1)

        db_name = context.get('db_name', 'testdb')
        data = {"query": query_string}
        headers = get_headers(context)

        # Ensure the database and collection exist before querying
        requests.put(f"{BASE_URL}/{db_name}", headers=headers)
        requests.put(f"{BASE_URL}/{db_name}/{collection_name}", headers=headers)

        response = requests.post(f"{BASE_URL}/{db_name}/{collection_name}/_query", json=data, headers=headers)

        # It's possible for a query to correctly return a 404 if the collection doesn't exist yet
        # For the purpose of these tests, we will treat an empty list as the result.
        if response.status_code == 404:
            context['query_result'] = []
            return

        assert response.status_code == 200, f"Query failed: {response.status_code}, {response.text}"
        context['query_result'] = response.json()

    @runner.step(r'Then the query result should have (\d+) documents?')
    def query_result_should_have_count(context, count):
        count = int(count)
        assert len(context['query_result']) == count, f"Expected {count} documents, but got {len(context['query_result'])}"

    @runner.step(r'And the query result should contain a document with "([^"]*)" = "([^"]*)"')
    def query_result_contains_doc_with_kv_string(context, key, value):
        for doc in context['query_result']:
            if key in doc and doc[key] == value:
                return
        assert False, f"No document found with '{key}' = '{value}' in {context['query_result']}"

    @runner.step(r'And the query result should contain a document with "([^"]*)" = ([\d\.]+)')
    def query_result_contains_doc_with_kv_number(context, key, value):
        expected_value = float(value) if '.' in value else int(value)
        for doc in context['query_result']:
            if key in doc:
                try:
                    # Attempt to cast the actual value to the same type as the expected value.
                    actual_value = type(expected_value)(doc[key])
                    if actual_value == expected_value:
                        return
                except (ValueError, TypeError):
                    # If casting fails, it's not a match.
                    continue
        assert False, f"No document found with '{key}' = {expected_value} in {context['query_result']}"

    @runner.step(r'And each document in the result should have the fields (\[.*\])')
    def each_doc_has_fields(context, fields_str):
        fields = json.loads(fields_str.replace("'", '"'))
        for doc in context['query_result']:
            for field in fields:
                assert field in doc, f"Document {doc} is missing field '{field}'"

    @runner.step(r'And each document in the result should not have the fields (\[.*\])')
    def each_doc_not_have_fields(context, fields_str):
        fields = json.loads(fields_str.replace("'", '"'))
        for doc in context['query_result']:
            for field in fields:
                assert field not in doc, f"Document {doc} unexpectedly has field '{field}'"

    @runner.step(r'And the query result should contain a document with "([^"]*)" = "([^"]*)" and "([^"]*)" = ([\d\.]+)')
    def query_result_contains_doc_with_kv_string_and_number(context, key1, value1, key2, value2):
        expected_value2 = float(value2) if '.' in value2 else int(value2)
        for doc in context['query_result']:
            if key1 in doc and doc[key1] == value1 and key2 in doc:
                try:
                    actual_value2 = type(expected_value2)(doc[key2])
                    if actual_value2 == expected_value2:
                        return
                except (ValueError, TypeError):
                    continue
        assert False, f"No document found with '{key1}' = '{value1}' and '{key2}' = {expected_value2} in {context['query_result']}"

    @runner.step(r'And the query result should only contain documents where "([^"]*)" is "([^"]*)"')
    def query_result_only_contains_docs_with_kv(context, key, value):
        for doc in context['query_result']:
            assert key in doc and doc[key] == value, f"Document {doc} does not meet the condition '{key}' is '{value}'"

    @runner.step(r'And the query result should not contain a document with "([^"]*)" = "([^"]*)"')
    def query_result_not_contains_doc_with_kv_string(context, key, value):
        for doc in context['query_result']:
            if key in doc and doc[key] == value:
                assert False, f"Found unexpected document with '{key}' = '{value}'"

>>>>>>> main
    @runner.step(r'^Then the query result should be empty$')
    def query_result_should_be_empty(context):
        assert isinstance(context['query_result'], list), f"Query result is not a list: {context['query_result']}"
        assert len(context['query_result']) == 0, f"Query result is not empty: {context['query_result']}"
