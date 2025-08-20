import requests
import json
from typing import List, Dict, Any
import re

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb" # Use a consistent test database

def register_steps(runner):

    @runner.step(r'^When I execute the TissQL query "(.*)"$')
    def execute_tissql_query_from_string(context, query_string):
        # Extracts the collection name from the query, assuming "FROM [collection_name]"
        match = re.search(r'FROM\s+(\w+)', query_string, re.IGNORECASE)
        if not match:
            # Some queries like 'SELECT 1' don't have a FROM clause.
            # In this case, we can't determine a collection.
            # We'll need a different endpoint or handling for such queries.
            # For now, let's assume all test queries have a FROM clause.
            assert match, f"Could not find collection name in query: {query_string}"

        collection_name = match.group(1) if match else '_default'
        db_name = context.get('db_name', 'testdb')
        data = {"query": query_string}

        # Ensure the database and collection exist before querying
        requests.put(f"{BASE_URL}/{db_name}")
        if match:
            requests.put(f"{BASE_URL}/{db_name}/{collection_name}")

        # The query endpoint is on the database, not the collection
        response = requests.post(f"{BASE_URL}/{db_name}/_query", json=data)

        assert response.status_code == 200, f"Query failed: {response.status_code}, {response.text}"
        context['query_result'] = response.json()


    @runner.step(r'^Then the query result should have (\d+) documents?$')
    def result_should_have_n_documents(context, num_docs):
        num_docs = int(num_docs)
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        assert len(result) == num_docs, f"Expected {num_docs} documents, but found {len(result)}"

    @runner.step(r'^the query result should contain a document with "(.*)" = "(.*)"$')
    def result_should_contain_doc_with_kv_string(context, key, value):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        for doc in result:
            if doc.get(key) == value:
                return
        assert False, f"No document found with '{key}' = '{value}' in {context['query_result']}"

    @runner.step(r'^the query result should contain a document with "(.*)" = (.*)$')
    def result_should_contain_doc_with_kv_numeric(context, key, value):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        try:
            num_value = int(value)
        except ValueError:
            num_value = float(value)

        for doc in result:
            # Also check for string representation
            if doc.get(key) == num_value or str(doc.get(key)) == str(num_value):
                return
        assert False, f"No document found with '{key}' = {num_value} in {context['query_result']}"


    @runner.step(r'^each document in the result should have the fields (.*)$')
    def each_doc_should_have_fields(context, fields_str: str):
        fields: List[str] = json.loads(fields_str.replace("'", '"'))
        result: List[Dict[str, Any]] = context.get('query_result')
        assert result is not None, "Query result not found in context"
        for doc in result:
            for field in fields:
                assert field in doc, f"Document {doc} is missing field '{field}'"

    @runner.step(r'^each document in the result should not have the fields (.*)$')
    def each_doc_should_not_have_fields(context, fields_str: str):
        fields: List[str] = json.loads(fields_str.replace("'", '"'))
        result: List[Dict[str, Any]] = context.get('query_result')
        assert result is not None, "Query result not found in context"
        for doc in result:
            for field in fields:
                assert field not in doc, f"Document {doc} unexpectedly contains field '{field}'"

    @runner.step(r'^the query result should only contain documents where "(.*)" is "(.*)"$')
    def result_should_only_contain_docs_where(context, key, value):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        for doc in result:
            assert doc.get(key) == value, f"Document {doc} has wrong value for '{key}'. Expected '{value}', got '{doc.get(key)}'"

    @runner.step(r'^the query result should contain a document with "(.*)" = "(.*)" and "(.*)" = (.*)$')
    def result_should_contain_doc_with_two_kv(context, key1, value1, key2, value2):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        try:
            num_value2 = int(value2)
        except ValueError:
            num_value2 = float(value2)

        for doc in result:
            if doc.get(key1) == value1 and (doc.get(key2) == num_value2 or str(doc.get(key2)) == str(num_value2)):
                return
        assert False, f"No document found with '{key1}' = '{value1}' and '{key2}' = {num_value2} in {context['query_result']}"

    @runner.step(r'^Then the query result should be empty$')
    def result_should_be_empty(context):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        assert len(result) == 0, f"Expected empty result, but found {len(result)} documents"

    @runner.step(r'^the query result should not contain a document with "([^"]*)" = "([^"]*)"$')
    def query_result_not_contains_doc_with_kv_string(context, key, value):
        for doc in context['query_result']:
            if key in doc and doc[key] == value:
                assert False, f"Found unexpected document with '{key}' = '{value}'"

    @runner.step(r'^the documents should be in the following order for the key "([^"]*)": (.*)$')
    def documents_should_be_in_order(context, key, order_str):
        expected_order = json.loads(order_str.replace("'", '"'))
        actual_order = [doc.get(key) for doc in context['query_result']]
        assert actual_order == expected_order, f"Documents are not in the expected order. Expected {expected_order}, but got {actual_order}"
