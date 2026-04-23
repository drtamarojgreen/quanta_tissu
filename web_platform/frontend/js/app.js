// Main entry point for the QuantaTissu Platform
// Loads modules and initializes the UI

document.addEventListener('DOMContentLoaded', () => {
    // Initial View
    if (window.switchTab) {
        window.switchTab('dashboard');
    }
});

// Global Config Utilities
window.saveConfiguration = async () => {
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
};

window.exportConfiguration = async () => {
    const res = await fetch('/api/config');
    const data = await res.json();
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'model_config.json';
    a.click();
};

window.importConfiguration = async () => {
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
            if (UIModule.state.activeTab === 'config') AdminModule.loadConfig();
        };
        reader.readAsText(file);
    };
    input.click();
};

window.runTradingEngine = () => {
    if (window.AnalyticsModule && window.AnalyticsModule.runTradingEngine) {
        window.AnalyticsModule.runTradingEngine();
    }
};

// For testing purposes, trigger initialization manually if needed
if (document._onDOMContentLoaded) {
    document._onDOMContentLoaded();
}
