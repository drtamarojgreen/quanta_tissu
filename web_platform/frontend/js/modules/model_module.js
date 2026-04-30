const ModelModule = {
    async generate(mode = 'python') {
        const prompt = document.getElementById('prompt-input').value;
        const temp = parseFloat(document.getElementById('temp-input').value);
        const len = parseInt(document.getElementById('len-input').value);
        const rag = document.getElementById('rag-checkbox').checked;
        const responseEl = document.getElementById('model-response');
        responseEl.innerText = 'Thinking...';

        const taskId = `inference_${mode}_${Date.now()}`;

        try {
            const endpoint = mode === 'cpp' ? '/api/model/cpp/generate' : '/api/model/generate';
            const res = await fetch(endpoint, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ prompt, length: len, temperature: temp, use_rag: rag, task_id: taskId })
            });
            const data = await res.json();
            responseEl.innerText = data.generated_text;

            // Register task in state if it's long running (mocking for now as existing endpoints are synchronous)
            // AppState.pollTask(taskId);

        } catch (e) { responseEl.innerText = 'Error: ' + e.message; }
    },

    async runModalInference(type) {
        const prompt = document.getElementById('modal-prompt-input').value;
        const resEl = document.getElementById('modal-response');
        resEl.style.display = 'block';
        resEl.innerText = 'Running...';

        const mode = (type === 'cpp_generator' || type === 'pipeline') ? 'cpp' : 'python';
        try {
            const endpoint = mode === 'cpp' ? '/api/model/cpp/generate' : '/api/model/generate';
            const res = await fetch(endpoint, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ prompt, length: 50, temperature: 0.8 })
            });
            const data = await res.json();
            resEl.innerText = data.generated_text || data.error;
        } catch(e) { resEl.innerText = 'Error: ' + e.message; }
    }
};

window.ModelModule = ModelModule;
