const state = {
    activeTab: 'dashboard',
    dbName: 'main_db'
};

const views = {
    dashboard: `
        <div class="grid">
            <div class="card"><h3>System Status</h3><p id="status-val" style="font-size: 2rem; color: green">Online</p></div>
            <div class="card"><h3>DB Stats</h3><div id="db-stats-val" style="font-size: 1.2rem;">Loading...</div></div>
            <div class="card"><h3>Model Status</h3><p style="font-size: 2rem; color: purple">Ready</p></div>
        </div>
    `,
    explorer: `
        <div class="card">
            <h3>Database Browser</h3>
            <div style="margin-bottom: 1rem">
                <select id="db-select" onchange="loadCollections()"></select>
                <select id="coll-select"></select>
            </div>
            <h3>TissQL Query</h3>
            <textarea id="query-input" rows="4">SELECT * FROM knowledge</textarea>
            <button class="btn btn-primary" onclick="runQuery()">Execute Query</button>
            <div id="query-results" class="results">Results will appear here...</div>
        </div>
    `,
    playground: `
        <div class="card">
            <h3>Chat with QuantaTissu</h3>
            <div style="display: flex; gap: 1rem; margin-bottom: 1rem; align-items: center;">
                <div style="flex: 1">
                    <label><input type="checkbox" id="rag-checkbox"> Use RAG</label>
                </div>
                <div style="flex: 1">
                    <label>Temp</label>
                    <input type="range" id="temp-input" min="0.1" max="2.0" step="0.1" value="0.8">
                </div>
                <div style="flex: 1">
                    <label>Length</label>
                    <input type="number" id="len-input" value="50" style="width: 60px; margin-bottom: 0">
                </div>
            </div>
            <div style="display: flex; gap: 1rem; margin-bottom: 1rem;">
                <button class="btn btn-primary" onclick="generate('python')" data-tooltip="Run inference using the Python-based QuantaTissu model.">Send (Python)</button>
                <button class="btn btn-primary" onclick="generate('cpp')" style="background: #059669" data-tooltip="Run inference using the high-performance indigenous C++ Transformer stack.">Send (C++)</button>
            </div>
            <input type="text" id="prompt-input" placeholder="Ask me anything...">
            <div id="model-response" class="results">Response will appear here...</div>
        </div>
    `,
    analytics: `
        <div class="card">
            <h3>Performance Analytics</h3>
            <button class="btn btn-primary" onclick="loadAnalyticsFromServer()">Fetch Chart Data</button>
            <div id="chart-container" style="margin-top: 1rem"></div>
            <hr style="margin: 1rem 0">
            <h3>Trading Signal Engine</h3>
            <input type="text" id="symbol-input" value="AAPL" style="width: 100px; margin-right: 1rem">
            <button class="btn btn-primary" onclick="runTradingEngine()">Generate Signal</button>
            <div id="trading-results" style="margin-top: 1rem"></div>
        </div>
    `,
    tisslang: `
        <div class="card">
            <h3>TissLang Orchestration</h3>
            <p style="font-size: 0.8rem; color: #666; margin-bottom: 0.5rem">Define tasks and steps in TissLang syntax.</p>
            <textarea id="tisslang-input" rows="10" style="font-family: monospace">TASK "Example Workflow"

STEP "Analyze" {
    ASSERT 1 == 1
}</textarea>
            <button class="btn btn-primary" onclick="runTissLang()">Run Workflow</button>
            <div id="tisslang-results" class="results">Execution log will appear here...</div>
        </div>
    `,
    nexus: `
        <div class="card">
            <h3>Nexus Flow Canvas</h3>
            <canvas id="nexus-canvas" width="800" height="500" style="border: 1px solid #ddd; background: #000"></canvas>
        </div>
    `,
    config: `
        <div class="grid">
            <div class="card">
                <h3>Model Architecture</h3>
                <div class="form-group">
                    <label>Embed Dim (n_embd)</label>
                    <input type="number" id="conf-n_embd" value="128">
                </div>
                <div class="form-group">
                    <label>Layers (n_layer)</label>
                    <input type="number" id="conf-n_layer" value="4">
                </div>
                <div class="form-group">
                    <label>Heads (n_head)</label>
                    <input type="number" id="conf-n_head" value="8">
                </div>
                <div class="form-group">
                    <label>FFN Dim (d_ff)</label>
                    <input type="number" id="conf-d_ff" value="512">
                </div>
                <div class="form-group">
                    <label>Dropout Rate</label>
                    <input type="number" step="0.01" id="conf-dropout" value="0.1">
                </div>
                <div class="form-group">
                    <label>LayerNorm Eps</label>
                    <input type="number" step="1e-7" id="conf-eps" value="1e-6">
                </div>
                <button class="btn btn-primary" onclick="saveConfig()">Save Architecture</button>
            </div>
            <div class="card">
                <h3>Training Parameters</h3>
                <div class="form-group">
                    <label>Learning Rate</label>
                    <input type="number" step="0.0001" id="conf-lr" value="0.0001">
                </div>
                <div class="form-group">
                    <label>Batch Size</label>
                    <input type="number" id="conf-batch" value="1">
                </div>
                <div class="form-group">
                    <label>Epochs</label>
                    <input type="number" id="conf-epochs" value="5">
                </div>
                <div class="form-group">
                    <label>Weight Decay</label>
                    <input type="number" step="0.01" id="conf-wd" value="0.01">
                </div>
                <button class="btn btn-primary" onclick="saveConfig()">Save Hyperparams</button>
            </div>
        </div>
        <div class="card" style="margin-top: 1rem">
            <h3>Training Control</h3>
            <div id="training-status-area">
                <p>Status: <span id="train-status-text">Idle</span></p>
                <div id="progress-bar-container" style="width: 100%; height: 20px; background: #eee; border-radius: 10px; overflow: hidden; display: none;">
                    <div id="progress-bar-fill" style="width: 0%; height: 100%; background: #4a90e2; transition: width 0.3s"></div>
                </div>
                <button id="start-train-btn" class="btn btn-primary" style="margin-top: 1rem" onclick="startTraining()">Start Training Job</button>
            </div>
        </div>
    `,
    help: `
        <div class="card">
            <h3>Platform Documentation</h3>
            <div class="help-section">
                <h4>Data Explorer</h4>
                <p>Use TissQL to query your TissDB collections. Supports SELECT, INSERT, UPDATE, DELETE.</p>
                <span class="badge" data-tooltip="TissDB uses a custom query language similar to SQL but optimized for NoSQL.">Query Language Info</span>
            </div>
            <div class="help-section" style="margin-top: 1rem">
                <h4>Model Playground</h4>
                <p>Interact with QuantaTissu. Use the <strong data-tooltip="Retrieval-Augmented Generation: Fetches context from TissDB before generating.">RAG</strong> toggle to enhance responses with your knowledge base.</p>
            </div>
            <div class="help-section" style="margin-top: 1rem">
                <h4>Nexus Flow</h4>
                <p>Visualize the interconnections between system components in real-time.</p>
            </div>
        </div>
    `,
    tests: `
        <div class="card">
            <h3>Test Orchestration</h3>
            <div id="test-results-area">
                <p>Status: <span id="test-status">Idle</span></p>
                <div id="test-details" class="results">Results will appear here...</div>
            </div>
        </div>
    `,
    admin: `
        <div class="card">
            <h3>Database Administration</h3>
            <div class="help-section">
                <h4>Collection Migration</h4>
                <p>Migrate data between TissDB collections.</p>
                <div class="grid" style="grid-template-columns: 1fr 1fr; margin-top: 1rem">
                    <div>
                        <label>Source Collection</label>
                        <select id="migrate-source-select"></select>
                    </div>
                    <div>
                        <label>Target Collection</label>
                        <input type="text" id="migrate-target-input" placeholder="New collection name">
                    </div>
                </div>
                <button class="btn btn-primary" style="margin-top: 1rem" onclick="runMigration()">Run Migration</button>
                <div id="migration-results" class="results" style="display:none"></div>
            </div>
        </div>
    `
};

