const AnalyzerModule = {
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
        const session_id = parseInt(document.getElementById('analyzer-session-id').value || 0);
        UIModule.openModal('modal-confirm-process', {
            command: `./analyzer -s \${session_id} -o analyzer_log.txt`,
            callback: 'AnalyzerModule.executeStart'
        });
    },

    async executeStart() {
        const session_id = parseInt(document.getElementById('analyzer-session-id').value || 0);
        const resultsEl = document.getElementById('analyzer-results');
        try {
            const res = await fetch('/api/analyzer/start', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ session_id })
            });
            const data = await res.json();
            if (data.success) {
                resultsEl.innerText = \`Analyzer started (ID: \${data.taskId}). Waiting for errors...\`;
                AppState.pollTask(data.taskId);
            } else {
                resultsEl.innerText = 'Error: ' + data.message;
            }
        } catch (e) {
            resultsEl.innerText = 'Error: ' + e.message;
        }
    },

    async stop() {
        const resultsEl = document.getElementById('analyzer-results');
        try {
            const res = await fetch('/api/analyzer/stop', { method: 'POST' });
            const data = await res.json();
            if (data.success) {
                resultsEl.innerText = 'Analyzer stop request sent.';
            } else {
                resultsEl.innerText = 'Error: ' + data.message;
            }
        } catch (e) {
            resultsEl.innerText = 'Error: ' + e.message;
        }
    },

    async checkStatus() {
        const task = AppState.tasks['analyzer'];
        const statusEl = document.getElementById('analyzer-status-text');
        if (statusEl) {
            if (task) {
                statusEl.innerText = task.status === 'running' ? \`Running (PID: \${task.pid})\` : \`Status: \${task.status}\`;
                statusEl.style.color = task.status === 'running' ? 'green' : 'red';
            } else {
                statusEl.innerText = 'Not started';
                statusEl.style.color = 'gray';
            }
        }

        // Render logs if they exist
        if (task && task.logs) {
            const logEl = document.getElementById('analyzer-log-stream');
            if (logEl) {
                logEl.innerText = task.logs.join('\n');
                logEl.scrollTop = logEl.scrollHeight;
            }
        }
    }
};

// Auto-bind to AppState changes
AppState.subscribe(() => {
    if (AppState.tabs.active === 'analyzer') {
        AnalyzerModule.checkStatus();
    }
});

window.AnalyzerModule = AnalyzerModule;
