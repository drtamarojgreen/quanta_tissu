import json

def find_doc_with_field_value(context, field, value):
    assert 'query_result' in context, "No query result found in context"
    for doc in context.get('query_result', []):
        if field in doc and doc[field] == value:
            return True
    return False

def register_steps(runner):
    @runner.step(r'the document should contain the field "(.*)" with the date "(.*)"')
    def step_impl_date(context, field, value):
        assert find_doc_with_field_value(context, field, value), \
            f"Could not find a document with field '{field}' having date value '{value}'"

    @runner.step(r'the document should contain the field "(.*)" with the time "(.*)"')
    def step_impl_time(context, field, value):
        assert find_doc_with_field_value(context, field, value), \
            f"Could not find a document with field '{field}' having time value '{value}'"

    @runner.step(r'the document should contain the field "(.*)" with the datetime "(.*)"')
    def step_impl_datetime(context, field, expected_value):
        assert 'query_result' in context, "No query result found in context"

        normalized_expected = expected_value.replace('T', ' ')

        for doc in context.get('query_result', []):
            if field in doc and isinstance(doc[field], str):
                # Normalize the actual value from the DB (e.g., handle 'T' separator)
                normalized_actual = doc[field].replace('T', ' ')
                if normalized_actual == normalized_expected:
                    return  # Found a match

        # If loop completes without finding a match, fail the test
        assert False, f"Could not find a document with field '{field}' having datetime value '{expected_value}'"

    @runner.step(r'the result should contain a document with the field "(.*)" having the value "(.*)"')
    def step_impl_field_value(context, field, value):
        try:
            expected_value = json.loads(value)
        except (json.JSONDecodeError, TypeError):
            expected_value = value

        assert find_doc_with_field_value(context, field, expected_value), \
            f"Could not find a document with field '{field}' having value '{expected_value}'"