function switchTab(tab) {
    state.activeTab = tab;
    document.getElementById('tab-title').innerText = tab.charAt(0).toUpperCase() + tab.slice(1);
    document.getElementById('tab-content').innerHTML = views[tab];

    document.querySelectorAll('.nav-item').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.tab === tab);
    });

    if (tab === 'dashboard') loadStats();
    if (tab === 'explorer') loadDatabases();
    if (tab === 'admin') loadAdminCollections();
    if (tab === 'config') loadConfig();
    if (tab === 'nexus') {
        if (window.initCanvas) window.initCanvas();
    }
}

// Modals
function openModal(id, data = {}) {
    const overlay = document.getElementById('modal-overlay');
    const content = document.getElementById('modal-content');
    overlay.style.display = 'flex';

    if (id === 'modal-prompt') {
        content.innerHTML = `
            <h3>${data.name}</h3>
            <p>Enter your prompt and generation parameters for the ${data.type}.</p>
            <input type="text" id="modal-prompt-input" placeholder="Enter prompt...">
            <div class="grid" style="grid-template-columns: 1fr 1fr; margin-top: 1rem">
                <div>
                    <label>Temp</label>
                    <input type="number" id="modal-temp" value="0.8" step="0.1">
                </div>
                <div>
                    <label>Tokens</label>
                    <input type="number" id="modal-len" value="50">
                </div>
            </div>
            <button class="btn btn-primary" style="margin-top: 1rem; width: 100%" onclick="runModalInference('${data.type}')">Run Inference</button>
            <div id="modal-response" class="results" style="display:none"></div>
        `;
    } else if (id === 'modal-add-collection') {
        content.innerHTML = `
            <h3>Add New Collection</h3>
            <input type="text" id="modal-new-coll" placeholder="Collection Name">
            <button class="btn btn-primary" style="width: 100%" onclick="createCollectionModal()">Create</button>
        `;
    } else if (id === 'modal-delete-collection') {
        content.innerHTML = `
            <h3>Delete Collection</h3>
            <p style="color: #dc2626; font-size: 0.9rem; margin-bottom: 1rem">Warning: This action is permanent.</p>
            <select id="modal-del-coll-select"></select>
            <button class="btn btn-primary" style="background: #dc2626; width: 100%" onclick="confirmDeleteModal()">Delete</button>
        `;
        loadCollectionsToModal();
    } else if (id === 'modal-confirm-delete') {
         content.innerHTML = `
            <h3>Confirm Deletion</h3>
            <p>Are you sure you want to delete collection "<strong>${data.coll}</strong>"?</p>
            <div style="margin-top: 1.5rem; display: flex; gap: 1rem">
                <button class="btn btn-primary" style="background: #dc2626; flex: 1" onclick="executeDelete('${data.coll}')">Yes, Delete</button>
                <button class="btn btn-secondary" style="flex: 1" onclick="closeModals()">Cancel</button>
            </div>
        `;
    } else if (id === 'modal-about') {
        content.innerHTML = `
            <h3>About QuantaTissu</h3>
            <p>Version 1.0.0 "Stellar Baseline"</p>
            <p style="margin-top: 1rem">A modern web-orchestrated platform for indigenous Transformer stacks, distributed NoSQL storage, and market analytics.</p>
            <p style="margin-top: 1rem; font-style: italic">"Think clearly. Compute lightly."</p>
        `;
    }
}

