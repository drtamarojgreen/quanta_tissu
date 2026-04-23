const test = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

// --- Comprehensive Mock Browser Environment ---
const createElementMock = (tag) => {
    const el = {
        tag,
        innerHTML: '',
        innerText: '',
        value: '',
        type: '',
        style: {},
        appendChild: (child) => { el.children.push(child); return child; },
        children: [],
        classList: {
            add: (c) => el.classes.add(c),
            remove: (c) => el.classes.delete(c),
            toggle: (c, val) => val ? el.classes.add(c) : el.classes.delete(c)
        },
        classes: new Set(),
        dataset: {},
        getContext: () => ({
            clearRect: () => {}, beginPath: () => {}, moveTo: () => {}, lineTo: () => {},
            stroke: () => {}, arc: () => {}, fill: () => {}, fillText: () => {}
        }),
        click: () => {
            if (el.onclick) el.onclick({ target: el });
            if (el.tag === 'input' && el.type === 'file' && el.onchange) {
                el.onchange({ target: { files: [{ name: 'test.json' }] } });
            }
        },
        setAttribute: (k, v) => { el[k] = v; },
        getAttribute: (k) => el[k]
    };
    return el;
};

const elements = {};
const getElementByIdMock = (id) => {
    if (!elements[id]) elements[id] = createElementMock('div');
    return elements[id];
};

const context = {
    document: {
        getElementById: getElementByIdMock,
        querySelectorAll: (sel) => {
            if (sel === '.nav-item') return [getElementByIdMock('nav-1')];
            return [];
        },
        createElement: createElementMock,
        body: { appendChild: (child) => {} },
        addEventListener: (event, fn) => {
            if (event === 'DOMContentLoaded') context._onDOMContentLoaded = fn;
        }
    },
    fetch: async (url, options = {}) => {
        if (url === '/api/db/stats') return { ok: true, json: async () => ({ total_docs: 100, feedback_entries: 5 }) };
        if (url === '/api/db/databases') return { ok: true, json: async () => ['main_db', 'test_db'] };
        if (url === '/api/db/collections') return { ok: true, json: async () => ['knowledge', 'users'] };
        if (url === '/api/config') return { ok: true, json: async () => ({ model: { n_embd: 128, n_layer: 4, n_head: 8, d_ff: 512, dropout_rate: 0.1, layer_norm_eps: 1e-6 }, training: { learning_rate: 0.0001, batch_size: 1, num_epochs: 5, weight_decay: 0.01 } }) };
        if (url === '/api/training/status') return { ok: true, json: async () => ({ active: false, progress: 100 }) };
        if (url === '/api/nexus/graph') return { ok: true, json: async () => ({ nodes: [{id: 'A', x:0, y:0, label: 'A'}], links: [] }) };
        if (url === '/api/analytics/chart') return { ok: true, json: async () => ({ data: [10, 20, 30] }) };
        if (url === '/api/tisslang/run') return { ok: true, json: async () => ({ log: [{status:'SUCCESS', command_type:'ASSERT'}], halted: false, vars: {} }) };
        if (url.includes('error')) return { ok: false, status: 500, json: async () => ({ error: 'Server Error' }) };
        return { ok: true, json: async () => ({ generated_text: 'mock response', results: [], summary: {passed:4, total:4}, status: 'success', migrated_count: 150, source: 'a', target: 'b', market_data: {price: 150.25}, symbol: 'AAPL', signal: 'BUY', historical_data: [{open:10, high:20, low:5, close:15}] }) };
    },
    localStorage: { getItem: () => null, setItem: () => {} },
    alert: (msg) => { context.lastAlert = msg; },
    URL: { createObjectURL: () => 'blob:url' },
    Blob: class { constructor(parts, opts) { this.parts = parts; this.opts = opts; } },
    FileReader: class {
        readAsText(file) {
            setTimeout(() => {
                this.onload({ target: { result: JSON.stringify({ model: { n_embd: 256 }, training: {} }) } });
            }, 0);
        }
    },
    setInterval: (fn) => { context.lastIntervalFn = fn; return 1; },
    clearInterval: () => {},
    Math: Math,
    console: console,
    JSON: JSON,
    parseFloat: parseFloat,
    parseInt: parseInt,
    setTimeout: setTimeout,
    Image: class {}
};
context.window = context;
context.global = context;

const visualizationCode = fs.readFileSync(path.join(__dirname, '../js/visualization.js'), 'utf8');
const candlestickCode = fs.readFileSync(path.join(__dirname, '../js/candlestick.js'), 'utf8');
const graphCode = fs.readFileSync(path.join(__dirname, '../js/graph.js'), 'utf8');

