const TestModule = {
    async runCategorizedTests(type) {
        window.switchTab('tests');
        const statusText = document.getElementById('test-status');
        const detailsEl = document.getElementById('test-details');
        statusText.innerText = 'Initializing ' + type + ' test suite...';
        detailsEl.innerText = '';

        const taskId = `test_run_${type}_${Date.now()}`;

        try {
            const res = await fetch('/api/tests/run', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ type, task_id: taskId })
            });
            const data = await res.json();

            // For now, legacy sync response
            if (data.summary) {
                statusText.innerText = `${data.summary.passed}/${data.summary.total} Passed`;
                detailsEl.innerText = data.results.map(r => `[${r.status}] ${r.name} (${r.duration})`).join('\n');
            } else {
                statusText.innerText = 'Test job queued: ' + taskId;
                AppState.pollTask(taskId);
            }
        } catch(e) {
            statusText.innerText = 'Error';
            detailsEl.innerText = e.message;
        }
    }
};

window.TestModule = TestModule;
window.runCategorizedTests = TestModule.runCategorizedTests.bind(TestModule);
