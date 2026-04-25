import json
import os
import subprocess
import signal

_db_process = None
_project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
_tissdb_dir = os.path.join(_project_root, 'tissdb')

def handle_db_lifecycle(handler, path, data, command):
    if path == '/api/db/build':
        return handle_build(handler)
    elif path == '/api/db/start':
        return handle_start(handler)
    elif path == '/api/db/stop':
        return handle_stop(handler)
    elif path == '/api/db/lifecycle_status':
        return handle_status(handler)
    return False

def handle_build(handler):
    try:
        # Use the Makefile in tissdb directory
        process = subprocess.Popen(['make', 'all'], cwd=_tissdb_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate()
        success = process.returncode == 0
        _send_json(handler, 200, {'success': success, 'stdout': stdout, 'stderr': stderr})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_start(handler):
    global _db_process
    if _db_process and _db_process.poll() is None:
        _send_json(handler, 400, {'error': "TissDB is already running."})
        return True

    try:
        # Start the compiled binary
        binary_path = os.path.join(_tissdb_dir, 'tissdb')
        if not os.path.exists(binary_path):
             _send_json(handler, 400, {'error': "TissDB binary not found. Please build it first."})
             return True

        _db_process = subprocess.Popen(['./tissdb'], cwd=_tissdb_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        _send_json(handler, 200, {'success': True, 'pid': _db_process.pid})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_stop(handler):
    global _db_process
    if not _db_process or _db_process.poll() is not None:
        _send_json(handler, 400, {'error': "TissDB is not running."})
        return True

    try:
        _db_process.terminate()
        _db_process.wait(timeout=5)
        _db_process = None
        _send_json(handler, 200, {'success': True})
    except Exception as e:
        if _db_process:
            _db_process.kill()
        _db_process = None
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_status(handler):
    is_running = _db_process is not None and _db_process.poll() is None
    binary_exists = os.path.exists(os.path.join(_tissdb_dir, 'tissdb'))
    _send_json(handler, 200, {
        'running': is_running,
        'binary_exists': binary_exists,
        'pid': _db_process.pid if is_running else None
    })
    return True

def _send_json(handler, status, data):
    response = json.dumps(data).encode('utf-8')
    handler.send_response(status)
    handler.send_header('Content-Type', 'application/json')
    handler.send_header('Content-Length', len(response))
    handler.end_headers()
    handler.wfile.write(response)