function closeModals() {
    document.getElementById('modal-overlay').style.display = 'none';
}

async function runModalInference(type) {
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

async function loadCollectionsToModal() {
    const res = await fetch('/api/db/collections', { method: 'POST', body: JSON.stringify({ db_name: state.dbName }) });
    const colls = await res.json();
    const select = document.getElementById('modal-del-coll-select');
    if (select) {
        select.innerHTML = colls.map(c => `<option value="${c}">${c}</option>`).join('');
    }
}

function confirmDeleteModal() {
    const coll = document.getElementById('modal-del-coll-select').value;
    openModal('modal-confirm-delete', { coll });
}

async function executeDelete(coll) {
    await fetch('/api/db/collection', {
        method: 'DELETE',
        body: JSON.stringify({ db_name: state.dbName, collection: coll })
    });
    alert('Deleted!');
    closeModals();
}

async function createCollectionModal() {
    const coll = document.getElementById('modal-new-coll').value;
    await fetch('/api/db/collection', {
        method: 'POST',
        body: JSON.stringify({ db_name: state.dbName, collection: coll })
    });
    alert('Created!');
    closeModals();
}

// Initial View
switchTab('dashboard');

// API Functions
async function loadStats() {
    try {
        const res = await fetch('/api/db/stats', { method: 'POST', body: JSON.stringify({ db_name: state.dbName }) });
        const data = await res.json();
        const el = document.getElementById('db-stats-val');
        if (el) el.innerHTML = `<p>Total Docs: ${data.total_docs || 0}</p><p>Feedback: ${data.feedback_entries || 0}</p>`;
    } catch(e) {}
}

async function loadDatabases() {
    try {
        const res = await fetch('/api/db/databases');
        const dbs = await res.json();
        const select = document.getElementById('db-select');
        if (select) {
            select.innerHTML = dbs.map(db => `<option value="${db}" ${db === state.dbName ? 'selected' : ''}>${db}</option>`).join('');
            loadCollections();
        }
    } catch(e) {}
}

async function loadCollections() {
    const dbSelect = document.getElementById('db-select');
    if (!dbSelect) return;
    try {
        const res = await fetch('/api/db/collections', { method: 'POST', body: JSON.stringify({ db_name: dbSelect.value }) });
        const colls = await res.json();
        const collSelect = document.getElementById('coll-select');
        if (collSelect) collSelect.innerHTML = colls.map(c => `<option value="${c}">${c}</option>`).join('');
    } catch(e) {}
}

async function runQuery() {
    const query = document.getElementById('query-input').value;
    const coll = document.getElementById('coll-select').value;
    const res = await fetch('/api/db/query', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ collection: coll, query })
    });
    const data = await res.json();
    document.getElementById('query-results').innerText = JSON.stringify(data, null, 2);
}

