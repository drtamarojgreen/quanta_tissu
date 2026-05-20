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

    async rehydrateTasks() {
        try {
            const res = await fetch('/api/tasks');
            if (!res.ok) return;
            const tasks = await res.json();
            for (const taskId in tasks) {
                this.tasks[taskId] = tasks[taskId];
                if (tasks[taskId].status === 'running') {
                    this.pollTask(taskId);
                }
            }
            this.notify();
        } catch (e) {
            console.error('Failed to rehydrate tasks:', e);
        }
    },

    // Task Management Integration
    async pollTask(taskId) {
        try {
            const res = await fetch(`/api/tasks/${taskId}`);
            if (!res.ok) return;
            const data = await res.json();

            // Persist and accumulate logs to prevent loss during navigation
            if (!this.tasks[taskId]) {
                this.tasks[taskId] = { ...data, logs: data.logs || [] };
            } else {
                const existingLogs = this.tasks[taskId].logs || [];
                const incomingLogs = data.logs || [];

                // Simple deduplication based on exact string match for the end of the log
                const lastLine = existingLogs.length > 0 ? existingLogs[existingLogs.length - 1] : null;
                const newLines = lastLine ? incomingLogs.slice(incomingLogs.indexOf(lastLine) + 1) : incomingLogs;

                this.tasks[taskId] = {
                    ...data,
                    logs: [...existingLogs, ...newLines]
                };
            }

            this.notify();

            if (data.status === 'running') {
                setTimeout(() => this.pollTask(taskId), 2000);
            }
        } catch (e) {
            console.error(`Error polling task ${taskId}:`, e);
        }
    }
};

window.AppState = AppState;
