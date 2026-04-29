const UIModule = {
    state: {
        activeTab: 'dashboard',
        dbName: 'main_db'
    },

    views: {
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
                    <select id="db-select" onchange="DBModule.loadCollections()"></select>
                    <select id="coll-select"></select>
                </div>
                <h3>TissQL Query</h3>
                <textarea id="query-input" rows="4">SELECT * FROM knowledge</textarea>
                <button class="btn btn-primary" onclick="DBModule.runQuery()">Execute Query</button>
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
                    <button class="btn btn-primary" onclick="ModelModule.generate('python')" data-tooltip="Run inference using the Python-based QuantaTissu model.">Send (Python)</button>
                    <button class="btn btn-primary" onclick="ModelModule.generate('cpp')" style="background: #059669" data-tooltip="Run inference using the high-performance indigenous C++ Transformer stack.">Send (C++)</button>
                </div>
                <input type="text" id="prompt-input" placeholder="Ask me anything...">
                <div id="model-response" class="results">Response will appear here...</div>
            </div>
        `,
        analytics: `
            <div class="card">
                <h3>Performance Analytics</h3>
                <button class="btn btn-primary" onclick="AnalyticsModule.loadAnalyticsFromServer()">Fetch Chart Data</button>
                <div id="chart-container" style="margin-top: 1rem"></div>
                <hr style="margin: 1rem 0">
                <h3>Trading Signal Engine</h3>
                <input type="text" id="symbol-input" value="AAPL" style="width: 100px; margin-right: 1rem">
                <button class="btn btn-primary" onclick="AnalyticsModule.runTradingEngine()">Generate Signal</button>
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
                <button class="btn btn-primary" onclick="TissLangModule.runTissLang()">Run Workflow</button>
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
                    <button class="btn btn-primary" onclick="AdminModule.saveConfig()">Save Architecture</button>
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
                    <button class="btn btn-primary" onclick="AdminModule.saveConfig()">Save Hyperparams</button>
                </div>
            </div>
            <div class="card" style="margin-top: 1rem">
                <h3>Training Control</h3>
                <div id="training-status-area">
                    <p>Status: <span id="train-status-text">Idle</span></p>
                    <div id="progress-bar-container" style="width: 100%; height: 20px; background: #eee; border-radius: 10px; overflow: hidden; display: none;">
                        <div id="progress-bar-fill" style="width: 0%; height: 100%; background: #4a90e2; transition: width 0.3s"></div>
                    </div>
                    <button id="start-train-btn" class="btn btn-primary" style="margin-top: 1rem" onclick="AdminModule.startTraining()">Start Training Job</button>
                </div>
            </div>
        `,
        help: `
            <div class="card">
                <h3>Platform Documentation</h3>
                <div class="help-section">
                    <h4>Web Orchestration Platform</h4>
                    <p>A modular, dependency-free dashboard for the QuantaTissu ecosystem. It provides a high-level interface for model interaction, database management, and system monitoring.</p>
                    <ul>
                        <li><strong>Backend:</strong> Python-based <code>http.server</code> with modular domain handlers.</li>
                        <li><strong>Frontend:</strong> Vanilla JS modules for performance and zero-dependency reliability.</li>
                    </ul>
                </div>
                <div class="help-section" style="margin-top: 1rem">
                    <h4>Runtime Model Analyzer (RMA)</h4>
                    <p>The RMA is a dual-process monitoring framework. It uses <strong>POSIX Shared Memory</strong> to track the C++ model's execution in real-time, catching arithmetic anomalies like <code>NaN</code> or <code>Division by Zero</code> without interrupting the primary model flow. Use the <strong>Analyzer</strong> tab to build and start the monitor.</p>
                </div>
                <div class="help-section" style="margin-top: 1rem">
                    <h4>Data Explorer</h4>
                    <p>Use TissQL to query your TissDB collections. Supports SELECT, INSERT, UPDATE, DELETE.</p>
                    <span class="badge" data-tooltip="TissDB uses a custom query language similar to SQL but optimized for NoSQL.">Query Language Info</span>
                </div>
                <div class="help-section" style="margin-top: 1rem">
                    <h4>Model Playground & Testing</h4>
                    <p>Interact with QuantaTissu using <strong data-tooltip="Retrieval-Augmented Generation: Fetches context from TissDB before generating.">RAG</strong> or run automated workout scripts from the <strong>Tests</strong> tab to verify model behavior.</p>
                </div>
                <div class="help-section" style="margin-top: 1rem">
                    <h4>TissDB Lifecycle</h4>
                    <p>The <strong>Admin</strong> tab allows you to build the native C++ database from source and manage its process lifecycle directly from the browser.</p>
                </div>
                <div class="help-section" style="margin-top: 1rem">
                    <h4>Nexus Flow</h4>
                    <p>Visualize the interconnections between system components in real-time via the interactive canvas.</p>
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
                <hr style="margin: 2rem 0">
                <h3>Model Script Runner</h3>
                <p style="color: #666; font-size: 0.8rem; margin-bottom: 1rem">Select a script and override parameters for this specific run.</p>
                
                <div class="grid" style="grid-template-columns: 1fr 1fr; gap: 1rem; margin-bottom: 1rem;">
                    <div class="card" style="padding: 0.5rem; background: #f9fafb">
                        <h4 style="margin-top: 0">Model Overrides</h4>
                        <div class="form-group">
                            <label style="font-size: 0.7rem">Embed Dim (Default: 128)</label>
                            <select id="test-arg-n_embd">
                                <option value="">Default</option>
                                <option value="64">64</option>
                                <option value="128">128</option>
                                <option value="256">256</option>
                                <option value="512">512</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label style="font-size: 0.7rem">Layers (Default: 4)</label>
                            <select id="test-arg-n_layer">
                                <option value="">Default</option>
                                <option value="1">1</option>
                                <option value="2">2</option>
                                <option value="4">4</option>
                                <option value="8">8</option>
                            </select>
                        </div>
                    </div>
                    <div class="card" style="padding: 0.5rem; background: #f9fafb">
                        <h4 style="margin-top: 0">Training Overrides</h4>
                        <div class="form-group">
                            <label style="font-size: 0.7rem">Epochs (Default: 5)</label>
                            <select id="test-arg-epochs">
                                <option value="">Default</option>
                                <option value="1">1</option>
                                <option value="3">3</option>
                                <option value="5">5</option>
                                <option value="10">10</option>
                                <option value="20">20</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label style="font-size: 0.7rem">Learning Rate (Default: 1e-4)</label>
                            <select id="test-arg-lr">
                                <option value="">Default</option>
                                <option value="0.0001">1e-4</option>
                                <option value="0.0005">5e-4</option>
                                <option value="0.001">1e-3</option>
                                <option value="0.005">5e-3</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label style="display: flex; align-items: center; gap: 0.5rem; font-size: 0.8rem; cursor: pointer; margin-top: 0.5rem;">
                                <input type="checkbox" id="test-arg-skip-compile" style="width: auto; margin-bottom: 0;"> Skip Compilation
                            </label>
                        </div>
                    </div>
                </div>

                <div style="display: flex; gap: 1rem; align-items: center">
                    <select id="script-select" style="flex: 1; margin-bottom: 0"></select>
                    <button class="btn btn-primary" onclick="TestModule.runScript()">Run Script</button>
                    <button id="stop-script-btn" class="btn btn-primary" style="background: #dc2626; display: none;" onclick="TestModule.stopScript()">Stop Script</button>
                </div>
                <div style="position: relative; margin-top: 1rem;">
                    <button class="btn" style="position: absolute; right: 1rem; top: 1rem; background: #374151; color: white; padding: 0.3rem 0.6rem; font-size: 0.7rem; z-index: 10;" onclick="TestModule.copyResults()">Copy Output</button>
                    <div id="script-results" class="results" style="height: 400px; overflow: auto; background: #111827; color: #e5e7eb; font-family: 'Fira Code', 'Courier New', monospace; border: 1px solid #374151; line-height: 1.5; padding: 1.25rem;">
                        Select a script and click "Run Script"
                    </div>
                </div>
            </div>
        `,
        admin: `
            <div class="card">
                <h3>Platform Administration</h3>
                <p>General system settings and administrative tasks.</p>
                <hr style="margin: 2rem 0">
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
                    <button class="btn btn-primary" style="margin-top: 1rem" onclick="AdminModule.runMigration()">Run Migration</button>
                    <div id="migration-results" class="results" style="display:none"></div>
                </div>
            </div>
        `,
        db_mgmt: `
            <div class="card">
                <h3>Database Management</h3>
                <div class="grid" style="grid-template-columns: 1fr 1fr; gap: 1rem;">
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>Lifecycle Control</h4>
                        <p style="font-size: 0.8rem; color: #666; margin-bottom: 1rem;">Build, start, and stop the native TissDB instance.</p>
                        <button class="btn btn-primary" style="width: 100%; margin-bottom: 0.5rem; background: #4b5563" onclick="DBModule.buildTissDB()">Build TissDB (Native)</button>
                        <div style="display: flex; gap: 0.5rem;">
                            <button class="btn btn-primary" style="flex: 1" onclick="DBModule.startTissDB()">Start Server</button>
                            <button class="btn btn-primary" style="flex: 1; background: #dc2626" onclick="DBModule.stopTissDB()">Stop Server</button>
                        </div>
                    </div>
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>System Status</h4>
                        <p>Status: <span id="db-status-text">Checking...</span></p>
                        <div id="db-stats-val" style="font-size: 0.9rem; margin-top: 0.5rem;">Loading stats...</div>
                        <hr style="margin: 0.5rem 0">
                        <div id="db-lifecycle-results" style="font-size: 0.7rem; font-family: monospace; white-space: pre-wrap; max-height: 100px; overflow-y: auto;">Idle...</div>
                    </div>
                </div>

                <div class="grid" style="grid-template-columns: 1fr 1fr; gap: 1rem; margin-top: 1rem;">
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>Collections</h4>
                        <div style="margin-bottom: 1rem">
                            <label>Select Database</label>
                            <select id="db-select" onchange="DBModule.loadCollections()" style="width: 100%"></select>
                        </div>
                        <div style="display: flex; gap: 0.5rem; margin-top: 1rem;">
                            <button class="btn btn-primary" style="flex: 1" onclick="UIModule.openModal('modal-add-collection')">Create Collection</button>
                            <button class="btn btn-primary" style="flex: 1; background: #dc2626" onclick="UIModule.openModal('modal-delete-collection')">Delete Collection</button>
                        </div>
                    </div>
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>Tools</h4>
                        <button class="btn btn-primary" style="width: 100%; margin-bottom: 0.5rem;" onclick="switchTab('explorer')">Open Data Explorer</button>
                        <button class="btn btn-primary" style="width: 100%; background: #059669" onclick="AdminModule.loadAdminCollections(); switchTab('admin')">Data Migration</button>
                    </div>
                </div>
            </div>
        `,
        analyzer: `
            <div class="card">
                <h3>Runtime Model Analyzer (RMA)</h3>
                <p style="color: #666; font-size: 0.9rem; margin-bottom: 1rem;">
                    Monitor C++ model execution for arithmetic anomalies (NaN, DivZero) via POSIX Shared Memory.
                </p>
                <div class="grid" style="grid-template-columns: 1fr 1fr 1fr; gap: 1rem; margin-bottom: 1rem;">
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>Controls</h4>
                        <button class="btn btn-primary" style="width: 100%; margin-bottom: 0.5rem; background: #4b5563" onclick="AnalyzerModule.build()">Build Analyzer</button>
                        <div style="display: flex; gap: 0.5rem;">
                            <input type="number" id="analyzer-session-id" value="0" style="width: 60px; margin-bottom: 0" title="Session ID">
                            <button class="btn btn-primary" style="flex: 1; margin-bottom: 0;" onclick="AnalyzerModule.start()">Start</button>
                        </div>
                        <button class="btn btn-primary" style="width: 100%; margin-top: 0.5rem; background: #dc2626" onclick="AnalyzerModule.stop()">Stop</button>
                    </div>
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>Status</h4>
                        <p>Status: <span id="analyzer-status-text">Checking...</span></p>
                        <hr style="margin: 0.5rem 0">
                        <div id="analyzer-results" style="font-size: 0.8rem; font-family: monospace; white-space: pre-wrap; max-height: 100px; overflow-y: auto;">Idle...</div>
                    </div>
                    <div class="card" style="background: #f9fafb; border: 1px solid #e5e7eb;">
                        <h4>Info</h4>
                        <p style="font-size: 0.8rem;">Session IPC: <code>/dev/shm/rma_shm_*</code></p>
                        <p style="font-size: 0.8rem;">Logs: <code>analyzer_log.txt</code></p>
                    </div>
                </div>
                <h4>Error Stream</h4>
                <div id="analyzer-log-stream" class="results" style="height: 300px; font-family: 'Courier New', monospace; background: #1a1a1a; color: #00ff00; overflow-y: auto;">
                    Waiting for analyzer process...
                </div>
            </div>
        `
    },

    switchTab(tab) {
        if (!this.views[tab]) return;
        this.state.activeTab = tab;
        document.getElementById('tab-title').innerText = tab.charAt(0).toUpperCase() + tab.slice(1);
        document.getElementById('tab-content').innerHTML = this.views[tab];

        document.querySelectorAll('.nav-item').forEach(btn => {
            btn.classList.toggle('active', btn.dataset.tab === tab);
        });

        if (tab === 'dashboard') DBModule.loadStats();
        if (tab === 'explorer') DBModule.loadDatabases();
        if (tab === 'db_mgmt') {
            DBModule.loadDatabases();
            DBModule.loadStats();
            DBModule.checkStatus();
        }
        if (tab === 'admin') {
            AdminModule.loadAdminCollections();
        }
        if (tab === 'tests') {
            TestModule.loadTestScripts();
            TestModule.refreshUI();
        }
        if (tab === 'config') AdminModule.loadConfig();
        if (tab === 'analyzer') AnalyzerModule.checkStatus();
        if (tab === 'nexus') {
            if (window.initCanvas) window.initCanvas();
        }
    },

    openModal(id, data = {}) {
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
                <button class="btn btn-primary" style="margin-top: 1rem; width: 100%" onclick="ModelModule.runModalInference('${data.type}')">Run Inference</button>
                <div id="modal-response" class="results" style="display:none"></div>
            `;
        } else if (id === 'modal-add-collection') {
            content.innerHTML = `
                <h3>Add New Collection</h3>
                <input type="text" id="modal-new-coll" placeholder="Collection Name">
                <button class="btn btn-primary" style="width: 100%" onclick="DBModule.createCollectionModal()">Create</button>
            `;
        } else if (id === 'modal-delete-collection') {
            content.innerHTML = `
                <h3>Delete Collection</h3>
                <p style="color: #dc2626; font-size: 0.9rem; margin-bottom: 1rem">Warning: This action is permanent.</p>
                <select id="modal-del-coll-select"></select>
                <button class="btn btn-primary" style="background: #dc2626; width: 100%" onclick="DBModule.confirmDeleteModal()">Delete</button>
            `;
            DBModule.loadCollectionsToModal();
        } else if (id === 'modal-confirm-delete') {
             content.innerHTML = `
                <h3>Confirm Deletion</h3>
                <p>Are you sure you want to delete collection "<strong>${data.coll}</strong>"?</p>
                <div style="margin-top: 1.5rem; display: flex; gap: 1rem">
                    <button class="btn btn-primary" style="background: #dc2626; flex: 1" onclick="DBModule.executeDelete('${data.coll}')">Yes, Delete</button>
                    <button class="btn btn-secondary" style="flex: 1" onclick="UIModule.closeModals()">Cancel</button>
                </div>
            `;
        } else if (id === 'modal-confirm-process') {
            content.innerHTML = `
                <h3>Security Warning: Local Process Execution</h3>
                <p style="margin-bottom: 1rem;">The platform is about to execute a command on the local machine:</p>
                <div style="background: #f3f4f6; padding: 1rem; border-radius: 4px; font-family: monospace; font-size: 0.9rem; border-left: 4px solid #f59e0b; margin-bottom: 1.5rem;">
                    <strong>Command/Action:</strong><br>
                    <span style="color: #d97706;">${data.command}</span>
                </div>
                <p style="font-size: 0.85rem; color: #666; margin-bottom: 1.5rem;">
                    Executing untrusted scripts can compromise your system. Only proceed if you initiated this action and trust the command.
                </p>
                <div style="display: flex; gap: 1rem;">
                    <button class="btn btn-primary" style="flex: 1; background: #d97706;" onclick="UIModule.confirmProcess('${data.callback}')">Proceed with Execution</button>
                    <button class="btn btn-secondary" style="flex: 1" onclick="UIModule.closeModals()">Cancel</button>
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
    },

    closeModals() {
        document.getElementById('modal-overlay').style.display = 'none';
    },

    confirmProcess(callbackName) {
        this.closeModals();
        // Resolve the callback from window/modules
        const parts = callbackName.split('.');
        let func = window;
        for (const part of parts) {
            func = func[part];
        }
        if (typeof func === 'function') {
            func();
        }
    }
};

window.UIModule = UIModule;
window.switchTab = UIModule.switchTab.bind(UIModule);
window.openModal = UIModule.openModal.bind(UIModule);
window.closeModals = UIModule.closeModals.bind(UIModule);
