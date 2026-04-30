import json
from web_platform.backend.managers.task_manager import task_manager

def handle_tasks(handler, path, data, command):
    if not path.startswith('/api/tasks'):
        return False

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
