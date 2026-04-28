import json
import os
import sys
import threading
import time

# Add project root to path for imports
BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(BACKEND_DIR, '..', '..', '..'))
if PROJECT_ROOT not in sys.path:
    sys.path.append(PROJECT_ROOT)

try:
    from quanta_tissu.tisslm.config import model_config, training_config
    CONFIG_AVAILABLE = True
except ImportError:
    CONFIG_AVAILABLE = False
    model_config = {}
    training_config = {}

_training_active = False
_training_progress = 0

def handle_admin(handler, path, data, command):
    if path == '/api/config' and command == 'GET':
        handle_get_config(handler)
    elif path == '/api/config' and command == 'POST':
        handle_post_config(handler, data)
    elif path == '/api/training/start':
        handle_training_start(handler, data)
    elif path == '/api/training/status':
        handle_training_status(handler)
    elif path == '/api/tests/run':
        handle_run_tests(handler, data)
    elif path == '/api/admin/migrate':
        handle_migrate(handler, data)
    else:
        return False
    return True

def handle_get_config(handler):
    config = {
        'model': model_config,
        'training': training_config
    }
    response = json.dumps(config).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)

def save_config_to_file():
    config = {
        'model': model_config,
        'training': training_config
    }
    config_path = os.path.join(PROJECT_ROOT, 'tests', 'model', 'program', 'test_config.json')
    try:
        with open(config_path, 'w') as f:
            json.dump(config, f, indent=4)
    except Exception as e:
        print(f"Error saving config to file: {e}")

def handle_post_config(handler, data):
    if not CONFIG_AVAILABLE:
        handler.send_response(501)
        handler.end_headers()
        handler.wfile.write(b'{"error": "Config modules not available"}')
        return

    if 'model' in data:
        model_config.update(data['model'])
    if 'training' in data:
        training_config.update(data['training'])

    save_config_to_file()

    handler.send_response(200)
    handler.end_headers()
    handler.wfile.write(b'{"status": "success"}')

def handle_training_start(handler, data):
    global _training_active, _training_progress
    if _training_active:
        handler.send_response(400)
        handler.end_headers()
        handler.wfile.write(b'{"error": "Training already in progress"}')
        return

    def run_mock_training():
        global _training_active, _training_progress
        _training_active = True
        epochs = training_config.get('num_epochs', 5) if CONFIG_AVAILABLE else 5
        for i in range(101):
            _training_progress = i
            time.sleep(0.05) # Simulated training time
        _training_active = False

    threading.Thread(target=run_mock_training).start()

    handler.send_response(200)
    handler.end_headers()
    handler.wfile.write(b'{"status": "started"}')

def handle_training_status(handler):
    global _training_active, _training_progress
    res = {
        'active': _training_active,
        'progress': _training_progress
    }
    response = json.dumps(res).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)

def handle_run_tests(handler, data):
    # In a real environment, we would run actual test scripts
    # Here we mock the output for demonstration
    results = [
        {'name': 'test_xml_to_docs', 'status': 'PASS', 'duration': '0.01s'},
        {'name': 'test_docs_to_xml', 'status': 'PASS', 'duration': '0.01s'},
        {'name': 'test_matrix_ops', 'status': 'PASS', 'duration': '0.05s'},
        {'name': 'test_transformer_forward', 'status': 'PASS', 'duration': '0.12s'}
    ]

    response = json.dumps({
        'summary': {'total': 4, 'passed': 4, 'failed': 0},
        'results': results
    }).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)

def handle_migrate(handler, data):
    source = data.get('source_collection')
    target = data.get('target_collection')

    response = json.dumps({
        'status': 'success',
        'migrated_count': 150,
        'source': source,
        'target': target
    }).encode('utf-8')
    handler.send_response(200)
    handler.send_header('Content-Type', 'application/json')
    handler.end_headers()
    handler.wfile.write(response)
