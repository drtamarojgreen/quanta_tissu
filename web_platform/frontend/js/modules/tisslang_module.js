const TissLangModule = {
    async runTissLang() {
        const script = document.getElementById('tisslang-input').value;
        const resultsEl = document.getElementById('tisslang-results');
        resultsEl.innerText = 'Running...';
        try {
            const res = await fetch('/api/tisslang/run', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ script })
            });
            const data = await res.json();
            resultsEl.innerText = JSON.stringify(data, null, 2);
        } catch (e) { resultsEl.innerText = 'Error: ' + e.message; }
    }
};

window.TissLangModule = TissLangModule;
