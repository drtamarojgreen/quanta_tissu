const TestModule = {
    state: {
        currentTaskId: null,
        pollingInterval: null,
        lastStatus: null
    },

    async runCategorizedTests(type) {
        window.switchTab('tests');
        const statusText = document.getElementById('test-status');
        const detailsEl = document.getElementById('test-details');
        statusText.innerText = 'Running ' + type + ' tests...';
        detailsEl.innerText = '';
        try {
            const res = await fetch('/api/tests/run', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ type })
            });
            const data = await res.json();
            statusText.innerText = `${data.summary.passed}/${data.summary.total} Passed`;
            detailsEl.innerText = data.results.map(r => `[${r.status}] ${r.name} (${r.duration})`).join('\n');
        } catch(e) {
            statusText.innerText = 'Error';
            detailsEl.innerText = e.message;
        }
    },

    async loadTestScripts() {
        try {
            const res = await fetch('/api/testing/list');
            const data = await res.json();
            const select = document.getElementById('script-select');
            if (select) {
                select.innerHTML = data.tests.map(t => `<option value="${t}">${t}</option>`).join('');
            }
        } catch (e) {}
    },

    async runScript() {
        const testName = document.getElementById('script-select').value;
        const skipCompile = document.getElementById('test-arg-skip-compile').checked;
        const overrides = {
            n_embd: document.getElementById('test-arg-n_embd').value,
            n_layer: document.getElementById('test-arg-n_layer').value,
            num_epochs: document.getElementById('test-arg-epochs').value,
            skip_compile: skipCompile ? "1" : ""
        };
        
        Object.keys(overrides).forEach(key => {
            if (overrides[key] === "") delete overrides[key];
        });

        const argSummary = Object.entries(overrides).map(([k, v]) => `${k}=${v}`).join(' ') || 'None';

        UIModule.openModal('modal-confirm-process', {
            command: `bash tests/model/program/${testName}.sh [Overrides: ${argSummary}]`,
            callback: 'TestModule.executeRunScript'
        });
    },

    async executeRunScript() {
        const testName = document.getElementById('script-select').value;
        const resultsEl = document.getElementById('script-results');
        const skipCompile = document.getElementById('test-arg-skip-compile').checked;
        
        const overrides = {
            n_embd: document.getElementById('test-arg-n_embd').value,
            n_layer: document.getElementById('test-arg-n_layer').value,
            num_epochs: document.getElementById('test-arg-epochs').value,
            skip_compile: skipCompile ? "1" : ""
        };

        resultsEl.innerText = `Starting ${testName}.sh...`;
        
        try {
            const res = await fetch('/api/testing/run', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ 
                    test_name: testName,
                    overrides: overrides
                })
            });
            const data = await res.json();
            if (data.task_id) {
                TestModule.state.currentTaskId = data.task_id;
                TestModule.refreshUI();
                TestModule.startPolling();
            } else {
                resultsEl.innerText = 'Error: No task ID received.';
            }
        } catch (e) { 
            resultsEl.innerText = 'Error: ' + e.message; 
        }
    },

    async stopScript() {
        if (!TestModule.state.currentTaskId) return;
        try {
            await fetch('/api/testing/stop', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ task_id: TestModule.state.currentTaskId })
            });
            // Task status update will be handled by next poll
        } catch (e) { console.error('Stop request failed', e); }
    },

    startPolling() {
        if (TestModule.state.pollingInterval) return;
        TestModule.state.pollingInterval = setInterval(async () => {
            await TestModule.checkTaskStatus();
        }, 1000);
    },

    async checkTaskStatus() {
        if (!TestModule.state.currentTaskId) return;

        try {
            const res = await fetch('/api/testing/status', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ task_id: TestModule.state.currentTaskId })
            });
            const data = await res.json();
            TestModule.state.lastStatus = data;
            TestModule.refreshUI();
        } catch (e) {
            console.error('Polling error:', e);
        }
    },

    refreshUI() {
        const resultsEl = document.getElementById('script-results');
        const stopBtn = document.getElementById('stop-script-btn');
        const data = TestModule.state.lastStatus;

        if (!resultsEl) return; // Not on the tests tab

        if (!data) {
            if (stopBtn) stopBtn.style.display = 'none';
            return;
        }

        // Handle Stop Button visibility
        if (stopBtn) {
            stopBtn.style.display = (data.status === 'running') ? 'inline-block' : 'none';
        }

        // Show progressive output
        let output = data.stdout || '';
        if (data.stderr) output += `\n[STDERR]\n${data.stderr}`;
        
        const isScrolling = resultsEl.scrollHeight - resultsEl.scrollTop <= resultsEl.clientHeight + 50;

        if (data.status === 'running') {
            resultsEl.innerText = output || 'Initializing output stream...';
        } else {
            const color = data.status === 'completed' ? '#10b981' : '#f43f5e';
            const prefix = `<div style="color: ${color}; font-weight: bold; margin-bottom: 0.5rem; border-bottom: 1px solid #374151; padding-bottom: 0.25rem;">[TASK ${data.status.toUpperCase()}]</div>`;
            resultsEl.innerHTML = prefix + (output || 'No output.');
            
            clearInterval(TestModule.state.pollingInterval);
            TestModule.state.pollingInterval = null;
            // Note: We keep currentTaskId for copy/reference until next run
        }

        if (isScrolling) {
            resultsEl.scrollTop = resultsEl.scrollHeight;
        }
    },

    async copyResults() {
        const resultsEl = document.getElementById('script-results');
        if (!resultsEl) return;
        
        try {
            await navigator.clipboard.writeText(resultsEl.innerText);
            const originalText = resultsEl.previousElementSibling?.innerText || 'Copy Output';
            const copyBtn = document.querySelector('button[onclick="TestModule.copyResults()"]');
            if (copyBtn) {
                copyBtn.innerText = 'Copied!';
                setTimeout(() => copyBtn.innerText = 'Copy Output', 2000);
            }
        } catch (err) {
            console.error('Failed to copy: ', err);
        }
    }
};

window.TestModule = TestModule;
window.runCategorizedTests = TestModule.runCategorizedTests.bind(TestModule);
