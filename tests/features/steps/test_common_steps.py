import requests
import json

BASE_URL = "http://localhost:9876"

def get_headers(context):
    headers = {}
    if 'auth_token' in context:
        headers['Authorization'] = f"Bearer {context['auth_token']}"
    return headers

def register_steps(runner):
    @runner.step(r'^And I insert the following documents into "([^"]*)":')
    def insert_documents(context, collection_name, table):
        db_name = context.get('db_name', 'testdb')
        api_headers = get_headers(context)
        table_headers = [h.strip() for h in table[0].strip('|').split('|')]

        for i in range(1, len(table)):
            values = [v.strip() for v in table[i].strip('|').split('|')]
            doc = dict(zip(table_headers, values))

            doc_id = doc.get('id')
            if not doc_id:
                raise ValueError("Table in feature file must have an 'id' column.")

            # The content can be in a 'content' column or spread across other columns
            if 'content' in doc:
                content = json.loads(doc['content'])
            else:
                content = {k: v for k, v in doc.items() if k != 'id'}

            # Attempt to convert string values to numbers
            for key, value in content.items():
                if isinstance(value, str):
                    try:
                        content[key] = int(value)
                    except ValueError:
                        try:
                            content[key] = float(value)
                        except ValueError:
                            pass # Keep as string if it's not a valid number

            response = requests.put(f"{BASE_URL}/{db_name}/{collection_name}/{doc_id}", json=content, headers=api_headers)
            response.raise_for_status()
            assert response.status_code == 200, f"Failed to insert doc {doc_id}. Status: {response.status_code}, Body: {response.text}"
