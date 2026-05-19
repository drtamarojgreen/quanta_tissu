# Sorrel Checkins - Process Management Unified Orchestrator

## 2023-10-27 - Initial Baseline Capture
- **Symptom**: Analyzer module uses local `setInterval` and local state for polling logs/status. Switching tabs or reloading loses this local state, leading to "lost process" visibility even if the backend process is still running.
- **Symptom**: `task_manager.py` only returns the last 50 lines of logs, leading to gaps if polling is interrupted.
- **Symptom**: No unified `task_id` for analyzer vs generic tasks.
- **Constraint**: Java Orchestrator exists but currently only proxies back to Python for task listing.
- **Goal**: Move canonical lifecycle ownership to Java, provide cursor-based logs, and unified API.
