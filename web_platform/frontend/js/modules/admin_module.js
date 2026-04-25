const AdminModule = {
    async loadConfig() {
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
        this.checkTrainingStatus();
    },

    async saveConfig() {
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
    },

    async startTraining() {
        UIModule.openModal('modal-confirm-process', {
            command: 'python3 quanta_tissu/tisslm/legacylm/train.py (simulated)',
            callback: 'AdminModule.executeStartTraining'
        });
    },

    async executeStartTraining() {
        const res = await fetch('/api/training/start', { method: 'POST' });
        if (res.ok) this.checkTrainingStatus();
        else { const err = await res.json(); alert(err.error); }
    },

    trainingInterval: null,
    async checkTrainingStatus() {
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
                if (!this.trainingInterval) this.trainingInterval = setInterval(() => this.checkTrainingStatus(), 1000);
            } else {
                statusText.innerText = data.progress === 100 ? 'Completed' : 'Idle';
                progressContainer.style.display = data.progress === 100 ? 'block' : 'none';
                progressFill.style.width = data.progress + '%';
                startBtn.disabled = false;
                if (this.trainingInterval) { clearInterval(this.trainingInterval); this.trainingInterval = null; }
            }
        }
    },

    async loadAdminCollections() {
        const res = await fetch('/api/db/collections', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ db_name: UIModule.state.dbName })
        });
        const colls = await res.json();
        const select = document.getElementById('migrate-source-select');
        if (select) {
            select.innerHTML = colls.map(c => `<option value="${c}">${c}</option>`).join('');
        }
    },

    async runMigration() {
        const source = document.getElementById('migrate-source-select').value;
        const target = document.getElementById('migrate-target-input').value;
        UIModule.openModal('modal-confirm-process', {
            command: `Migrate TissDB documents from [${source}] to [${target}]`,
            callback: 'AdminModule.executeRunMigration'
        });
    },

    async executeRunMigration() {
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
};

window.AdminModule = AdminModule;
