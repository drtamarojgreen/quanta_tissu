const AnalyticsModule = {
    async loadAnalyticsFromServer() {
        const res = await fetch('/api/analytics/chart', { method: 'POST' });
        const data = await res.json();
        if (window.createBarChart) document.getElementById('chart-container').innerHTML = window.createBarChart(data.data);
    },

    async runTradingEngine() {
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
};

window.AnalyticsModule = AnalyticsModule;
