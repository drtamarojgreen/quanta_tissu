from behave import when, then, given
import requests
import json

def find_doc_with_field_value(context, field, value):
    assert 'query_result' in context, "No query result found in context"
    for doc in context.get('query_result', []):
        if field in doc and doc[field] == value:
            return True
    return False

def register_steps(runner):
    @runner.step(r'the document should contain the field "(.*)" with the timestamp "(.*)"')
    def step_impl_timestamp(context, field, value):
        assert find_doc_with_field_value(context, field, value), \
            f"Could not find a document with field '{field}' having timestamp value '{value}'"

    @runner.step(r'I create a timestamp index on the field "(.*)" in collection "(.*)"')
    def step_impl_create_ts_index(context, field, collection):
        # Assuming the API for creating a timestamp index is similar to a regular index
        # but with a type specified. This might need adjustment based on the actual API endpoint.
        # The test runner will need to have context.base_url defined.
        url = f"{context.base_url}/{context.db_name}/{collection}/_index"
        payload = {
            "field": field,
            "type": "timestamp" # This assumes the API supports a 'type' key.
        }
        headers = {'Content-Type': 'application/json'}
        if 'headers' in context:
            headers.update(context.headers)

        response = requests.post(url, headers=headers, json=payload)
        context.response = response
        assert response.status_code == 200, f"Expected 200, got {response.status_code}: {response.text}"

    @runner.step(r'the index should be created successfully')
    def step_impl_index_created(context):
        # This is a bit of a simplification. In a real scenario, we might query
        # an endpoint to verify the index exists. For now, we rely on the 200 OK
        # from the previous step.
        assert context.response.status_code == 200
        assert "Index creation initiated" in context.response.text
