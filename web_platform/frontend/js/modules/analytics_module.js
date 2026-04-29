const AnalyticsModule = {
    async loadAnalyticsFromServer() {
        const res = await fetch('/api/analytics/chart', { method: 'POST' });
        const data = await res.json();
        if (window.createBarChart) {
             const container = document.getElementById('chart-container');
             if (container) container.innerHTML = window.createBarChart(data.data);
        }
    },

    async runTradingEngine() {
        const symbolInput = document.getElementById('symbol-input');
        if (!symbolInput) return;
        const symbol = symbolInput.value;
        const resultsEl = document.getElementById('trading-results');
        resultsEl.innerText = 'Analyzing...';
        try {
            const res = await fetch('/api/analytics/trade', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ symbol })
            });
            const data = await res.json();
            resultsEl.innerHTML = `
                <div class="card" style="margin-top: 1rem; border-top: 3px solid var(--accent-color);">
                    <div style="display: flex; gap: 2rem; align-items: center; justify-content: space-between;">
                        <div>
                            <p style="font-size: 1.1rem;">Symbol: <strong>${data.symbol}</strong></p>
                            <p>Current Price: <span style="font-family: monospace;">$${data.market_data.price}</span></p>
                            <p style="margin-top: 0.5rem;">Recommended Signal:</p>
                            <span class="status-badge" style="background: ${data.signal === 'BUY' ? '#dcfce7' : '#fee2e2'}; color: ${data.signal === 'BUY' ? '#166534' : '#991b1b'}; font-size: 1.2rem;">
                                ${data.signal}
                            </span>
                        </div>
                        <div id="candlestick-container" style="flex: 1; min-width: 400px;"></div>
                    </div>
                </div>
            `;
            if (window.createCandlestickChart) {
                const candleContainer = document.getElementById('candlestick-container');
                if (candleContainer) candleContainer.innerHTML = window.createCandlestickChart(data.historical_data, 450, 250);
            }
        } catch (e) { resultsEl.innerText = 'Error: ' + e.message; }
    }
};

window.AnalyticsModule = AnalyticsModule;
