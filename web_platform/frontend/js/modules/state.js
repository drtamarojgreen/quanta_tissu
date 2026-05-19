/**
 * Centralized Application State (The Foliage Heart).
 * Resolves "State Fragmentation" by providing a single source of truth.
 */
const AppState = {
    tabs: {
        active: 'dashboard',
        history: []
    },
    tasks: {}, // task_id -> {status, progress, logs}
    db: {
        selected: 'main_db',
        collections: [],
        stats: {}
    },
    config: {
        model: {},
        training: {}
    },

    // Observers/Subscribers pattern for reactive updates
    subscribers: [],
    subscribe(callback) {
        this.subscribers.push(callback);
    },
    notify() {
        this.subscribers.forEach(cb => cb(this));
    },

    // Unified update method
    update(patch) {
        Object.assign(this, patch);
        this.notify();
    },

    // Task Management Integration
    async pollTask(taskId) {
        try {
            const task = this.tasks[taskId];
            const cursor = (task && task.logs) ? task.logs.length : 0;

            // Use unified processes API with cursor for efficiency and continuity
            const res = await fetch(`/api/processes/${taskId}/logs?cursor=${cursor}`);
            if (!res.ok) {
                // Fallback to status only if logs fail or 404
                this._pollTaskStatus(taskId);
                return;
            }
            const logData = await res.json();

            // Get current status too
            const statusRes = await fetch(`/api/processes/${taskId}`);
            const statusData = await statusRes.json();

            const existingLogs = (task && task.logs) ? task.logs : [];
            const newLogs = logData.logs || [];

            this.tasks[taskId] = {
                ...statusData,
                logs: [...existingLogs, ...newLogs]
            };

            this.notify();

            if (statusData.state === 'RUNNING') {
                setTimeout(() => this.pollTask(taskId), 2000);
            }
        } catch (e) {
            console.error(`Error polling task ${taskId}:`, e);
            // Fallback for legacy tasks
            this._pollLegacyTask(taskId);
        }
    },

    async _pollTaskStatus(taskId) {
         try {
            const res = await fetch(`/api/processes/${taskId}`);
            if (!res.ok) return;
            const data = await res.json();
            this.tasks[taskId] = { ...this.tasks[taskId], ...data };
            this.notify();
            if (data.state === 'RUNNING') setTimeout(() => this.pollTask(taskId), 2000);
        } catch(e) {}
    },

    async _pollLegacyTask(taskId) {
        try {
            const res = await fetch(`/api/tasks/${taskId}`);
            if (!res.ok) return;
            const data = await res.json();
            const existingLogs = this.tasks[taskId] ? (this.tasks[taskId].logs || []) : [];
            const incomingLogs = data.logs || [];
            const lastLine = existingLogs.length > 0 ? existingLogs[existingLogs.length - 1] : null;
            const newLines = lastLine ? incomingLogs.slice(incomingLogs.indexOf(lastLine) + 1) : incomingLogs;

            this.tasks[taskId] = { ...data, logs: [...existingLogs, ...newLines] };
            this.notify();
            if (data.status === 'running') setTimeout(() => this.pollTask(taskId), 2000);
        } catch(e) {}
    },

    // Unified rehydration for tab switches and reloads
    async rehydrate() {
        try {
            const res = await fetch('/api/processes'); // Should return list of active processes
            if (!res.ok) return;
            const tasks = await res.json();

            tasks.forEach(task => {
                if (!this.tasks[task.taskId]) {
                    this.tasks[task.taskId] = { ...task, logs: task.logs || [] };
                    if (task.state === 'RUNNING') {
                        this.pollTask(task.taskId);
                    }
                }
            });
            this.notify();
        } catch (e) {
            console.error("Rehydration failed:", e);
        }
    }
};

// Rehydrate on load and visibility change
window.addEventListener('load', () => AppState.rehydrate());
document.addEventListener('visibilitychange', () => {
    if (document.visibilityState === 'visible') AppState.rehydrate();
});

window.AppState = AppState;
