const AnalyzerModule = {
    state: {
        pollingInterval: null,
        isRunning: false
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
            command: `./analyzer -s ${session_id} -o analyzer_log.txt`,
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
                resultsEl.innerText = `Analyzer started (PID: ${data.pid}). Waiting for errors...`;
                this.startPolling();
            } else {
                resultsEl.innerText = 'Error: ' + data.error;
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
                resultsEl.innerText = 'Analyzer stopped.';
                this.stopPolling();
            } else {
                resultsEl.innerText = 'Error: ' + data.error;
            }
        } catch (e) {
            resultsEl.innerText = 'Error: ' + e.message;
        }
    },

    async checkStatus() {
        try {
            const res = await fetch('/api/analyzer/status');
            const data = await res.json();
            const statusEl = document.getElementById('analyzer-status-text');
            if (statusEl) {
                statusEl.innerText = data.running ? `Running (PID: ${data.pid})` : 'Stopped';
                statusEl.style.color = data.running ? 'green' : 'red';
            }
            this.state.isRunning = data.running;
            if (data.running && !this.state.pollingInterval) {
                this.startPolling();
            }
        } catch (e) { console.error('Status check failed', e); }
    },

    async fetchLogs() {
        try {
            const res = await fetch('/api/analyzer/logs');
            const data = await res.json();
            const logEl = document.getElementById('analyzer-log-stream');
            if (logEl && data.logs) {
                if (data.logs.length === 0) {
                    logEl.innerText = 'No logs yet...';
                } else {
                    logEl.innerText = data.logs.join('\n');
                    logEl.scrollTop = logEl.scrollHeight;
                }
            }
        } catch (e) { console.error('Log fetch failed', e); }
    },

    startPolling() {
        if (this.state.pollingInterval) return;
        this.fetchLogs();
        this.state.pollingInterval = setInterval(() => {
            this.fetchLogs();
            this.checkStatus();
        }, 2000);
    },

    stopPolling() {
        if (this.state.pollingInterval) {
            clearInterval(this.state.pollingInterval);
            this.state.pollingInterval = null;
        }
    }
};

window.AnalyzerModule = AnalyzerModule;
