const TestModule = {
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
        UIModule.openModal('modal-confirm-process', {
            command: `bash tests/model/program/${testName}.sh`,
            callback: 'TestModule.executeRunScript'
        });
    },

    async executeRunScript() {
        const testName = document.getElementById('script-select').value;
        const resultsEl = document.getElementById('script-results');
        resultsEl.innerText = `Running ${testName}.sh...`;
        try {
            const res = await fetch('/api/testing/run', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ test_name: testName })
            });
            const data = await res.json();
            if (data.success) {
                resultsEl.innerHTML = `<span style="color: green">Test Successful:</span>\n${data.stdout}`;
            } else {
                resultsEl.innerHTML = `<span style="color: red">Test Failed:</span>\n${data.stderr}\n\nSTDOUT:\n${data.stdout}`;
            }
        } catch (e) { resultsEl.innerText = 'Error: ' + e.message; }
    }
};

window.TestModule = TestModule;
window.runCategorizedTests = TestModule.runCategorizedTests.bind(TestModule);
