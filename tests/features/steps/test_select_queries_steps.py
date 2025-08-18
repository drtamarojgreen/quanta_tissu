import requests
import json
from typing import List, Dict, Any

BASE_URL = "http://localhost:8080"

def register_steps(runner):

    @runner.step(r'^I create the following documents in "(.*)":$')
    def create_multiple_documents(context, collection_name, table):
        headers = list(table.headings)
        for row in table.rows:
            doc_id = None
            doc_content = {}
            for i, cell in enumerate(row.cells):
                header = headers[i]
                # Try to convert to number if possible
                try:
                    value = int(cell)
                except ValueError:
                    try:
                        value = float(cell)
                    except ValueError:
                        value = cell

                if header == "_id":
                    doc_id = value
                else:
                    doc_content[header] = value

            if doc_id is None:
                raise ValueError("'_id' column is required to create documents.")

            response = requests.put(f"{BASE_URL}/{collection_name}/{doc_id}", json=doc_content)
            assert response.status_code in [200, 201], f"Failed to create document {doc_id}. Status: {response.status_code}"

    @runner.step(r'^the query result should have (\d+) documents$')
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
        assert False, f"No document found with '{key}' = '{value}'"

    @runner.step(r'^the query result should contain a document with "(.*)" = (.*)$')
    def result_should_contain_doc_with_kv_numeric(context, key, value):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        # Convert value to numeric type
        try:
            num_value = int(value)
        except ValueError:
            num_value = float(value)

        for doc in result:
            if doc.get(key) == num_value:
                return
        assert False, f"No document found with '{key}' = {num_value}"

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
            if doc.get(key1) == value1 and doc.get(key2) == num_value2:
                return
        assert False, f"No document found with '{key1}' = '{value1}' and '{key2}' = {num_value2}"

    @runner.step(r'^the query result should be empty$')
    def result_should_be_empty(context):
        result = context.get('query_result')
        assert result is not None, "Query result not found in context"
        assert len(result) == 0, f"Expected empty result, but found {len(result)} documents"
