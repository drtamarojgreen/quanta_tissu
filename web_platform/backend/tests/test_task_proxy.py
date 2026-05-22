import unittest
import json
import io
import sys
import os

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from web_platform.backend.handlers.task_handler import handle_tasks
from web_platform.backend.managers.task_manager import task_manager

class StubHandler:
    def __init__(self):
        self.response_status = None
        self.response_headers = {}
        self.wfile = io.BytesIO()

    def send_response(self, code, message=None):
        self.response_status = code

    def send_header(self, keyword, value):
        self.response_headers[keyword] = value

    def end_headers(self):
        pass

class TestTaskHandler(unittest.TestCase):
    def setUp(self):
        # Clear task manager tasks before each test
        task_manager.tasks.clear()
        task_manager.processes.clear()

    def test_processes_route_not_matched(self):
        handler = StubHandler()
        # Unmatched routes should return False
        result = handle_tasks(handler, '/api/unmatched', {}, 'GET')
        self.assertFalse(result)

    def test_start_process_post(self):
        handler = StubHandler()
        data = {
            'task_id': 'test_post_task',
            'command': 'echo hello',
            'type': 'shell'
        }
        result = handle_tasks(handler, '/api/processes', data, 'POST')
        self.assertTrue(result)
        self.assertEqual(handler.response_status, 200)

        # Verify task is created in task_manager
        task = task_manager.get_process('test_post_task')
        self.assertIsNotNone(task)
        self.assertEqual(task['command'], 'echo hello')

    def test_get_processes_list(self):
        # Manually seed task_manager
        task_manager.start_process('task_1', 'shell', 'echo 1')
        
        handler = StubHandler()
        result = handle_tasks(handler, '/api/processes', {}, 'GET')
        self.assertTrue(result)
        self.assertEqual(handler.response_status, 200)

        response_body = json.loads(handler.wfile.getvalue().decode('utf-8'))
        self.assertIsInstance(response_body, list)
        self.assertEqual(len(response_body), 1)
        self.assertEqual(response_body[0]['taskId'], 'task_1')

    def test_delete_process(self):
        task_manager.start_process('task_to_delete', 'shell', 'sleep 10')
        
        handler = StubHandler()
        result = handle_tasks(handler, '/api/processes/task_to_delete', {}, 'DELETE')
        self.assertTrue(result)
        self.assertEqual(handler.response_status, 200)

        task = task_manager.get_process('task_to_delete')
        self.assertEqual(task['state'], 'STOPPED')

if __name__ == '__main__':
    unittest.main()
