/**
 * Query Browser: runs TissQL or SPARQL queries against a TissDB database and shows
 * the results as a sortable, paginated table.
 */
const QueryModule = {
    PAGE_SIZES: [10, 25, 50, 100],
    // The platform server rejects request bodies over 10 MB.
    MAX_UPLOAD_BYTES: 10 * 1024 * 1024,

    state: {
        query: 'SELECT * FROM knowledge',
        dbName: null,
        columns: [],
        rows: null,         // null until a query has been run
        sortColumn: null,   // index into columns, or null for result order
        sortDirection: 'asc',
        page: 1,
        pageSize: 25,
        error: null,
        loading: false
    },

    // --- Pure helpers ---

    escapeHtml(value) {
        return String(value)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#39;');
    },

    formatCell(value) {
        if (value === null || value === undefined) return '';
        if (typeof value === 'object') return JSON.stringify(value);
        return String(value);
    },

    /**
     * Converts a query response into table form. TissDB returns an array of
     * documents; columns are the union of their keys in first-seen order, with
     * "_id" first when present.
     */
    toTable(data) {
        const docs = Array.isArray(data) ? data : (data === null || data === undefined ? [] : [data]);
        const isDoc = d => d !== null && typeof d === 'object' && !Array.isArray(d);
        const columns = [];
        const seen = new Set();
        const addColumn = key => {
            if (!seen.has(key)) {
                seen.add(key);
                columns.push(key);
            }
        };
        docs.forEach(doc => {
            if (isDoc(doc)) Object.keys(doc).forEach(addColumn);
            else addColumn('value'); // scalars and arrays are shown in a "value" column
        });
        const idIndex = columns.indexOf('_id');
        if (idIndex > 0) columns.unshift(columns.splice(idIndex, 1)[0]);

        const rows = docs.map(doc => columns.map(col => isDoc(doc) ? doc[col] : (col === 'value' ? doc : undefined)));
        return { columns, rows };
    },

    /**
     * Sort key for a cell. Numbers, numeric strings and numeric RDF literals in
     * N-Triples form (e.g. "34"^^<http://www.w3.org/2001/XMLSchema#integer>) sort
     * numerically; everything else sorts as text.
     */
    sortKey(value) {
        if (typeof value === 'number') return value;
        if (typeof value === 'boolean') return value ? 1 : 0;
        const text = this.formatCell(value);
        if (/^\s*-?\d+(\.\d+)?([eE][+-]?\d+)?\s*$/.test(text)) return parseFloat(text);
        const literal = text.match(/^"([^"]*)"\^\^<http:\/\/www\.w3\.org\/2001\/XMLSchema#(\w+)>$/);
        if (literal && /^(integer|decimal|double|float|long|int|short|byte|(non)?(Positive|Negative)Integer|nonNegativeInteger|unsigned\w+)$/.test(literal[2])) {
            const n = parseFloat(literal[1]);
            if (!Number.isNaN(n)) return n;
        }
        return text;
    },

    /** Compares two cells; empty cells always sort last regardless of direction. */
    compareCells(a, b, direction = 'asc') {
        const aEmpty = a === null || a === undefined || a === '';
        const bEmpty = b === null || b === undefined || b === '';
        if (aEmpty || bEmpty) return aEmpty === bEmpty ? 0 : (aEmpty ? 1 : -1);

        const ka = this.sortKey(a);
        const kb = this.sortKey(b);
        let cmp;
        if (typeof ka === 'number' && typeof kb === 'number') {
            cmp = ka - kb;
        } else if (typeof ka === 'number' || typeof kb === 'number') {
            cmp = typeof ka === 'number' ? -1 : 1; // numbers before text
        } else {
            cmp = ka.localeCompare(kb, undefined, { numeric: true, sensitivity: 'base' });
        }
        return direction === 'desc' ? -cmp : cmp;
    },

    /** Returns a sorted copy of rows (stable); rows are arrays indexed by column. */
    sortRows(rows, columnIndex, direction = 'asc') {
        if (columnIndex === null || columnIndex === undefined) return rows.slice();
        return rows
            .map((row, i) => ({ row, i }))
            .sort((x, y) => this.compareCells(x.row[columnIndex], y.row[columnIndex], direction) || x.i - y.i)
            .map(entry => entry.row);
    },

    /** Slices rows for a 1-based page, clamping the page into range. */
    paginate(rows, page, pageSize) {
        const totalPages = Math.max(1, Math.ceil(rows.length / pageSize));
        const current = Math.min(Math.max(1, page), totalPages);
        const start = (current - 1) * pageSize;
        const end = Math.min(start + pageSize, rows.length);
        return { pageRows: rows.slice(start, end), page: current, totalPages, start, end, total: rows.length };
    },

    // --- Rendering ---

    renderResults() {
        const el = document.getElementById('query-browser-results');
        if (!el) return;
        const s = this.state;

        if (s.loading) {
            el.innerHTML = '<div class="query-status">Running query...</div>';
            return;
        }
        if (s.error) {
            el.innerHTML = `<div class="query-error">${this.escapeHtml(s.error)}</div>`;
            return;
        }
        if (s.columns.length === 0) {
            el.innerHTML = `<div class="query-status">${s.rows === null ? 'Results will appear here...' : 'Query returned no rows.'}</div>`;
            return;
        }

        const sorted = this.sortRows(s.rows, s.sortColumn, s.sortDirection);
        const view = this.paginate(sorted, s.page, s.pageSize);
        s.page = view.page;

        const header = s.columns.map((col, i) => {
            const active = s.sortColumn === i;
            const ariaSort = active ? (s.sortDirection === 'asc' ? 'ascending' : 'descending') : 'none';
            const arrow = active ? (s.sortDirection === 'asc' ? '▲' : '▼') : '';
            return `<th class="sortable${active ? ' sorted' : ''}" aria-sort="${ariaSort}" onclick="QueryModule.setSort(${i})" title="Sort by ${this.escapeHtml(col)}">` +
                `${this.escapeHtml(col)}<span class="sort-indicator">${arrow}</span></th>`;
        }).join('');

        const body = view.pageRows.map(row =>
            `<tr>${row.map(cell => `<td>${this.escapeHtml(this.formatCell(cell))}</td>`).join('')}</tr>`
        ).join('');

        const sizeOptions = this.PAGE_SIZES.map(n =>
            `<option value="${n}"${n === s.pageSize ? ' selected' : ''}>${n}</option>`).join('');

        el.innerHTML = `
            <div class="query-summary">Showing ${view.start + 1}–${view.end} of ${view.total} row${view.total === 1 ? '' : 's'}</div>
            <div class="query-table-wrapper">
                <table class="query-table">
                    <thead><tr>${header}</tr></thead>
                    <tbody>${body}</tbody>
                </table>
            </div>
            <div class="pagination">
                <button class="btn btn-secondary" onclick="QueryModule.goToPage(1)" ${view.page === 1 ? 'disabled' : ''}>« First</button>
                <button class="btn btn-secondary" onclick="QueryModule.goToPage(${view.page - 1})" ${view.page === 1 ? 'disabled' : ''}>‹ Prev</button>
                <span class="page-indicator">Page ${view.page} of ${view.totalPages}</span>
                <button class="btn btn-secondary" onclick="QueryModule.goToPage(${view.page + 1})" ${view.page === view.totalPages ? 'disabled' : ''}>Next ›</button>
                <button class="btn btn-secondary" onclick="QueryModule.goToPage(${view.totalPages})" ${view.page === view.totalPages ? 'disabled' : ''}>Last »</button>
                <label class="page-size">Rows per page
                    <select id="query-browser-page-size" onchange="QueryModule.setPageSize(this.value)">${sizeOptions}</select>
                </label>
            </div>
        `;
    },

    // --- Actions ---

    setSort(columnIndex) {
        if (this.state.sortColumn === columnIndex) {
            this.state.sortDirection = this.state.sortDirection === 'asc' ? 'desc' : 'asc';
        } else {
            this.state.sortColumn = columnIndex;
            this.state.sortDirection = 'asc';
        }
        this.state.page = 1;
        this.renderResults();
    },

    goToPage(page) {
        this.state.page = page;
        this.renderResults();
    },

    setPageSize(size) {
        const n = parseInt(size, 10);
        if (!this.PAGE_SIZES.includes(n)) return;
        this.state.pageSize = n;
        this.state.page = 1;
        this.renderResults();
    },

    handleKeydown(event) {
        // Ctrl+Enter / Cmd+Enter runs the query.
        if (event.key === 'Enter' && (event.ctrlKey || event.metaKey)) {
            event.preventDefault();
            this.runQuery();
        }
    },

    async runQuery() {
        const input = document.getElementById('query-browser-input');
        const dbSelect = document.getElementById('query-browser-db');
        const query = input ? input.value.trim() : '';
        const s = this.state;
        s.query = input ? input.value : s.query;
        if (dbSelect && dbSelect.value) s.dbName = dbSelect.value;

        if (!query) {
            s.error = 'Enter a query to run.';
            this.renderResults();
            return;
        }

        s.loading = true;
        s.error = null;
        this.renderResults();
        try {
            const res = await fetch('/api/db/query', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ db_name: s.dbName || AppState.db.selected, query })
            });
            const text = await res.text();
            let data;
            try {
                data = text ? JSON.parse(text) : [];
            } catch (e) {
                data = null;
            }
            if (!res.ok || data === null || (data && !Array.isArray(data) && data.error)) {
                const message = (data && data.error) ? data.error : (text || `HTTP ${res.status}`);
                throw new Error(message);
            }
            const table = this.toTable(data);
            s.columns = table.columns;
            s.rows = table.rows;
            s.sortColumn = null;
            s.sortDirection = 'asc';
            s.page = 1;
        } catch (e) {
            s.columns = [];
            s.rows = [];
            s.error = 'Query failed: ' + e.message;
        } finally {
            s.loading = false;
        }
        this.renderResults();
    },

    // --- N-Triples import ---

    setImportStatus(message, kind) {
        const el = document.getElementById('query-browser-import-status');
        if (el) el.innerHTML = message ? `<span class="${kind}">${this.escapeHtml(message)}</span>` : '';
    },

    readFile(file) {
        if (typeof file.text === 'function') return file.text();
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = event => resolve(event.target.result);
            reader.onerror = () => reject(reader.error || new Error('could not read file'));
            reader.readAsText(file);
        });
    },

    /** Uploads the chosen N-Triples file into the selected database's RDF triples. */
    async importNTriples() {
        const input = document.getElementById('query-browser-file');
        const file = input && input.files && input.files[0];
        if (!file) {
            this.setImportStatus('Choose an N-Triples (.nt) file to import.', 'error');
            return;
        }
        const dbSelect = document.getElementById('query-browser-db');
        const dbName = (dbSelect && dbSelect.value) || this.state.dbName || AppState.db.selected;

        this.setImportStatus(`Importing ${file.name}...`, 'pending');
        try {
            const body = JSON.stringify({ db_name: dbName, data: await this.readFile(file) });
            if (new TextEncoder().encode(body).length > this.MAX_UPLOAD_BYTES) {
                throw new Error('file is too large to upload (10 MB limit); load it with "tissdb --import-ntriples <file>" instead');
            }
            const res = await fetch('/api/db/ntriples', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body
            });
            const text = await res.text();
            let data = null;
            try {
                data = JSON.parse(text);
            } catch (e) {
                // Errors from TissDB are plain text.
            }
            if (!res.ok || !data || typeof data.loaded !== 'number') {
                throw new Error((data && data.error) || text || `HTTP ${res.status}`);
            }
            this.setImportStatus(
                `Imported ${file.name} into ${dbName}: ${data.loaded} new triple${data.loaded === 1 ? '' : 's'} added (triples already present are skipped).`,
                'success');
            if (input) input.value = '';
        } catch (e) {
            this.setImportStatus(`Import failed: ${e.message}`, 'error');
        }
    },

    async loadDatabases() {
        const select = document.getElementById('query-browser-db');
        if (!select) return;
        const current = this.state.dbName || AppState.db.selected;
        try {
            const res = await fetch('/api/db/databases');
            const dbs = await res.json();
            if (!Array.isArray(dbs)) throw new Error('unexpected response');
            const names = dbs.includes(current) ? dbs : [current, ...dbs];
            select.innerHTML = names.map(db =>
                `<option value="${this.escapeHtml(db)}"${db === current ? ' selected' : ''}>${this.escapeHtml(db)}</option>`).join('');
        } catch (e) {
            select.innerHTML = `<option value="${this.escapeHtml(current)}" selected>${this.escapeHtml(current)}</option>`;
        }
        this.state.dbName = select.value || current;
    },

    /** Called when the tab is shown: restores the last query and results. */
    onShow() {
        const input = document.getElementById('query-browser-input');
        if (input) input.value = this.state.query;
        this.loadDatabases();
        this.renderResults();
    }
};


window.QueryModule = QueryModule;
