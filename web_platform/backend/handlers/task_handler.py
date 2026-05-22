import json
from urllib.parse import urlparse, parse_qs
from web_platform.backend.managers.task_manager import task_manager

def _send_json(handler, status, data):
    response = json.dumps(data).encode('utf-8')
    handler.send_response(status)
    handler.send_header('Content-Type', 'application/json')
    handler.send_header('Content-Length', len(response))
    handler.end_headers()
    handler.wfile.write(response)

def handle_tasks(handler, path, data, command):
    # Parse path and query params safely
    parsed_url = urlparse(path)
    clean_path = parsed_url.path
    query_params = parse_qs(parsed_url.query)

    # We only handle routes starting with /api/processes or /api/tasks
    # (And internal/tasks is mapped to tasks)
    if clean_path.startswith('/api/internal/tasks'):
        clean_path = clean_path.replace('/api/internal/tasks', '/api/tasks')

    if not clean_path.startswith('/api/tasks') and not clean_path.startswith('/api/processes'):
        return False

    parts = clean_path.split('/')
    # parts[0] is '', parts[1] is 'api', parts[2] is 'processes' or 'tasks'

    if parts[2] == 'processes':
        # GET /api/processes
        if len(parts) == 3:
            if command == 'GET':
                _send_json(handler, 200, task_manager.list_processes())
                return True
            elif command == 'POST':
                task_id = data.get('task_id')
                type_ = data.get('type', 'shell')
                cmd = data.get('command')
                working_dir = data.get('working_dir')
                
                if not task_id or not cmd:
                    _send_json(handler, 400, {'error': 'task_id and command are required'})
                    return True
                
                try:
                    task_dict = task_manager.start_process(task_id, type_, cmd, working_dir)
                    _send_json(handler, 200, task_dict)
                except Exception as e:
                    _send_json(handler, 500, {'error': str(e)})
                return True

        # /api/processes/{task_id} or /api/processes/{task_id}/logs or /api/processes/verify/{task_id}
        elif len(parts) >= 4:
            # Check for /api/processes/verify/{task_id}
            if parts[3] == 'verify' and len(parts) == 5:
                task_id = parts[4]
                task = task_manager.get_process(task_id)
                if not task:
                    _send_json(handler, 200, {'valid': False, 'error': 'Task not found'})
                    return True
                log_continuity = len(task['logs']) > 0
                _send_json(handler, 200, {
                    'valid': True,
                    'taskId': task_id,
                    'state': task['state'],
                    'log_count': len(task['logs']),
                    'log_continuity': log_continuity
                })
                return True

            task_id = parts[3]
            if len(parts) == 4:
                # GET /api/processes/{task_id}
                if command == 'GET':
                    task_dict = task_manager.get_process(task_id)
                    if task_dict:
                        _send_json(handler, 200, task_dict)
                    else:
                        _send_json(handler, 404, {'error': f'Process {task_id} not found'})
                    return True
                # DELETE /api/processes/{task_id}
                elif command == 'DELETE':
                    task_manager.stop_process(task_id)
                    _send_json(handler, 200, {'message': 'Process stop signal sent'})
                    return True
            
            elif len(parts) == 5 and parts[4] == 'logs':
                # GET /api/processes/{task_id}/logs
                if command == 'GET':
                    cursor_val = 0
                    if 'cursor' in query_params:
                        try:
                            cursor_val = int(query_params['cursor'][0])
                        except ValueError:
                            pass
                    
                    logs_slice, next_cursor = task_manager.get_process_logs(task_id, cursor_val)
                    _send_json(handler, 200, {
                        'logs': logs_slice,
                        'cursor': next_cursor
                    })
                    return True

    elif parts[2] == 'tasks':
        # GET /api/tasks
        if len(parts) == 3:
            if command == 'GET':
                _send_json(handler, 200, task_manager.list_tasks())
                return True

        # /api/tasks/{task_id}
        elif len(parts) == 4:
            task_id = parts[3]
            if command == 'GET':
                status = task_manager.get_task_status(task_id)
                if status.get('status') == 'not_found':
                    _send_json(handler, 404, status)
                else:
                    _send_json(handler, 200, status)
                return True
            elif command == 'DELETE':
                success, msg = task_manager.stop_task(task_id)
                status_code = 200 if success else 400
                _send_json(handler, status_code, {'message': msg})
                return True

    return False
