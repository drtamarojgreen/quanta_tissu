import json
import os
import subprocess

_project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def handle_testing(handler, path, data, command):
    if path == '/api/testing/run':
        return handle_run_test(handler, data)
    elif path == '/api/testing/list':
        return handle_list_tests(handler)
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

    try:
        # Run the shell script
        process = subprocess.Popen(['bash', test_path], cwd=_project_root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate()
        success = process.returncode == 0
        _send_json(handler, 200, {'success': success, 'stdout': stdout, 'stderr': stderr})
    except Exception as e:
        _send_json(handler, 500, {'error': str(e)})
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
