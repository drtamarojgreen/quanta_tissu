import unittest
import unittest.mock as mock
import os
import sys
import time

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from web_platform.backend.managers.task_manager import TaskManager

class TestTaskManager(unittest.TestCase):
    def setUp(self):
        self.tm = TaskManager()

    @mock.patch('requests.post')
    @mock.patch('requests.get')
    @mock.patch('requests.delete')
    def test_start_and_stop_task(self, mock_delete, mock_get, mock_post):
        # Mock start
        mock_post.return_value.status_code = 200
        mock_post.return_value.json.return_value = {'success': True, 'taskId': 'sleep_task'}

        success, msg = self.tm.start_task('sleep_task', 'sleep 10')
        self.assertTrue(success)
        self.assertEqual(msg, "Task started")

        # Mock status
        mock_get.return_value.status_code = 200
        mock_get.return_value.json.return_value = {
            'status': 'RUNNING',
            'logs': [],
            'startTime': time.time() * 1000
        }
        status = self.tm.get_task_status('sleep_task')
        self.assertEqual(status['status'], 'running')

        # Mock stop
        mock_delete.return_value.status_code = 200
        success, msg = self.tm.stop_task('sleep_task')
        self.assertTrue(success)
        self.assertEqual(msg, "Task stopped")

    @mock.patch('requests.post')
    @mock.patch('requests.get')
    def test_task_logs(self, mock_get, mock_post):
        # Mock start
        mock_post.return_value.status_code = 200
        success, msg = self.tm.start_task('log_task', 'echo hello world')
        self.assertTrue(success)

        # Mock completed status with logs
        mock_get.return_value.status_code = 200
        mock_get.return_value.json.return_value = {
            'status': 'COMPLETED',
            'logs': ['hello world'],
            'startTime': time.time() * 1000
        }

        status = self.tm.get_task_status('log_task')
        self.assertEqual(status['status'], 'completed')
        self.assertTrue(any('hello world' in log for log in status['logs']))

if __name__ == '__main__':
    unittest.main()
