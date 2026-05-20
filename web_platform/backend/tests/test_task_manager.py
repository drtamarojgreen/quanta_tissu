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

    def test_start_and_complete_task(self):
        # Start a quick echo task
        success, msg = self.tm.start_task('echo_task', 'echo hello world')
        self.assertTrue(success)
        self.assertEqual(msg, "Task started")

        # Wait for execution and thread log reader to finish
        retries = 10
        status = None
        while retries > 0:
            status = self.tm.get_task_status('echo_task')
            if status['status'] == 'completed':
                break
            time.sleep(0.2)
            retries -= 1

        self.assertEqual(status['status'], 'completed')
        self.assertIn('hello world', status['logs'])

    def test_stop_task(self):
        # Start a longer sleep task
        success, msg = self.tm.start_task('sleep_task', 'sleep 10')
        self.assertTrue(success)

        # Ensure it's running
        status = self.tm.get_task_status('sleep_task')
        self.assertEqual(status['status'], 'running')

        # Stop the task
        success_stop, msg_stop = self.tm.stop_task('sleep_task')
        self.assertTrue(success_stop)

        # Verify status is stopped
        status = self.tm.get_task_status('sleep_task')
        self.assertEqual(status['status'], 'stopped')

    def test_get_process_logs_cursor(self):
        # Start a process that outputs multiple lines
        # Using bash to output multiple lines separated by sleep to ensure they are read
        cmd = 'bash -c "echo line1 && sleep 0.1 && echo line2 && sleep 0.1 && echo line3"'
        self.tm.start_process('multi_line', 'shell', cmd)

        # Wait a bit for it to run and output
        time.sleep(0.5)

        # Fetch first set of logs
        logs1, cursor1 = self.tm.get_process_logs('multi_line', 0)
        self.assertGreater(len(logs1), 0)
        self.assertEqual(cursor1, len(logs1))

        # Fetch next set of logs with cursor
        logs2, cursor2 = self.tm.get_process_logs('multi_line', cursor1)
        self.assertEqual(len(logs2), 0)
        self.assertEqual(cursor2, cursor1)

if __name__ == '__main__':
    unittest.main()