async function generate(mode = 'python') {
    const prompt = document.getElementById('prompt-input').value;
    const temp = parseFloat(document.getElementById('temp-input').value);
    const len = parseInt(document.getElementById('len-input').value);
    const rag = document.getElementById('rag-checkbox').checked;
    const responseEl = document.getElementById('model-response');
    responseEl.innerText = 'Thinking...';
    try {
        const endpoint = mode === 'cpp' ? '/api/model/cpp/generate' : '/api/model/generate';
        const res = await fetch(endpoint, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ prompt, length: len, temperature: temp, use_rag: rag })
        });
        const data = await res.json();
        responseEl.innerText = data.generated_text;
    } catch (e) { responseEl.innerText = 'Error: ' + e.message; }
}

async function runCategorizedTests(type) {
    switchTab('tests');
    const statusText = document.getElementById('test-status');
    const detailsEl = document.getElementById('test-details');
    statusText.innerText = 'Running ' + type + ' tests...';
    detailsEl.innerText = '';
    try {
        const res = await fetch('/api/tests/run', {
            method: 'POST',
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

async function saveConfiguration() {
    const config = {
        model: {
            n_embd: parseInt(document.getElementById('conf-n_embd')?.value || 128),
            n_layer: parseInt(document.getElementById('conf-n_layer')?.value || 4),
            n_head: parseInt(document.getElementById('conf-n_head')?.value || 8),
            d_ff: parseInt(document.getElementById('conf-d_ff')?.value || 512),
            dropout_rate: parseFloat(document.getElementById('conf-dropout')?.value || 0.1),
            layer_norm_eps: parseFloat(document.getElementById('conf-eps')?.value || 1e-6),
        },
        training: {
            learning_rate: parseFloat(document.getElementById('conf-lr')?.value || 0.0001),
            batch_size: parseInt(document.getElementById('conf-batch')?.value || 1),
            num_epochs: parseInt(document.getElementById('conf-epochs')?.value || 5),
            weight_decay: parseFloat(document.getElementById('conf-wd')?.value || 0.01),
        }
    };
    await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    });
    alert('Configuration persisted to server.');
}

async function exportConfiguration() {
    const res = await fetch('/api/config');
    const data = await res.json();
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'model_config.json';
    a.click();
}

async function importConfiguration() {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = 'application/json';
    input.onchange = e => {
        const file = e.target.files[0];
        const reader = new FileReader();
        reader.onload = async event => {
            const config = JSON.parse(event.target.result);
            await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            alert('Configuration imported and saved!');
            if (state.activeTab === 'config') loadConfig();
        };
        reader.readAsText(file);
    };
    input.click();
}

async function loadConfig() {
    const res = await fetch('/api/config');
    const data = await res.json();
    document.getElementById('conf-n_embd').value = data.model.n_embd;
    document.getElementById('conf-n_layer').value = data.model.n_layer;
    document.getElementById('conf-n_head').value = data.model.n_head;
    document.getElementById('conf-d_ff').value = data.model.d_ff;
    document.getElementById('conf-dropout').value = data.model.dropout_rate;
    document.getElementById('conf-eps').value = data.model.layer_norm_eps;
    document.getElementById('conf-lr').value = data.training.learning_rate;
    document.getElementById('conf-batch').value = data.training.batch_size;
    document.getElementById('conf-epochs').value = data.training.num_epochs;
    document.getElementById('conf-wd').value = data.training.weight_decay;
    checkTrainingStatus();
}

async function saveConfig() {
    const config = {
        model: {
            n_embd: parseInt(document.getElementById('conf-n_embd').value),
            n_layer: parseInt(document.getElementById('conf-n_layer').value),
            n_head: parseInt(document.getElementById('conf-n_head').value),
            d_ff: parseInt(document.getElementById('conf-d_ff').value),
            dropout_rate: parseFloat(document.getElementById('conf-dropout').value),
            layer_norm_eps: parseFloat(document.getElementById('conf-eps').value),
        },
        training: {
            learning_rate: parseFloat(document.getElementById('conf-lr').value),
            batch_size: parseInt(document.getElementById('conf-batch').value),
            num_epochs: parseInt(document.getElementById('conf-epochs').value),
            weight_decay: parseFloat(document.getElementById('conf-wd').value),
        }
    };
    await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    });
    alert('Configuration saved!');
}

