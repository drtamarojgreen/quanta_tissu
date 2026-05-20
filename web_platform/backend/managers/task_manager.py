import os
import requests
import json
import time

class TaskManager:
    """
    Proxies all task operations to the Java Orchestrator.
    Eliminates "Process Orphanage" by delegating lifecycle to a durable orchestrator.
    """
    def __init__(self):
        self.orchestrator_url = os.environ.get('ORCHESTRATOR_URL', 'http://localhost:8080/api/orchestrator')

    def start_task(self, task_id, command, cwd=None):
        payload = {
            'taskId': task_id,
            'command': command,
            'workingDir': cwd
        }
        try:
            res = requests.post(f"{self.orchestrator_url}/processes", json=payload)
            if res.status_code == 200:
                return True, "Task started"
            else:
                return False, res.json().get('error', 'Failed to start task')
        except Exception as e:
            return False, f"Orchestrator unreachable: {e}"

    def stop_task(self, task_id):
        try:
            res = requests.delete(f"{self.orchestrator_url}/processes/{task_id}")
            if res.status_code == 200:
                return True, "Task stopped"
            else:
                return False, "Task not found or failed to stop"
        except Exception as e:
            return False, f"Orchestrator unreachable: {e}"

    def get_task_status(self, task_id):
        try:
            res = requests.get(f"{self.orchestrator_url}/processes/{task_id}")
            if res.status_code == 200:
                data = res.json()
                return {
                    'status': data['status'].lower(),
                    'logs': data['logs'],
                    'runtime': (time.time() * 1000 - data['startTime']) / 1000 if data['status'] == 'RUNNING' else 0
                }
            return {'status': 'not_found'}
        except Exception as e:
            return {'status': 'error', 'message': str(e)}

    def list_tasks(self):
        try:
            res = requests.get(f"{self.orchestrator_url}/processes")
            if res.status_code == 200:
                tasks = res.json()
                return {tid: {
                    'status': t['status'].lower(),
                    'logs': t['logs'],
                    'runtime': (time.time() * 1000 - t['startTime']) / 1000 if t['status'] == 'RUNNING' else 0
                } for tid, t in tasks.items()}
            return {}
        except Exception as e:
            return {'error': str(e)}

# Global Instance
task_manager = TaskManager()
