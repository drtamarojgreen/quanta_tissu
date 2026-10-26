const test = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

// --- Minimal browser environment for the Query Browser module ---
const elements = {};
const getElement = (id) => {
    if (!elements[id]) elements[id] = { id, value: '', innerHTML: '', innerText: '' };
    return elements[id];
};

let fetchHandler = async () => ({ ok: true, status: 200, text: async () => '[]', json: async () => [] });
const fetchCalls = [];

const context = {
    document: { getElementById: getElement },
    fetch: async (url, options = {}) => {
        fetchCalls.push({ url, options });
        return fetchHandler(url, options);
    },
    AppState: { db: { selected: 'main_db' } },
    console,
    JSON,
    Math,
    Number,
    String,
    Set,
    parseInt,
    parseFloat,
    TextEncoder,
    Promise,
    Error,
    // Used only for File objects without text() (older browsers).
    FileReader: class {
        readAsText(file) {
            setTimeout(() => this.onload({ target: { result: file.contents } }), 0);
        }
    }
};
context.window = context;
vm.createContext(context);
const code = fs.readFileSync(path.join(__dirname, '../js/modules/query_module.js'), 'utf8')
    .replace(/^const QueryModule =/m, 'var QueryModule =');
vm.runInContext(code, context);
const QM = context.QueryModule;

const jsonResponse = (body, status = 200) => ({
    ok: status >= 200 && status < 300,
    status,
    text: async () => (typeof body === 'string' ? body : JSON.stringify(body)),
    json: async () => body
});