// We use var for modules in tests to avoid const re-declaration issues in VM
const loadModule = (filePath) => {
    let code = fs.readFileSync(path.join(__dirname, filePath), 'utf8');
    // Replace const with var for the main module object to ensure it attaches to context
    code = code.replace(/^const (\w+Module) =/, 'var $1 =');
    vm.runInContext(code, context);
};

vm.createContext(context);
vm.runInContext(visualizationCode, context);
vm.runInContext(candlestickCode, context);
vm.runInContext(graphCode, context);

loadModule('../js/modules/ui_module.js');
loadModule('../js/modules/db_module.js');
loadModule('../js/modules/model_module.js');
loadModule('../js/modules/analytics_module.js');
loadModule('../js/modules/tisslang_module.js');
loadModule('../js/modules/admin_module.js');
loadModule('../js/modules/test_module.js');

const appCode = fs.readFileSync(path.join(__dirname, '../js/app.js'), 'utf8');
vm.runInContext(appCode, context);

// --- Comprehensive Tests ---

test('Visualization: Bar Chart', () => {
    const svg = context.createBarChart([10, 50, 100]);
    assert.ok(svg.includes('<svg'));
    assert.ok(svg.includes('<rect'));
});

test('Visualization: Candlestick Chart', () => {
    const svg = context.createCandlestickChart([{ open: 10, high: 20, low: 5, close: 15 }], 400, 200);
    assert.ok(svg.includes('<svg'));
    assert.ok(svg.includes('fill="#22c55e"'));
});

test('App: Tab Switching', () => {
    context.switchTab('playground');
    assert.strictEqual(context.UIModule.state.activeTab, 'playground');
});

test('App: Database Browsing', async () => {
    await context.DBModule.loadDatabases();
    assert.ok(getElementByIdMock('db-select').innerHTML.includes('main_db'));
});

test('App: Configuration Management', async () => {
    await context.window.saveConfiguration();
    assert.strictEqual(context.lastAlert, 'Configuration persisted to server.');
    await context.window.importConfiguration();
    await new Promise(r => setTimeout(r, 10));
    assert.strictEqual(context.lastAlert, 'Configuration imported and saved!');
});

test('App: Training Orchestration', async () => {
    await context.AdminModule.startTraining();
    await context.AdminModule.checkTrainingStatus();
    assert.strictEqual(getElementByIdMock('train-status-text').innerText, 'Completed');
});

test('App: Model Inference', async () => {
    getElementByIdMock('prompt-input').value = 'test';
    await context.ModelModule.generate('python');
    assert.ok(getElementByIdMock('model-response').innerText.includes('mock response'));
});

test('App: TissLang Workflow', async () => {
    getElementByIdMock('tisslang-input').value = 'TASK "test"';
    await context.TissLangModule.runTissLang();
    assert.ok(getElementByIdMock('tisslang-results').innerText.includes('SUCCESS'));
});

test('App: Analytics & Trading', async () => {
    await context.AnalyticsModule.loadAnalyticsFromServer();
    assert.ok(getElementByIdMock('chart-container').innerHTML.includes('<svg'));
    await context.window.runTradingEngine();
    await new Promise(r => setTimeout(r, 10));
    assert.ok(getElementByIdMock('trading-results').innerHTML.includes('BUY'));
});

test('App: Modal & Admin Logic', async () => {
    // Modal open
    context.UIModule.openModal('modal-add-collection');
    assert.ok(getElementByIdMock('modal-overlay').style.display === 'flex');

    // Migration
    await context.AdminModule.runMigration();
    assert.ok(getElementByIdMock('migration-results').innerText.includes('Migrated 150 documents'));

    // Deletion
    await context.DBModule.executeDelete('knowledge');
    assert.strictEqual(context.lastAlert, 'Deleted!');
});

test('App: Modal Inference & Collections', async () => {
    getElementByIdMock('modal-prompt-input').value = 'query';
    await context.ModelModule.runModalInference('pipeline');
    assert.ok(getElementByIdMock('modal-response').innerText.includes('mock response'));

    await context.DBModule.createCollectionModal();
    assert.strictEqual(context.lastAlert, 'Created!');

    await context.DBModule.loadCollectionsToModal();
    assert.ok(getElementByIdMock('modal-del-coll-select').innerHTML.includes('knowledge'));
});

test('Graph: Canvas Drawing', async () => {
    await context.initCanvas();
    assert.ok(true);
});