async function startTraining() {
    const res = await fetch('/api/training/start', { method: 'POST' });
    if (res.ok) checkTrainingStatus();
    else { const err = await res.json(); alert(err.error); }
}

let trainingInterval = null;
async function checkTrainingStatus() {
    const res = await fetch('/api/training/status');
    const data = await res.json();
    const statusText = document.getElementById('train-status-text');
    const progressContainer = document.getElementById('progress-bar-container');
    const progressFill = document.getElementById('progress-bar-fill');
    const startBtn = document.getElementById('start-train-btn');

    if (statusText) {
        if (data.active) {
            statusText.innerText = `Training... ${data.progress}%`;
            progressContainer.style.display = 'block';
            progressFill.style.width = data.progress + '%';
            startBtn.disabled = true;
            if (!trainingInterval) trainingInterval = setInterval(checkTrainingStatus, 1000);
        } else {
            statusText.innerText = data.progress === 100 ? 'Completed' : 'Idle';
            progressContainer.style.display = data.progress === 100 ? 'block' : 'none';
            progressFill.style.width = data.progress + '%';
            startBtn.disabled = false;
            if (trainingInterval) { clearInterval(trainingInterval); trainingInterval = null; }
        }
    }
}

async function runTissLang() {
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

async function loadAnalyticsFromServer() {
    const res = await fetch('/api/analytics/chart', { method: 'POST' });
    const data = await res.json();
    if (window.createBarChart) document.getElementById('chart-container').innerHTML = window.createBarChart(data.data);
}

async function loadAdminCollections() {
    const res = await fetch('/api/db/collections', { method: 'POST', body: JSON.stringify({ db_name: state.dbName }) });
    const colls = await res.json();
    const select = document.getElementById('migrate-source-select');
    if (select) {
        select.innerHTML = colls.map(c => `<option value="${c}">${c}</option>`).join('');
    }
}

async function runMigration() {
    const source = document.getElementById('migrate-source-select').value;
    const target = document.getElementById('migrate-target-input').value;
    const resEl = document.getElementById('migration-results');
    resEl.style.display = 'block';
    resEl.innerText = 'Migrating...';

    try {
        const res = await fetch('/api/admin/migrate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ source_collection: source, target_collection: target })
        });
        const data = await res.json();
        resEl.innerText = `Success: Migrated ${data.migrated_count} documents from ${data.source} to ${data.target}.`;
    } catch(e) { resEl.innerText = 'Error: ' + e.message; }
}

async function runTradingEngine() {
    const symbol = document.getElementById('symbol-input').value;
    const resultsEl = document.getElementById('trading-results');
    resultsEl.innerText = 'Analyzing...';
    try {
        const res = await fetch('/api/analytics/trade', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ symbol })
        });
        const data = await res.json();
        resultsEl.innerHTML = `<div style="display: flex; gap: 2rem; align-items: center;"><div><p>Symbol: <strong>${data.symbol}</strong></p><p>Price: ${data.market_data.price}</p><p>Signal: <span style="color: ${data.signal === 'BUY' ? 'green' : 'red'}; font-weight: bold">${data.signal}</span></p></div><div id="candlestick-container"></div></div>`;
        if (window.createCandlestickChart) document.getElementById('candlestick-container').innerHTML = window.createCandlestickChart(data.historical_data, 400, 200);
    } catch (e) { resultsEl.innerText = 'Error: ' + e.message; }
}

window.openModal = openModal;
window.closeModals = closeModals;
window.runModalInference = runModalInference;
window.createCollectionModal = createCollectionModal;
window.confirmDeleteModal = confirmDeleteModal;
window.executeDelete = executeDelete;
window.runCategorizedTests = runCategorizedTests;
window.runMigration = runMigration;
window.switchTab = switchTab;
window.runTradingEngine = runTradingEngine;
window.saveConfiguration = saveConfiguration;
window.importConfiguration = importConfiguration;
window.exportConfiguration = exportConfiguration;