// Text content of each <td> in the rendered table, row by row.
const renderedRows = () => {
    const html = getElement('query-browser-results').innerHTML;
    const body = (html.match(/<tbody>([\s\S]*)<\/tbody>/) || [, ''])[1];
    return [...body.matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(m => [...m[1].matchAll(/<td>([\s\S]*?)<\/td>/g)].map(c => c[1]));
};

const resetState = () => {
    Object.assign(QM.state, {
        query: '', dbName: null, columns: [], rows: null, sortColumn: null,
        sortDirection: 'asc', page: 1, pageSize: 25, error: null, loading: false
    });
    fetchCalls.length = 0;
    for (const id of Object.keys(elements)) delete elements[id];
};

// --- Pure helpers ---

test('QueryModule.toTable: union of keys, _id first, scalars in a value column', () => {
    const table = QM.toTable([{ name: 'a', _id: '1' }, { _id: '2', age: 3 }]);
    assert.deepStrictEqual([...table.columns], ['_id', 'name', 'age']);
    assert.deepStrictEqual(table.rows.map(r => [...r]), [['1', 'a', undefined], ['2', undefined, 3]]);

    const scalars = QM.toTable([1, 'x']);
    assert.deepStrictEqual([...scalars.columns], ['value']);
    assert.deepStrictEqual(scalars.rows.map(r => [...r]), [[1], ['x']]);

    assert.deepStrictEqual([...QM.toTable([]).columns], []);
    assert.deepStrictEqual([...QM.toTable({ a: 1 }).columns], ['a']);
});

test('QueryModule.sortRows: numeric, RDF numeric literals, text, empties last', () => {
    const int = n => `"${n}"^^<http://www.w3.org/2001/XMLSchema#integer>`;
    const rows = [[int(34)], [int(4)], [int(100)], [null]];
    assert.deepStrictEqual(QM.sortRows(rows, 0, 'asc').map(r => r[0]), [int(4), int(34), int(100), null]);
    assert.deepStrictEqual(QM.sortRows(rows, 0, 'desc').map(r => r[0]), [int(100), int(34), int(4), null]);

    const mixed = [['10'], [2], ['9.5'], ['banana'], ['Apple'], ['']];
    assert.deepStrictEqual(QM.sortRows(mixed, 0, 'asc').map(r => r[0]), [2, '9.5', '10', 'Apple', 'banana', '']);

    // Sorting is stable and does not mutate the input.
    const ties = [['b', 1], ['a', 2], ['b', 3], ['a', 4]];
    assert.deepStrictEqual(QM.sortRows(ties, 0, 'asc').map(r => r[1]), [2, 4, 1, 3]);
    assert.deepStrictEqual(ties.map(r => r[1]), [1, 2, 3, 4]);
    assert.deepStrictEqual(QM.sortRows(ties, null).map(r => r[1]), [1, 2, 3, 4]);
});

test('QueryModule.paginate: slices pages and clamps out-of-range pages', () => {
    const rows = Array.from({ length: 23 }, (_, i) => [i]);
    const first = QM.paginate(rows, 1, 10);
    assert.strictEqual(first.totalPages, 3);
    assert.deepStrictEqual(first.pageRows.map(r => r[0]), [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);

    const last = QM.paginate(rows, 3, 10);
    assert.deepStrictEqual(last.pageRows.map(r => r[0]), [20, 21, 22]);
    assert.strictEqual(last.start, 20);
    assert.strictEqual(last.end, 23);

    assert.strictEqual(QM.paginate(rows, 99, 10).page, 3);
    assert.strictEqual(QM.paginate(rows, 0, 10).page, 1);
    assert.strictEqual(QM.paginate([], 1, 10).totalPages, 1);
});

test('QueryModule.escapeHtml escapes markup in values', () => {
    assert.strictEqual(QM.escapeHtml('<http://ex.org/a> & "b"'), '&lt;http://ex.org/a&gt; &amp; &quot;b&quot;');
});

// --- Query execution and rendering ---

test('QueryModule.runQuery: posts the query and renders an escaped table', async () => {
    resetState();
    getElement('query-browser-input').value = 'PREFIX ex: <http://example.org/> SELECT ?s ?o WHERE { ?s ex:knows ?o }';
    getElement('query-browser-db').value = 'rdfdb';
    fetchHandler = async () => jsonResponse([
        { _id: '0', s: '<http://example.org/alice>', o: '<http://example.org/bob>' },
        { _id: '1', s: '<http://example.org/bob>', o: '<http://example.org/carol>' }
    ]);

    await QM.runQuery();

    assert.strictEqual(fetchCalls.length, 1);
    assert.strictEqual(fetchCalls[0].url, '/api/db/query');
    const body = JSON.parse(fetchCalls[0].options.body);
    assert.strictEqual(body.db_name, 'rdfdb');
    assert.ok(body.query.startsWith('PREFIX ex:'));
    assert.strictEqual(body.collection, undefined);

    const html = getElement('query-browser-results').innerHTML;
    assert.ok(html.includes('<table class="query-table">'));
    assert.ok(html.includes('Showing 1–2 of 2 rows'));
    assert.ok(html.includes('Page 1 of 1'));
    assert.ok(!html.includes('<http://example.org'), 'raw IRIs must be escaped');
    assert.deepStrictEqual(renderedRows(), [
        ['0', '&lt;http://example.org/alice&gt;', '&lt;http://example.org/bob&gt;'],
        ['1', '&lt;http://example.org/bob&gt;', '&lt;http://example.org/carol&gt;']
    ]);
});

test('QueryModule.setSort: clicking a header sorts, clicking again reverses', async () => {
    resetState();
    getElement('query-browser-input').value = 'SELECT * FROM people';
    fetchHandler = async () => jsonResponse([
        { _id: 'a', name: 'Carol', age: 41 },
        { _id: 'b', name: 'alice', age: 34 },
        { _id: 'c', name: 'Bob', age: 28 }
    ]);
    await QM.runQuery();

    QM.setSort(2); // age
    assert.deepStrictEqual(renderedRows().map(r => r[2]), ['28', '34', '41']);
    assert.ok(getElement('query-browser-results').innerHTML.includes('aria-sort="ascending"'));

    QM.setSort(2);
    assert.deepStrictEqual(renderedRows().map(r => r[2]), ['41', '34', '28']);
    assert.ok(getElement('query-browser-results').innerHTML.includes('aria-sort="descending"'));

    QM.setSort(1); // name, case-insensitive ascending
    assert.deepStrictEqual(renderedRows().map(r => r[1]), ['alice', 'Bob', 'Carol']);
    assert.ok(getElement('query-browser-results').innerHTML.includes('onclick="QueryModule.setSort(1)"'));
});

test('QueryModule pagination: page navigation and page size', async () => {
    resetState();
    getElement('query-browser-input').value = 'SELECT * FROM items';
    fetchHandler = async () => jsonResponse(Array.from({ length: 60 }, (_, i) => ({ _id: String(i), n: i })));
    await QM.runQuery();

    let html = getElement('query-browser-results').innerHTML;
    assert.strictEqual(renderedRows().length, 25);
    assert.ok(html.includes('Showing 1–25 of 60 rows'));
    assert.ok(html.includes('Page 1 of 3'));
    assert.ok(/onclick="QueryModule.goToPage\(0\)" disabled/.test(html), 'Prev is disabled on the first page');

    QM.goToPage(3);
    html = getElement('query-browser-results').innerHTML;
    assert.ok(html.includes('Showing 51–60 of 60 rows'));
    assert.deepStrictEqual(renderedRows().map(r => r[1]), Array.from({ length: 10 }, (_, i) => String(50 + i)));
    assert.ok(/onclick="QueryModule.goToPage\(4\)" disabled/.test(html), 'Next is disabled on the last page');

    // Sorting returns to the first page of the newly ordered rows.
    QM.setSort(1);
    QM.setSort(1);
    assert.ok(getElement('query-browser-results').innerHTML.includes('Page 1 of 3'));
    assert.strictEqual(renderedRows()[0][1], '59');

    QM.setPageSize('50');
    html = getElement('query-browser-results').innerHTML;
    assert.strictEqual(renderedRows().length, 50);
    assert.ok(html.includes('Page 1 of 2'));
    assert.ok(html.includes('<option value="50" selected>'));

    QM.setPageSize('7'); // not an offered size: ignored
    assert.strictEqual(QM.state.pageSize, 50);
});

test('QueryModule.runQuery: shows server errors and empty results', async () => {
    resetState();
    getElement('query-browser-input').value = 'PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s missing:p ?o }';
    // TissDB answers invalid SPARQL with 400 and a message giving the line and column.
    fetchHandler = async () => jsonResponse("SPARQL syntax error at line 1, column 54: undeclared prefix 'missing:'; declare it with PREFIX missing: <namespace-iri>", 400);
    await QM.runQuery();
    let html = getElement('query-browser-results').innerHTML;
    assert.ok(html.includes('class="query-error"'));
    assert.ok(html.includes('Query failed: SPARQL syntax error at line 1, column 54: undeclared prefix &#39;missing:&#39;'));
    assert.ok(!html.includes('<table'));

    fetchHandler = async () => jsonResponse([]);
    await QM.runQuery();
    html = getElement('query-browser-results').innerHTML;
    assert.ok(html.includes('Query returned no rows.'));

    getElement('query-browser-input').value = '   ';
    fetchCalls.length = 0;
    await QM.runQuery();
    assert.strictEqual(fetchCalls.length, 0);
    assert.ok(getElement('query-browser-results').innerHTML.includes('Enter a query to run.'));
});

test('QueryModule.onShow: populates databases and restores the last query', async () => {
    resetState();
    QM.state.query = 'SELECT * FROM knowledge';
    fetchHandler = async (url) => url === '/api/db/databases' ? jsonResponse(['main_db', 'rdfdb']) : jsonResponse([]);
    QM.onShow();
    await new Promise(r => setTimeout(r, 0));

    assert.strictEqual(getElement('query-browser-input').value, 'SELECT * FROM knowledge');
    const options = getElement('query-browser-db').innerHTML;
    assert.ok(options.includes('<option value="main_db" selected>'));
    assert.ok(options.includes('<option value="rdfdb">'));
    assert.ok(getElement('query-browser-results').innerHTML.includes('Results will appear here...'));
});

// --- N-Triples file import ---

const NT_FILE = '<http://example.org/a> <http://example.org/p> "x" .\n<http://example.org/a> <http://example.org/q> <http://example.org/b> .\n';
const importStatus = () => getElement('query-browser-import-status').innerHTML;

test('QueryModule.importNTriples: uploads the chosen file to the selected database', async () => {
    resetState();
    getElement('query-browser-db').value = 'rdfdb';
    getElement('query-browser-file').files = [{ name: 'people.nt', text: async () => NT_FILE }];
    getElement('query-browser-file').value = 'C:\\fakepath\\people.nt';
    fetchHandler = async () => jsonResponse({ loaded: 2 });

    await QM.importNTriples();

    assert.strictEqual(fetchCalls.length, 1);
    assert.strictEqual(fetchCalls[0].url, '/api/db/ntriples');
    assert.strictEqual(fetchCalls[0].options.method, 'POST');
    const body = JSON.parse(fetchCalls[0].options.body);
    assert.strictEqual(body.db_name, 'rdfdb');
    assert.strictEqual(body.data, NT_FILE);
    assert.ok(importStatus().includes('class="success"'));
    assert.ok(importStatus().includes('Imported people.nt into rdfdb: 2 new triples added'));
    assert.strictEqual(getElement('query-browser-file').value, '', 'file input is cleared after a successful import');

    fetchHandler = async () => jsonResponse({ loaded: 1 });
    getElement('query-browser-file').files = [{ name: 'one.nt', text: async () => NT_FILE }];
    await QM.importNTriples();
    assert.ok(importStatus().includes('1 new triple added'));
});

test('QueryModule.importNTriples: reports a missing file, parse errors and oversized files', async () => {
    resetState();
    getElement('query-browser-file').files = [];
    await QM.importNTriples();
    assert.strictEqual(fetchCalls.length, 0);
    assert.ok(importStatus().includes('Choose an N-Triples (.nt) file to import.'));
    assert.ok(importStatus().includes('class="error"'));

    // TissDB answers malformed input with a plain-text 400 that names the line.
    getElement('query-browser-file').files = [{ name: 'bad.nt', text: async () => '<a> <b> oops .\n' }];
    getElement('query-browser-file').value = 'bad.nt';
    fetchHandler = async () => jsonResponse('N-Triples parse error on line 1: expected an IRI, blank node or literal (column 9)', 400);
    await QM.importNTriples();
    assert.ok(importStatus().includes('Import failed: N-Triples parse error on line 1'));
    assert.strictEqual(getElement('query-browser-file').value, 'bad.nt', 'file stays selected after a failure');

    // Bodies over the platform's 10 MB limit are rejected before uploading.
    fetchCalls.length = 0;
    const big = '<http://example.org/s> <http://example.org/p> "' + 'x'.repeat(200) + '" .\n';
    getElement('query-browser-file').files = [{ name: 'huge.nt', text: async () => big.repeat(Math.ceil(QM.MAX_UPLOAD_BYTES / big.length) + 1) }];
    await QM.importNTriples();
    assert.strictEqual(fetchCalls.length, 0);
    assert.ok(importStatus().includes('too large to upload'));
    assert.ok(importStatus().includes('tissdb --import-ntriples'));
});

test('QueryModule.importNTriples: falls back to FileReader and escapes file names', async () => {
    resetState();
    QM.state.dbName = 'main_db';
    getElement('query-browser-file').files = [{ name: '<script>.nt', contents: NT_FILE }];
    fetchHandler = async () => jsonResponse({ loaded: 2 });
    await QM.importNTriples();
    assert.strictEqual(JSON.parse(fetchCalls[0].options.body).data, NT_FILE);
    assert.strictEqual(JSON.parse(fetchCalls[0].options.body).db_name, 'main_db');
    assert.ok(importStatus().includes('&lt;script&gt;.nt'));
    assert.ok(!importStatus().includes('<script>'));
});
