/**
 * QuantaTissu Platform CLI API
 * Provides a command-line interface to the web platform modules.
 */

const fs = require('fs');
const path = require('path');
const { vm } = require('node:vm');

// --- Mocking Browser Environment ---
global.window = global;
global.document = {
    addEventListener: () => {},
    getElementById: (id) => ({
        value: '',
        innerText: '',
        innerHTML: '',
        style: {},
        dataset: {},
        previousElementSibling: null,
        appendChild: () => {},
        querySelector: () => null,
        querySelectorAll: () => []
    }),
    querySelector: () => null,
    querySelectorAll: () => []
};
global.navigator = {
    clipboard: {
        writeText: async () => {}
    }
};
global.alert = (msg) => console.log(`[ALERT] ${msg}`);
global.setTimeout = setTimeout;
global.setInterval = setInterval;
global.clearInterval = clearInterval;

// Mock fetch
global.fetch = async (url, options = {}) => {
    // console.log(`[FETCH] ${options.method || 'GET'} ${url}`);
    // In a real CLI we would use 'http' module or similar to call the actual backend.
    // For this mock, we'll try to use the backend if it's running, otherwise return mocks.
    const http = require('http');
    const [host, port] = ['localhost', 8000]; // Default platform port

    return new Promise((resolve, reject) => {
        const urlObj = new URL(url, `http://${host}:${port}`);
        const reqOptions = {
            hostname: urlObj.hostname,
            port: urlObj.port || port,
            path: urlObj.pathname + urlObj.search,
            method: options.method || 'GET',
            headers: options.headers || {}
        };

        const req = http.request(reqOptions, (res) => {
            let data = '';
            res.on('data', (chunk) => data += chunk);
            res.on('end', () => {
                resolve({
                    ok: res.statusCode >= 200 && res.statusCode < 300,
                    status: res.statusCode,
                    json: async () => JSON.parse(data),
                    text: async () => data
                });
            });
        });

        req.on('error', (e) => {
            // Fallback for when backend is not running during simple unit tests
            // if (url.includes('/api/config')) {
            //     resolve({ ok: true, json: async () => ({ model: {}, training: {} }) });
            // }
            reject(e);
        });

        if (options.body) {
            req.write(options.body);
        }
        req.end();
    });
};

// --- Load Platform Modules ---
const projectRoot = path.resolve(__dirname, '../../../../');
const modulesDir = path.join(projectRoot, 'web_platform/frontend/js/modules');

const vm_module = require('vm');
function loadModule(file) {
    const code = fs.readFileSync(path.join(modulesDir, file), 'utf8');
    const script = new vm_module.Script(code);
    script.runInThisContext();
}

loadModule('state.js');
loadModule('db_module.js');
loadModule('model_module.js');
loadModule('admin_module.js');
loadModule('analyzer_module.js');
loadModule('test_module.js');
loadModule('tisslang_module.js');
loadModule('ui_module.js');

const STATE_FILE = path.join(projectRoot, '.platform_cli_state.json');

function saveCLIState() {
    try {
        const state = {
            AppState: {
                tabs: AppState.tabs,
                tasks: AppState.tasks,
                db: AppState.db,
                config: AppState.config
            },
            ModelModuleState: ModelModule.state,
            AdminModuleState: AdminModule.state
        };
        fs.writeFileSync(STATE_FILE, JSON.stringify(state, null, 2));
    } catch (e) {
        console.error('Failed to save CLI state:', e);
    }
}

function loadCLIState() {
    try {
        if (fs.existsSync(STATE_FILE)) {
            const data = JSON.parse(fs.readFileSync(STATE_FILE, 'utf8'));
            if (data.AppState) Object.assign(AppState, data.AppState);
            if (data.ModelModuleState) Object.assign(ModelModule.state, data.ModelModuleState);
            if (data.AdminModuleState) Object.assign(AdminModule.state, data.AdminModuleState);
        }
    } catch (e) {
        console.error('Failed to load CLI state:', e);
    }
}

// Initial load
loadCLIState();

