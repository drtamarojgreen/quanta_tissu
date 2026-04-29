import unittest
import os
import sys
import time

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from web_platform.backend.managers.task_manager import TaskManager

class TestTaskManager(unittest.TestCase):
    def setUp(self):
        self.tm = TaskManager()

    def test_start_and_stop_task(self):
        # Start a simple sleep task
        success, msg = self.tm.start_task('sleep_task', ['sleep', '10'])
        self.assertTrue(success)
        self.assertEqual(msg, "Task started")

        status = self.tm.get_task_status('sleep_task')
        self.assertEqual(status['status'], 'running')

        # Stop it
        success, msg = self.tm.stop_task('sleep_task')
        self.assertTrue(success)
        self.assertEqual(msg, "Task stopped")

        status = self.tm.get_task_status('sleep_task')
        self.assertEqual(status['status'], 'stopped')

    def test_task_logs(self):
        # Start a task that prints
        success, msg = self.tm.start_task('log_task', ['echo', 'hello world'])
        self.assertTrue(success)

        # Wait for completion
        time.sleep(1)

        status = self.tm.get_task_status('log_task')
        self.assertIn(status['status'], ['completed', 'failed'])
        self.assertTrue(any('hello world' in log for log in status['logs']))

if __name__ == '__main__':
    unittest.main()
