function createCandlestickChart(data, width = 600, height = 300) {
    const padding = 40;
    const chartWidth = width - 2 * padding;
    const chartHeight = height - 2 * padding;

    const allPrices = data.flatMap(d => [d.open, d.high, d.low, d.close]);
    const minPrice = Math.min(...allPrices);
    const maxPrice = Math.max(...allPrices);
    const priceRange = maxPrice - minPrice;

    const candleWidth = chartWidth / data.length - 10;

    let svg = `<svg width="${width}" height="${height}" viewBox="0 0 ${width} ${height}" xmlns="http://www.w3.org/2000/svg">`;

    // Grid/Axes
    svg += `<line x1="${padding}" y1="${height-padding}" x2="${width-padding}" y2="${height-padding}" stroke="#ccc" />`;

    data.forEach((d, i) => {
        const x = padding + 10 + i * (candleWidth + 10);

        const scale = (p) => height - padding - ((p - minPrice) / priceRange) * chartHeight;

        const yHigh = scale(d.high);
        const yLow = scale(d.low);
        const yOpen = scale(d.open);
        const yClose = scale(d.close);

        const isUp = d.close >= d.open;
        const color = isUp ? '#22c55e' : '#ef4444';

        // Wick
        svg += `<line x1="${x + candleWidth/2}" y1="${yHigh}" x2="${x + candleWidth/2}" y2="${yLow}" stroke="${color}" />`;

        // Body
        const bodyTop = Math.min(yOpen, yClose);
        const bodyHeight = Math.abs(yOpen - yClose) || 1;
        svg += `<rect x="${x}" y="${bodyTop}" width="${candleWidth}" height="${bodyHeight}" fill="${color}" />`;
    });

    svg += '</svg>';
    return svg;
}

window.createCandlestickChart = createCandlestickChart;
