import json
import http.client
import os
from web_platform.backend.managers.task_manager import task_manager

ORCHESTRATOR_HOST = os.environ.get('ORCHESTRATOR_HOST', 'localhost')
ORCHESTRATOR_PORT = int(os.environ.get('ORCHESTRATOR_PORT', 8080))

def proxy_to_orchestrator(handler, method, path, data=None):
    try:
        conn = http.client.HTTPConnection(ORCHESTRATOR_HOST, ORCHESTRATOR_PORT)
        headers = {'Content-Type': 'application/json'}
        body = json.dumps(data) if data else None

        conn.request(method, path, body=body, headers=headers)
        res = conn.getresponse()

        handler.send_response(res.status)
        for header, value in res.getheaders():
            handler.send_header(header, value)
        handler.end_headers()
        handler.wfile.write(res.read())
        conn.close()
        return True
    except Exception as e:
        handler.send_response(500)
        handler.send_header('Content-Type', 'application/json')
        handler.end_headers()
        handler.wfile.write(json.dumps({'error': f"Failed to proxy to orchestrator: {e}"}).encode('utf-8'))
        return True

def handle_tasks(handler, path, data, command):
    # Internal route for Java to call back to Python without recursion
    if path.startswith('/api/internal/tasks'):
        return _handle_local_tasks(handler, path.replace('/api/internal/tasks', '/api/tasks'), data, command)

    if not path.startswith('/api/tasks') and not path.startswith('/api/processes'):
        return False

    # Map /api/processes to /api/orchestrator/processes
    if path.startswith('/api/processes'):
        orchestrator_path = path.replace('/api/processes', '/api/orchestrator/processes')
        return proxy_to_orchestrator(handler, command, orchestrator_path, data)

    # Legacy /api/tasks mapping - proxy but preserve the full path (including task_id)
    if path.startswith('/api/tasks'):
        orchestrator_path = path.replace('/api/tasks', '/api/orchestrator/tasks')
        return proxy_to_orchestrator(handler, command, orchestrator_path, data)

    return False

def _handle_local_tasks(handler, path, data, command):
    parts = path.split('/')
    # /api/tasks
    if len(parts) == 3:
        if command == 'GET':
            response = json.dumps(task_manager.list_tasks()).encode('utf-8')
            handler.send_response(200)
            handler.send_header('Content-Type', 'application/json')
            handler.end_headers()
            handler.wfile.write(response)
            return True

    # /api/tasks/{task_id}
    if len(parts) == 4:
        task_id = parts[3]
        if command == 'GET':
            response = json.dumps(task_manager.get_task_status(task_id)).encode('utf-8')
            handler.send_response(200)
            handler.send_header('Content-Type', 'application/json')
            handler.end_headers()
            handler.wfile.write(response)
            return True
        elif command == 'DELETE':
            success, msg = task_manager.stop_task(task_id)
            status = 200 if success else 400
            handler.send_response(status)
            handler.send_header('Content-Type', 'application/json')
            handler.end_headers()
            handler.wfile.write(json.dumps({'message': msg}).encode('utf-8'))
            return True
    return False
