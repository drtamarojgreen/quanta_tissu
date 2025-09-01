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
    def step_impl_datetime(context, field, value):
        assert find_doc_with_field_value(context, field, value), \
            f"Could not find a document with field '{field}' having datetime value '{value}'"

    @runner.step(r'the result should contain a document with the field "(.*)" having the value "(.*)"')
    def step_impl_field_value(context, field, value):
        # This is a more generic step, but we can make it work for our case.
        # The value might be a string or number, so we handle both.
        try:
            # Try to convert value to a number if it looks like one
            expected_value = json.loads(value)
        except (json.JSONDecodeError, TypeError):
            # Otherwise, treat it as a string
            expected_value = value

        assert find_doc_with_field_value(context, field, expected_value), \
            f"Could not find a document with field '{field}' having value '{expected_value}'"
