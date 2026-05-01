import json
import os
import subprocess
import threading
import uuid
import sys
import signal

_project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

# Global state for background tasks
_tasks = {}

def handle_testing(handler, path, data, command):
    if path == '/api/testing/run':
        return handle_run_test(handler, data)
    elif path == '/api/testing/list':
        return handle_list_tests(handler)
    elif path == '/api/testing/status':
        return handle_get_status(handler, data)
    elif path == '/api/testing/stop':
        return handle_stop_test(handler, data)
    return False

def handle_run_test(handler, data):
    test_name = data.get('test_name')
    if not test_name:
        _send_json(handler, 400, {'error': "test_name is required"})
        return True

    test_path = os.path.join(_project_root, 'tests', 'model', 'program', f"{test_name}.sh")
    if not os.path.exists(test_path):
        _send_json(handler, 404, {'error': f"Test script {test_name}.sh not found"})
        return True

    task_id = str(uuid.uuid4())
    _tasks[task_id] = {
        'status': 'running',
        'stdout': '',
        'stderr': '',
        'test_name': test_name,
        'success': None,
        'process': None
    }

    def run_task(tid, tpath, overrides):
        try:
            env = os.environ.copy()
            env["PYTHONUNBUFFERED"] = "1"
            for key, value in overrides.items():
                if value:
                    env[f"TISSLM_{key.upper()}"] = str(value)

            cmd = ['bash', tpath]
            process = subprocess.Popen(
                cmd, 
                cwd=_project_root, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE, 
                text=True, 
                env=env,
                bufsize=1,
                preexec_fn=os.setsid # Allow killing the entire process group
            )
            _tasks[tid]['process'] = process
            
            def read_stream(stream, key):
                # Using read(1) to avoid newline buffering issues
                while True:
                    char = stream.read(1)
                    if not char:
                        break
                    _tasks[tid][key] += char
                stream.close()

            t1 = threading.Thread(target=read_stream, args=(process.stdout, 'stdout'))
            t2 = threading.Thread(target=read_stream, args=(process.stderr, 'stderr'))
            t1.start()
            t2.start()

            process.wait()
            t1.join()
            t2.join()
            
            # Update status only if it wasn't stopped manually
            if _tasks[tid]['status'] == 'running':
                _tasks[tid]['success'] = (process.returncode == 0)
                _tasks[tid]['status'] = 'completed'
        except Exception as e:
            _tasks[tid]['status'] = 'failed'
            _tasks[tid]['stderr'] += f"\n[Internal Error] {str(e)}"

    overrides = data.get('overrides', {})
    threading.Thread(target=run_task, args=(task_id, test_path, overrides)).start()

    _send_json(handler, 200, {'task_id': task_id})
    return True

def handle_stop_test(handler, data):
    task_id = data.get('task_id')
    if not task_id or task_id not in _tasks:
        _send_json(handler, 404, {'error': "Task not found"})
        return True
    
    task = _tasks[task_id]
    if task['status'] == 'running' and task['process']:
        try:
            # Kill the process group (bash + its children like tissdb_exe)
            os.killpg(os.getpgid(task['process'].pid), signal.SIGTERM)
            task['status'] = 'stopped'
            task['stdout'] += "\n--- PROCESS TERMINATED BY USER ---"
            _send_json(handler, 200, {'success': True})
        except Exception as e:
            _send_json(handler, 500, {'error': str(e)})
    else:
        _send_json(handler, 400, {'error': "Task is not running"})
    return True

def handle_get_status(handler, data):
    task_id = data.get('task_id')
    if not task_id or task_id not in _tasks:
        _send_json(handler, 404, {'error': "Task not found"})
        return True
    
    # Return a copy without the process object (not serializable)
    status_data = {k: v for k, v in _tasks[task_id].items() if k != 'process'}
    _send_json(handler, 200, status_data)
    return True

def handle_list_tests(handler):
    tests_dir = os.path.join(_project_root, 'tests', 'model', 'program')
    try:
        scripts = [f[:-3] for f in os.listdir(tests_dir) if f.endswith('.sh')]
        _send_json(handler, 200, {'tests': scripts})
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
