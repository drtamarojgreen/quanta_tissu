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
    }
};

window.TestModule = TestModule;
window.runCategorizedTests = TestModule.runCategorizedTests.bind(TestModule);
