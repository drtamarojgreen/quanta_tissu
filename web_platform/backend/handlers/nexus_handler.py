import json

def handle_nexus(handler, path, data, command):
    if path == '/api/nexus/graph':
        get_graph(handler, data)
    else:
        return False
    return True

def get_graph(handler, data):
    nodes = [
        {'id': 'TissDB', 'x': 400, 'y': 250, 'label': 'TissDB'},
        {'id': 'QuantaTissu', 'x': 200, 'y': 100, 'label': 'QuantaTissu'},
        {'id': 'Analytics', 'x': 600, 'y': 100, 'label': 'Analytics'},
        {'id': 'NexusFlow', 'x': 400, 'y': 450, 'label': 'NexusFlow'}
    ]
    links = [
        {'source': 'QuantaTissu', 'target': 'TissDB'},
        {'source': 'Analytics', 'target': 'TissDB'},
        {'source': 'NexusFlow', 'target': 'TissDB'}
    ]
    response = json.dumps({'nodes': nodes, 'links': links}).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)
