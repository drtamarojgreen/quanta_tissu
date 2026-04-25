import json
import os
import subprocess
import signal

# Global state for the analyzer process
_analyzer_process = None
_analyzer_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'tests', 'model', 'analyzer'))
_analyzer_log_path = os.path.join(_analyzer_dir, 'analyzer_log.txt')

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
    try:
        process = subprocess.Popen(['make', 'all'], cwd=_analyzer_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate()
        success = process.returncode == 0
        _send_json(handler, 200, {'success': success, 'stdout': stdout, 'stderr': stderr})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_start(handler, data):
    global _analyzer_process
    if _analyzer_process and _analyzer_process.poll() is None:
        _send_json(handler, 400, {'error': "Analyzer is already running."})
        return True

    session_id = data.get('session_id', 0)
    try:
        # Clear old logs
        if os.path.exists(_analyzer_log_path):
            os.remove(_analyzer_log_path)
        
        cmd = ['./analyzer', '-s', str(session_id), '-o', 'analyzer_log.txt']
        _analyzer_process = subprocess.Popen(cmd, cwd=_analyzer_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        _send_json(handler, 200, {'success': True, 'pid': _analyzer_process.pid})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_stop(handler, data):
    global _analyzer_process
    if not _analyzer_process or _analyzer_process.poll() is not None:
        _send_json(handler, 400, {'error': "Analyzer is not running."})
        return True

    try:
        _analyzer_process.terminate()
        _analyzer_process.wait(timeout=5)
        _analyzer_process = None
        _send_json(handler, 200, {'success': True})
    except Exception as e:
        if _analyzer_process:
            _analyzer_process.kill()
        _analyzer_process = None
        _send_json(handler, 500, {'error': str(e)})
    return True

def handle_status(handler, data):
    is_running = _analyzer_process is not None and _analyzer_process.poll() is None
    binary_exists = os.path.exists(os.path.join(_analyzer_dir, 'analyzer'))
    
    _send_json(handler, 200, {
        'running': is_running,
        'binary_exists': binary_exists,
        'pid': _analyzer_process.pid if is_running else None
    })
    return True

def handle_logs(handler, data):
    try:
        logs = []
        if os.path.exists(_analyzer_log_path):
            with open(_analyzer_log_path, 'r') as f:
                logs = f.readlines()[-50:] # Last 50 lines
        
        _send_json(handler, 200, {'logs': [line.strip() for line in logs]})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
    return True

def _send_json(handler, status, data):
    response = json.dumps(data).encode('utf-8')
    handler.send_response(status)
    handler.send_header('Content-Type', 'application/json')
    handler.send_header('Content-Length', len(response))
    handler.end_headers()
    handler.wfile.write(response)
