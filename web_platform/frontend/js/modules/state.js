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
            const res = await fetch(`/api/tasks/${taskId}`);
            const data = await res.json();
            this.tasks[taskId] = data;
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
