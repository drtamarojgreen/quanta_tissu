import json
import os
from web_platform.backend.managers.task_manager import task_manager

# Constant ID for the analyzer task
ANALYZER_TASK_ID = 'analyzer'
_analyzer_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'tests', 'model', 'analyzer'))

def handle_analyzer(handler, path, data, command):
    if path == '/api/analyzer/start':
        return handle_start(handler, data)
    elif path == '/api/analyzer/stop':
        return handle_stop(handler, data)
    elif path == '/api/analyzer/status':
        return handle_status(handler, data)
    elif path == '/api/analyzer/logs':
        return handle_logs(handler, data)
    elif path == '/api/analyzer/build':
        return handle_build(handler, data)
    return False

def handle_build(handler, data):
    import subprocess
    try:
        process = subprocess.Popen(['make', 'all'], cwd=_analyzer_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate()
        success = process.returncode == 0
        _send_json(handler, 200, {'success': success, 'stdout': stdout, 'stderr': stderr})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_start(handler, data):
    session_id = data.get('session_id', 0)
    cmd = f"./analyzer -s {session_id} -o analyzer_log.txt"
    success, msg = task_manager.start_task(ANALYZER_TASK_ID, cmd, cwd=_analyzer_dir)
    status = 200 if success else 400
    _send_json(handler, status, {'success': success, 'message': msg, 'taskId': ANALYZER_TASK_ID})
    return True

def handle_stop(handler, data):
    success, msg = task_manager.stop_task(ANALYZER_TASK_ID)
    status = 200 if success else 400
    _send_json(handler, status, {'success': success, 'message': msg})
    return True

def handle_status(handler, data):
    status_info = task_manager.get_task_status(ANALYZER_TASK_ID)
    binary_exists = os.path.exists(os.path.join(_analyzer_dir, 'analyzer'))
    
    _send_json(handler, 200, {
        'running': status_info.get('status') == 'running',
        'binary_exists': binary_exists,
        'status': status_info.get('status')
    })
    return True

def handle_logs(handler, data):
    status_info = task_manager.get_task_status(ANALYZER_TASK_ID)
    _send_json(handler, 200, {'logs': status_info.get('logs', [])})
    return True

def _send_json(handler, status, data):
    response = json.dumps(data).encode('utf-8')
    handler.send_response(status)
    handler.send_header('Content-Type', 'application/json')
    handler.send_header('Content-Length', len(response))
    handler.end_headers()
    handler.wfile.write(response)
