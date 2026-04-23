const DBModule = {
    async loadStats() {
        try {
            const res = await fetch('/api/db/stats', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ db_name: UIModule.state.dbName })
            });
            const data = await res.json();
            const el = document.getElementById('db-stats-val');
            if (el) el.innerHTML = `<p>Total Docs: ${data.total_docs || 0}</p><p>Feedback: ${data.feedback_entries || 0}</p>`;
        } catch(e) {}
    },

    async loadDatabases() {
        try {
            const res = await fetch('/api/db/databases');
            const dbs = await res.json();
            const select = document.getElementById('db-select');
            if (select) {
                select.innerHTML = dbs.map(db => `<option value="${db}" ${db === UIModule.state.dbName ? 'selected' : ''}>${db}</option>`).join('');
                await this.loadCollections();
            }
        } catch(e) {}
    },

    async loadCollections() {
        const dbSelect = document.getElementById('db-select');
        if (!dbSelect) return;
        try {
            const res = await fetch('/api/db/collections', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ db_name: dbSelect.value })
            });
            const colls = await res.json();
            const collSelect = document.getElementById('coll-select');
            if (collSelect) collSelect.innerHTML = colls.map(c => `<option value="${c}">${c}</option>`).join('');
        } catch(e) {}
    },

    async runQuery() {
        const query = document.getElementById('query-input').value;
        const coll = document.getElementById('coll-select').value;
        const res = await fetch('/api/db/query', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ collection: coll, query })
        });
        const data = await res.json();
        document.getElementById('query-results').innerText = JSON.stringify(data, null, 2);
    },

    async loadCollectionsToModal() {
        const res = await fetch('/api/db/collections', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ db_name: UIModule.state.dbName })
        });
        const colls = await res.json();
        const select = document.getElementById('modal-del-coll-select');
        if (select) {
            select.innerHTML = colls.map(c => `<option value="${c}">${c}</option>`).join('');
        }
    },

    confirmDeleteModal() {
        const coll = document.getElementById('modal-del-coll-select').value;
        UIModule.openModal('modal-confirm-delete', { coll });
    },

    async executeDelete(coll) {
        await fetch('/api/db/collection', {
            method: 'DELETE',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ db_name: UIModule.state.dbName, collection: coll })
        });
        alert('Deleted!');
        UIModule.closeModals();
    },

    async createCollectionModal() {
        const coll = document.getElementById('modal-new-coll').value;
        await fetch('/api/db/collection', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ db_name: UIModule.state.dbName, collection: coll })
        });
        alert('Created!');
        UIModule.closeModals();
    }
};

window.DBModule = DBModule;
