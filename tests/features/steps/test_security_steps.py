import requests
import json

BASE_URL = "http://localhost:9876"

# This is a duplicate of the helper in test_database_steps.py
# In a real-world scenario, this would be moved to a shared utility module.
def get_headers(context):
    headers = {}
    if 'transaction_id' in context:
        headers['X-Transaction-ID'] = str(context['transaction_id'])
    if 'auth_token' in context:
        headers['Authorization'] = f"Bearer {context['auth_token']}"
    return headers

def register_steps(runner):

    @runner.step(r'I make a GET request to the protected endpoint "(.*)" without a token')
    def get_protected_no_token(context, path):
        if 'auth_token' in context:
            del context['auth_token'] # Ensure no token is sent
        context['response'] = requests.get(f"{BASE_URL}{path}", headers=get_headers(context))

    @runner.step(r'I make a GET request to the protected endpoint "(.*)" with an invalid token')
    def get_protected_invalid_token(context, path):
        context['auth_token'] = "this_is_not_a_valid_token"
        context['response'] = requests.get(f"{BASE_URL}{path}", headers=get_headers(context))

    @runner.step(r'I make a GET request to the protected endpoint "(.*)" with a valid "admin" token')
    def get_protected_admin_token(context, path):
        context['auth_token'] = "static_test_token"
        context['response'] = requests.get(f"{BASE_URL}{path}", headers=get_headers(context))

    @runner.step(r'a user with a valid "read_only" token')
    def user_with_readonly_token(context):
        context['auth_token'] = "read_only_token"

    @runner.step(r'I attempt to DELETE the database')
    def attempt_delete_database(context):
        # We assume the default db name is 'testdb' from the common steps
        context['response'] = requests.delete(f"{BASE_URL}/{context['db_name']}", headers=get_headers(context))

    @runner.step(r'the response status code should be (\d+)')
    def response_status_code(context, expected_code):
        assert context['response'].status_code == int(expected_code), \
            f"Expected status code {expected_code}, but got {context['response'].status_code}. Body: {context['response'].text}"

    @runner.step(r'the response body should contain "(.*)"')
    def response_body_contains(context, expected_text):
        assert expected_text in context['response'].text, \
            f"Expected '{expected_text}' not found in response body: {context['response'].text}"

    # Steps for Encryption Testing
    @runner.step(r'I create a document with ID "(.*)" and content \'(.*)\' in "(.*)"')
    def create_doc_with_single_quotes(context, doc_id, content, collection_name):
        # This is needed because the default step doesn't handle single-quoted JSON well.
        headers = get_headers(context)
        payload = json.loads(content)
        response = requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}", json=payload, headers=headers)
        assert response.status_code == 200, f"Failed to create document. Status: {response.status_code}, Body: {response.text}"
        context['doc_id'] = doc_id

    @runner.step(r'I inspect the raw data files for "(.*)"')
    def inspect_raw_data_files(context, collection_name):
        # This is a bit of a hack. We assume the server is running from the root of the repo.
        # The data is stored in tissdb_data/<db_name>/<collection_name>
        import os
        import time
        data_path = os.path.join("tissdb_data", context['db_name'], collection_name)
        context['raw_data_content'] = ""
        # We need to force a flush from memtable to sstable.
        # There's no API for this, so we'll just add more data to trigger it.
        # This is fragile and a proper test API would be better.
        headers = get_headers(context)
        for i in range(200): # Assuming memtable size is less than 200
             requests.put(f"{BASE_URL}/{context['db_name']}/{collection_name}/doc_{i}", json={"data": f"dummy{i}"}, headers=headers)

        time.sleep(2) # Give time for compaction/flush to occur

        for filename in os.listdir(data_path):
            if filename.endswith(".db"):
                with open(os.path.join(data_path, filename), 'rb') as f:
                    # Read raw bytes to avoid decoding errors
                    context['raw_data_content'] += f.read().decode('latin-1')


    @runner.step(r'the plaintext "(.*)" should not be found')
    def plaintext_should_not_be_found(context, plaintext):
        assert 'raw_data_content' in context, "Raw data was not inspected."
        assert plaintext not in context['raw_data_content'], f"Found plaintext '{plaintext}' in raw data file!"