// --- CLI API Implementation ---
const PlatformAPI = {
    async switch_tab(tab_name) {
        window.switchTab(tab_name);
        return { status: 'success', active_tab: AppState.tabs.active };
    },

    async train(params = {}) {
        // Set mock inputs if params are provided
        if (params.epochs) document.getElementById('conf-epochs').value = params.epochs;
        if (params.batch_size) document.getElementById('conf-batch').value = params.batch_size;

        await AdminModule.executeStartTraining();
        return { status: 'started', task_id: 'training_job' };
    },

    async generate(prompt, params = {}) {
        const doc = global.document;
        // Mock the elements used by ModelModule.generate
        const mockElements = {
            'prompt-input': { value: prompt },
            'temp-input': { value: params.temperature || 0.8 },
            'len-input': { value: params.length || 50 },
            'rag-checkbox': { checked: false },
            'model-response': { innerText: '' }
        };

        const oldGetElementById = doc.getElementById;
        doc.getElementById = (id) => mockElements[id] || oldGetElementById(id);

        await ModelModule.generate(params.mode || 'python');

        const response = mockElements['model-response'].innerText;
        doc.getElementById = oldGetElementById; // Restore

        return { status: 'success', response: response };
    },

    async query_db(query, collection) {
        document.getElementById('query-input').value = query;
        const select = document.getElementById('coll-select');
        select.value = collection;

        await DBModule.runQuery();
        return { status: 'success', results: document.getElementById('query-results').innerText };
    },

    async get_task_status(task_id) {
        const res = await fetch(`/api/tasks/${task_id}`);
        const data = await res.json();
        return data;
    },

    async list_tasks() {
        return AppState.tasks;
    },

    async get_state() {
        return AppState;
    },

    async get_module_state(module_name) {
        if (module_name === 'ModelModule') return ModelModule.state;
        if (module_name === 'AdminModule') return AdminModule.state;
        return { error: 'Module not found' };
    },

    async build_analyzer() {
        await AnalyzerModule.executeBuild();
        return { status: 'finished', output: document.getElementById('analyzer-results').innerText };
    },

    async start_analyzer(session_id = 0) {
        document.getElementById('analyzer-session-id').value = session_id;
        await AnalyzerModule.executeStart();
        return { status: 'started', output: document.getElementById('analyzer-results').innerText };
    },

    async stop_analyzer() {
        await AnalyzerModule.stop();
        return { status: 'stopped', output: document.getElementById('analyzer-results').innerText };
    },

    async run_test_script(script_name, overrides = {}) {
        const select = document.getElementById('script-select');
        select.value = script_name;
        // Mock overrides would need more detailed document element mocking if we use the UI fields
        // For simplicity, we can call the handler directly if exposed, or mock the inputs
        if (overrides.epochs) document.getElementById('test-arg-epochs').value = overrides.epochs;

        await TestModule.executeRunScript();
        return { status: 'started', task_id: TestModule.state.currentTaskId };
    },

    async clear_state() {
        if (fs.existsSync(STATE_FILE)) fs.unlinkSync(STATE_FILE);
        return { status: 'cleared' };
    },

    async create_platform_logs() {
        const res = await fetch('/api/db/collection', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ db_name: 'main_db', collection: 'platform_logs' })
        });
        const data = await res.json();
        return data;
    }
};

// --- Command Execution ---
if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.log(JSON.stringify({ error: 'No command provided' }));
        process.exit(1);
    }

    const command = args[0];
    const params = args[1] ? JSON.parse(args[1]) : {};

    if (PlatformAPI[command]) {
        PlatformAPI[command](params)
            .then(result => {
                saveCLIState();
                console.log(JSON.stringify(result, null, 2));
                process.exit(0);
            })
            .catch(err => {
                console.error(JSON.stringify({ error: err.message }));
                process.exit(1);
            });
    } else {
        console.error(JSON.stringify({ error: `Unknown command: ${command}` }));
        process.exit(1);
    }
}

module.exports = PlatformAPI;
