const ModelModule = {
    state: {
        lastResponse: '',
        lastPrompt: '',
        currentTaskId: null
    },

    async generate(mode = 'python') {
        const promptInput = document.getElementById('prompt-input');
        const prompt = promptInput ? promptInput.value : '';
        const temp = parseFloat(document.getElementById('temp-input').value);
        const len = parseInt(document.getElementById('len-input').value);
        const rag = document.getElementById('rag-checkbox').checked;
        const responseEl = document.getElementById('model-response');
        responseEl.innerText = 'Thinking...';

        const taskId = `inference_${mode}_${Date.now()}`;
        this.state.currentTaskId = taskId;

        try {
            // For inference, we use the specific model endpoints
            const endpoint = mode === 'cpp' ? '/api/model/cpp/generate' : '/api/model/generate';
            const res = await fetch(endpoint, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ prompt, length: len, temperature: temp, use_rag: rag, task_id: taskId })
            });
            const data = await res.json();
            this.state.lastResponse = data.generated_text || data.error || 'No response';
            this.state.lastPrompt = prompt;
            responseEl.innerText = this.state.lastResponse;

            // If it were a long-running training task, we'd use /api/processes
        } catch (e) { responseEl.innerText = 'Error: ' + e.message; }
    },

    async startTraining() {
        const taskId = 'training_singleton';
        this.state.currentTaskId = taskId;
        const responseEl = document.getElementById('model-response');
        responseEl.innerText = 'Starting training...';

        try {
            const res = await fetch('/api/processes', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    task_id: taskId,
                    type: 'training',
                    command: 'python3 -m tisslm.train --config config.json',
                    working_dir: '.'
                })
            });
            if (res.ok) {
                responseEl.innerText = 'Training started.';
                AppState.pollTask(taskId);
            } else {
                const data = await res.json();
                responseEl.innerText = 'Error: ' + (data.error || 'Failed to start training');
            }
        } catch (e) {
            responseEl.innerText = 'Error: ' + e.message;
        }
    },

    refreshUI() {
        const responseEl = document.getElementById('model-response');
        const promptEl = document.getElementById('prompt-input');
        if (responseEl) {
            const taskId = this.state.currentTaskId;
            const task = AppState.tasks[taskId];
            if (task && task.type === 'training') {
                responseEl.innerText = `Training ${task.state}\n` + (task.logs ? task.logs.slice(-5).join('\n') : '');
            } else {
                responseEl.innerText = this.state.lastResponse || 'Response will appear here...';
            }
        }
        if (promptEl) promptEl.value = this.state.lastPrompt || '';
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

AppState.subscribe(() => ModelModule.refreshUI());
window.ModelModule = ModelModule;
