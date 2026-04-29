import subprocess
import threading
import os
import signal
import time
import json

class TaskManager:
    """
    Centralized manager for background processes (the Root System).
    Eliminates "Process Orphanage" by tracking all spawned tasks.
    """
    def __init__(self):
        self.tasks = {} # task_id -> {process, thread, status, logs}

    def start_task(self, task_id, command, cwd=None):
        if task_id in self.tasks and self.tasks[task_id]['status'] == 'running':
            return False, "Task already running"

        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=cwd,
            text=True,
            start_new_session=True # Ensure child processes can be killed as a group
        )

        self.tasks[task_id] = {
            'process': process,
            'status': 'running',
            'logs': [],
            'start_time': time.time()
        }

        # Thread to read logs without blocking
        def read_logs():
            for line in iter(process.stdout.readline, ''):
                self.tasks[task_id]['logs'].append(line)
                if len(self.tasks[task_id]['logs']) > 1000:
                    self.tasks[task_id]['logs'].pop(0)
            process.stdout.close()
            process.wait()
            self.tasks[task_id]['status'] = 'completed' if process.returncode == 0 else 'failed'

        thread = threading.Thread(target=read_logs, daemon=True)
        thread.start()
        self.tasks[task_id]['thread'] = thread

        return True, "Task started"

    def stop_task(self, task_id):
        if task_id not in self.tasks or self.tasks[task_id]['status'] != 'running':
            return False, "Task not running"

        process = self.tasks[task_id]['process']
        try:
            pgid = os.getpgid(process.pid)
            os.killpg(pgid, signal.SIGTERM)
            process.wait(timeout=5)
        except ProcessLookupError:
            pass # Process already dead
        except subprocess.TimeoutExpired:
            try:
                os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            except ProcessLookupError:
                pass
        except Exception as e:
            return False, f"Error stopping task: {e}"

        self.tasks[task_id]['status'] = 'stopped'
        return True, "Task stopped"

    def get_task_status(self, task_id):
        if task_id not in self.tasks:
            return {'status': 'not_found'}
        task = self.tasks[task_id]
        return {
            'status': task['status'],
            'logs': task['logs'][-50:], # Return last 50 lines
            'runtime': time.time() - task['start_time'] if task['status'] == 'running' else 0
        }

    def list_tasks(self):
        return {tid: self.get_task_status(tid) for tid in self.tasks}

# Global Instance
task_manager = TaskManager()
