const AnalyzerModule = {
    state: {
        currentTaskId: 'analyzer_singleton'
    },

    async build() {
        UIModule.openModal('modal-confirm-process', {
            command: 'make all (in tests/model/analyzer)',
            callback: 'AnalyzerModule.executeBuild'
        });
    },

    async executeBuild() {
        const resultsEl = document.getElementById('analyzer-results');
        resultsEl.innerText = 'Building analyzer...';
        try {
            // Build is still synchronous for now, but we'll use the orchestrator for it soon
            const res = await fetch('/api/analyzer/build', { method: 'POST' });
            const data = await res.json();
            if (data.success) {
                resultsEl.innerHTML = '<span style="color: green">Build successful!</span>\n' + data.stdout;
            } else {
                resultsEl.innerHTML = '<span style="color: red">Build failed:</span>\n' + data.stderr;
            }
        } catch (e) {
            resultsEl.innerText = 'Error: ' + e.message;
        }
    },

    async start() {
        const session_id = document.getElementById('analyzer-session-id').value || 0;
        UIModule.openModal('modal-confirm-process', {
            command: `./analyzer -s \${session_id} -o analyzer_log.txt`,
            callback: 'AnalyzerModule.executeStart'
        });
    },

    async executeStart() {
        const session_id = document.getElementById('analyzer-session-id').value || 0;
        const resultsEl = document.getElementById('analyzer-results');
        const taskId = this.state.currentTaskId;

        try {
            const res = await fetch('/api/processes', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    task_id: taskId,
                    type: 'analyzer',
                    command: `./analyzer -s ${session_id} -o analyzer_log.txt`,
                    working_dir: 'tests/model/analyzer'
                })
            });
            const data = await res.json();
            if (res.ok) {
                resultsEl.innerText = `Analyzer started (PID: ${data.pid}).`;
                AppState.pollTask(taskId);
            } else {
                resultsEl.innerText = 'Error: ' + (data.error || 'Failed to start');
            }
        } catch (e) {
            resultsEl.innerText = 'Error: ' + e.message;
        }
    },

    async stop() {
        const resultsEl = document.getElementById('analyzer-results');
        const taskId = this.state.currentTaskId;
        try {
            const res = await fetch(`/api/processes/${taskId}`, { method: 'DELETE' });
            if (res.ok) {
                resultsEl.innerText = 'Analyzer stop signal sent.';
            } else {
                resultsEl.innerText = 'Error: Failed to stop';
            }
        } catch (e) {
            resultsEl.innerText = 'Error: ' + e.message;
        }
    },

    async checkStatus() {
        const taskId = this.state.currentTaskId;
        const task = AppState.tasks[taskId];
        const statusEl = document.getElementById('analyzer-status-text');
        if (statusEl) {
            if (task) {
                statusEl.innerText = `${task.state} (PID: ${task.pid || 'N/A'})`;
                statusEl.style.color = task.state === 'RUNNING' ? 'green' : 'gray';
            } else {
                statusEl.innerText = 'Stopped';
                statusEl.style.color = 'red';
            }
        }
    },

    async fetchLogs() {
        const taskId = this.state.currentTaskId;
        const task = AppState.tasks[taskId];
        const logEl = document.getElementById('analyzer-log-stream');
        if (logEl && task && task.logs) {
            if (task.logs.length === 0) {
                logEl.innerText = 'No logs yet...';
            } else {
                logEl.innerText = task.logs.join('\n');
                logEl.scrollTop = logEl.scrollHeight;
            }
        }
    },

    // UI synchronization called by AppState notify
    refreshUI() {
        this.checkStatus();
        this.fetchLogs();
    }
};

// Hook into AppState
AppState.subscribe(() => AnalyzerModule.refreshUI());

window.AnalyzerModule = AnalyzerModule;
