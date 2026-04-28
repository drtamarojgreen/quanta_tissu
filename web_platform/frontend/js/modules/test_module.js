const TestModule = {
    state: {
        currentTaskId: null,
        pollingInterval: null
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
        const overrides = {
            n_embd: document.getElementById('test-arg-n_embd').value,
            n_layer: document.getElementById('test-arg-n_layer').value,
            num_epochs: document.getElementById('test-arg-epochs').value,
            learning_rate: document.getElementById('test-arg-lr').value
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
        
        const overrides = {
            n_embd: document.getElementById('test-arg-n_embd').value,
            n_layer: document.getElementById('test-arg-n_layer').value,
            num_epochs: document.getElementById('test-arg-epochs').value,
            learning_rate: document.getElementById('test-arg-lr').value
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
                TestModule.startPolling();
            } else {
                resultsEl.innerText = 'Error: No task ID received.';
            }
        } catch (e) { resultsEl.innerText = 'Error: ' + e.message; }
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
            
            const resultsEl = document.getElementById('script-results');
            if (resultsEl) {
                // Show progressive output
                let output = data.stdout || '';
                if (data.stderr) output += `\n[STDERR]\n${data.stderr}`;
                
                resultsEl.innerText = output || (data.status === 'running' ? 'Initializing output stream...' : 'Waiting for logs...');
                resultsEl.scrollTop = resultsEl.scrollHeight;

                if (data.status === 'completed' || data.status === 'failed') {
                    const prefix = data.success ? '<span style="color: green">Test Successful:</span>\n' : '<span style="color: red">Test Failed/Finished:</span>\n';
                    resultsEl.innerHTML = prefix + resultsEl.innerText;
                    clearInterval(TestModule.state.pollingInterval);
                    TestModule.state.pollingInterval = null;
                }
            }
        } catch (e) {
            console.error('Polling error:', e);
        }
    }
};

window.TestModule = TestModule;
window.runCategorizedTests = TestModule.runCategorizedTests.bind(TestModule);
