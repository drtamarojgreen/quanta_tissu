import unittest
import json
import io
import sys
import os
import time

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from web_platform.backend.managers.task_manager import task_manager, ProcessTask
from web_platform.backend.handlers.task_handler import handle_tasks

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

class TestTaskEdgeCases(unittest.TestCase):
    def setUp(self):
        self.tm = task_manager
        self.tm.tasks.clear()
        self.tm.processes.clear()

    def test_start_non_existent_command(self):
        # Starting an invalid binary should fail on Popen
        with self.assertRaises((FileNotFoundError, OSError)):
            self.tm.start_process('invalid_cmd_task', 'shell', 'nonexistentbinary12345')

    def test_start_already_running_task(self):
        # Start a long-running process
        self.tm.start_process('running_task', 'shell', 'sleep 10')
        
        # Trying to start it again should raise RuntimeError
        with self.assertRaises(RuntimeError) as ctx:
            self.tm.start_process('running_task', 'shell', 'echo hello')
        self.assertIn("Task already running", str(ctx.exception))
        
        # Clean up
        self.tm.stop_process('running_task')

    def test_concurrent_tasks(self):
        # Start multiple tasks concurrently
        self.tm.start_process('task_a', 'shell', 'sleep 0.1')
        self.tm.start_process('task_b', 'shell', 'sleep 0.2')

        # Verify both exist and are running
        task_a = self.tm.get_process('task_a')
        task_b = self.tm.get_process('task_b')
        self.assertIsNotNone(task_a)
        self.assertIsNotNone(task_b)
        self.assertEqual(task_a['state'], 'RUNNING')
        self.assertEqual(task_b['state'], 'RUNNING')

        # Wait for them to finish
        time.sleep(0.5)

        self.assertEqual(self.tm.get_process('task_a')['state'], 'COMPLETED')
        self.assertEqual(self.tm.get_process('task_b')['state'], 'COMPLETED')

    def test_log_buffer_overflow(self):
        # Create a ProcessTask with small logs limit simulation
        task = ProcessTask('overflow_task', 'shell', 'dummy')
        for i in range(2500):
            task.add_log(f"log line {i}")
        
        # Buffer should be capped at 2000 lines
        task_dict = task.to_dict()
        self.assertEqual(len(task_dict['logs']), 2000)
        self.assertEqual(task_dict['logs'][0], "log line 500")
        self.assertEqual(task_dict['logs'][-1], "log line 2499")

    def test_get_logs_invalid_cursor(self):
        self.tm.start_process('cursor_task', 'shell', 'echo hello')
        time.sleep(0.2) # wait to complete

        # Cursor beyond actual logs
        logs, next_cursor = self.tm.get_process_logs('cursor_task', 50)
        self.assertEqual(logs, [])
        self.assertEqual(next_cursor, 1) # only has 1 log line

        # Cursor negative
        logs, next_cursor = self.tm.get_process_logs('cursor_task', -5)
        # Should return full logs if cursor is invalid/negative
        self.assertEqual(len(logs), 1)

    def test_stop_non_running_or_completed_task(self):
        # Stop a non-existent task (should not crash)
        self.tm.stop_process('non_existent_stop')

        # Stop a completed task (should keep COMPLETED state)
        self.tm.start_process('completed_task', 'shell', 'echo done')
        time.sleep(0.2)
        self.assertEqual(self.tm.get_process('completed_task')['state'], 'COMPLETED')

        self.tm.stop_process('completed_task')
        self.assertEqual(self.tm.get_process('completed_task')['state'], 'COMPLETED')

    def test_handler_missing_post_parameters(self):
        # Missing task_id
        handler = StubHandler()
        res = handle_tasks(handler, '/api/processes', {'command': 'echo 1'}, 'POST')
        self.assertTrue(res)
        self.assertEqual(handler.response_status, 400)

        # Missing command
        handler = StubHandler()
        res = handle_tasks(handler, '/api/processes', {'task_id': 't'}, 'POST')
        self.assertTrue(res)
        self.assertEqual(handler.response_status, 400)

    def test_handler_get_non_existent_process(self):
        handler = StubHandler()
        res = handle_tasks(handler, '/api/processes/non_existent_id', {}, 'GET')
        self.assertTrue(res)
        self.assertEqual(handler.response_status, 404)

    def test_handler_verify_route(self):
        # Test verification for running/completed task
        self.tm.start_process('verify_task', 'shell', 'echo testing verification')
        time.sleep(0.2)

        handler = StubHandler()
        res = handle_tasks(handler, '/api/processes/verify/verify_task', {}, 'GET')
        self.assertTrue(res)
        self.assertEqual(handler.response_status, 200)

        resp = json.loads(handler.wfile.getvalue().decode('utf-8'))
        self.assertTrue(resp['valid'])
        self.assertEqual(resp['taskId'], 'verify_task')
        self.assertEqual(resp['state'], 'COMPLETED')
        self.assertTrue(resp['log_continuity'])

        # Test verification for non-existent task
        handler = StubHandler()
        res = handle_tasks(handler, '/api/processes/verify/does_not_exist', {}, 'GET')
        self.assertTrue(res)
        self.assertEqual(handler.response_status, 200)
        resp = json.loads(handler.wfile.getvalue().decode('utf-8'))
        self.assertFalse(resp['valid'])
        self.assertEqual(resp['error'], 'Task not found')

    def test_legacy_tasks_routing(self):
        self.tm.start_process('legacy_task', 'shell', 'echo legacy')
        time.sleep(0.2)

        handler = StubHandler()
        res = handle_tasks(handler, '/api/tasks/legacy_task', {}, 'GET')
        self.assertTrue(res)
        self.assertEqual(handler.response_status, 200)

        resp = json.loads(handler.wfile.getvalue().decode('utf-8'))
        self.assertEqual(resp['status'], 'completed')
        self.assertIn('legacy', resp['logs'])
        self.assertIn('runtime', resp)

if __name__ == '__main__':
    unittest.main()
