const TissLangModule = {
    async runTissLang() {
        const script = document.getElementById('tisslang-input').value;
        const resultsEl = document.getElementById('tisslang-results');
        resultsEl.innerText = 'Initializing TissLang engine...';

        const taskId = `tisslang_${Date.now()}`;

        try {
            const res = await fetch('/api/tisslang/run', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ script, task_id: taskId })
            });
            const data = await res.json();

            // If the response is synchronous (legacy)
            if (data.status && data.steps) {
                resultsEl.innerHTML = `
                    <p style="margin-bottom: 1rem; color: var(--accent-color); font-weight: bold;">Execution ${data.status.toUpperCase()}</p>
                    ${data.steps.map(s => `<div>[${s.name}] - ${s.result}</div>`).join('')}
                `;
            } else {
                resultsEl.innerText = JSON.stringify(data, null, 2);
            }

            // In the future, this would be a background task:
            // AppState.pollTask(taskId);

        } catch (e) { resultsEl.innerText = 'Error: ' + e.message; }
    }
};

window.TissLangModule = TissLangModule;
