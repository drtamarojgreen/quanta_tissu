import json
import urllib.request
import urllib.error
import os

TISSDB_HOST = os.environ.get('TISSDB_HOST', 'localhost')
TISSDB_PORT = os.environ.get('TISSDB_PORT', '9876')
TISSDB_URL = f'http://{TISSDB_HOST}:{TISSDB_PORT}'

def proxy_tissdb(handler, method, path, body):
    url = TISSDB_URL + path
    req = urllib.request.Request(url, data=body.encode('utf-8') if body else None, method=method)
    token = os.environ.get('TISSDB_TOKEN', 'static_test_token')
    req.add_header('Authorization', f'Bearer {token}')
    req.add_header('Content-Type', 'application/json')
    try:
        with urllib.request.urlopen(req) as response:
            handler.send_response(response.status)
            for k, v in response.getheaders():
                handler.send_header(k, v)
            handler.end_headers()
            handler.wfile.write(response.read())
    except urllib.error.HTTPError as e:
        handler.send_response(e.code)
        handler.end_headers()
        handler.wfile.write(e.read())
    except Exception as e:
        handler.send_response(500)
        handler.end_headers()
        handler.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))

def handle_db(handler, path, data, command):
    if path == '/api/db/query':
        proxy_tissdb(handler, 'POST', f'/main_db/{data.get("collection")}/_query', json.dumps(data))
    elif path == '/api/db/doc' and command == 'PUT':
        coll = data.get('collection')
        doc_id = data.get('doc_id')
        content = data.get('content')
        proxy_tissdb(handler, 'PUT', f'/main_db/{coll}/{doc_id}', json.dumps(content))
    elif path == '/api/db/databases':
        proxy_tissdb(handler, 'GET', '/_databases', '')
    elif path == '/api/db/collections':
        proxy_tissdb(handler, 'GET', f'/{data.get("db_name", "main_db")}/_collections', '')
    elif path == '/api/db/stats':
        proxy_tissdb(handler, 'GET', f'/{data.get("db_name", "main_db")}/_stats', '')
    elif path == '/api/db/collection' and command == 'POST':
        proxy_tissdb(handler, 'PUT', f'/{data.get("db_name", "main_db")}/{data.get("collection")}', '')
    elif path == '/api/db/collection' and command == 'DELETE':
        proxy_tissdb(handler, 'DELETE', f'/{data.get("db_name", "main_db")}/{data.get("collection")}', '')
    else:
        return False
    return True
