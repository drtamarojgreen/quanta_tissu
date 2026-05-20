import os
import subprocess
import threading
import time
import shlex
import signal
from datetime import datetime, timezone

class ProcessTask:
    def __init__(self, task_id, type_, command):
        self.task_id = task_id
        self.type = type_
        self.command = command
        self.state = "PENDING"
        self.created_at = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")
        self.started_at = None
        self.ended_at = None
        self.pid = None
        self.logs = []
        self.started_time_epoch = None
        self.lock = threading.Lock()

    def add_log(self, line):
        with self.lock:
            self.logs.append(line)
            if len(self.logs) > 2000:
                self.logs.pop(0)

    def to_dict(self):
        with self.lock:
            return {
                "taskId": self.task_id,
                "type": self.type,
                "command": self.command,
                "state": self.state,
                "createdAt": self.created_at,
                "startedAt": self.started_at,
                "endedAt": self.ended_at,
                "pid": self.pid,
                "logs": list(self.logs)
            }

class TaskManager:
    """
    Centralized manager for background processes (the Root System).
    Eliminates "Process Orphanage" by tracking all spawned tasks natively in Python.
    """
    def __init__(self):
        self.tasks = {}       # task_id -> ProcessTask
        self.processes = {}   # task_id -> subprocess.Popen
        self.state_lock = threading.Lock()

    def start_process(self, task_id, type_, command, working_dir=None):
        with self.state_lock:
            if task_id in self.tasks:
                existing_task = self.tasks[task_id]
                if existing_task.state == "RUNNING":
                    raise RuntimeError(f"Task already running: {task_id}")

            task = ProcessTask(task_id, type_, command)
            self.tasks[task_id] = task

        try:
            cmd_args = shlex.split(command)
        except Exception:
            cmd_args = command.split()

        # Start the process in a new session to allow killing its process group
        process = subprocess.Popen(
            cmd_args,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=working_dir,
            text=True,
            start_new_session=True
        )

        with self.state_lock:
            self.processes[task_id] = process
            task.state = "RUNNING"
            task.pid = process.pid
            task.started_at = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")
            task.started_time_epoch = time.time()

        def read_logs():
            try:
                for line in iter(process.stdout.readline, ''):
                    task.add_log(line.rstrip('\r\n'))
            except Exception as e:
                task.add_log(f"Error reading logs: {e}")
            finally:
                process.stdout.close()
                exit_code = process.wait()
                with task.lock:
                    # Only transition to COMPLETED/FAILED if we weren't stopped manually
                    if task.state == "RUNNING":
                        task.state = "COMPLETED" if exit_code == 0 else "FAILED"
                        task.ended_at = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")
                with self.state_lock:
                    if self.processes.get(task_id) == process:
                        self.processes.pop(task_id, None)

        thread = threading.Thread(target=read_logs, daemon=True)
        thread.start()

        return task.to_dict()

    def stop_process(self, task_id):
        with self.state_lock:
            proc = self.processes.pop(task_id, None)
            task = self.tasks.get(task_id)

        if task:
            with task.lock:
                if task.state in ("PENDING", "RUNNING"):
                    task.state = "STOPPED"
                    task.ended_at = datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

        if proc:
            try:
                # Terminate the process group (process + its children)
                pgid = os.getpgid(proc.pid)
                os.killpg(pgid, signal.SIGTERM)
                proc.wait(timeout=5)
            except ProcessLookupError:
                pass
            except subprocess.TimeoutExpired:
                try:
                    os.killpg(pgid, signal.SIGKILL)
                except Exception:
                    pass
            except Exception:
                # Fallback to direct process termination
                try:
                    proc.terminate()
                    proc.wait(timeout=2)
                except Exception:
                    try:
                        proc.kill()
                    except Exception:
                        pass

    def get_process(self, task_id):
        with self.state_lock:
            task = self.tasks.get(task_id)
        return task.to_dict() if task else None

    def get_process_logs(self, task_id, cursor=0):
        with self.state_lock:
            task = self.tasks.get(task_id)
        if not task:
            return [], 0
        
        all_logs = task.logs
        if cursor >= len(all_logs):
            return [], len(all_logs)
        
        return all_logs[cursor:], len(all_logs)

    def list_processes(self):
        with self.state_lock:
            all_tasks = list(self.tasks.values())
        return [t.to_dict() for t in all_tasks]

    # --- Legacy API Compatibility ---

    def start_task(self, task_id, command, cwd=None):
        try:
            self.start_process(task_id, 'shell', command, cwd)
            return True, "Task started"
        except Exception as e:
            return False, str(e)

    def stop_task(self, task_id):
        with self.state_lock:
            exists = task_id in self.tasks
        if not exists:
            return False, "Task not found"
        self.stop_process(task_id)
        return True, "Task stopped"

    def get_task_status(self, task_id):
        with self.state_lock:
            task = self.tasks.get(task_id)
        if not task:
            return {'status': 'not_found'}
        
        with task.lock:
            state = task.state
            logs = list(task.logs)
            start_time = task.started_time_epoch

        runtime = (time.time() - start_time) if (state == "RUNNING" and start_time) else 0
        return {
            'status': state.lower(),
            'logs': logs,
            'runtime': runtime
        }

    def list_tasks(self):
        with self.state_lock:
            keys = list(self.tasks.keys())
        return {tid: self.get_task_status(tid) for tid in keys}

# Global Instance
task_manager = TaskManager()
