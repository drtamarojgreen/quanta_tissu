function createBarChart(data, width = 600, height = 300) {
    const padding = 40;
    const chartWidth = width - 2 * padding;
    const chartHeight = height - 2 * padding;
    const maxValue = Math.max(...data);
    const barWidth = chartWidth / data.length - 10;

    let svg = `<svg width="${width}" height="${height}" viewBox="0 0 ${width} ${height}" xmlns="http://www.w3.org/2000/svg">`;

    // Axes
    svg += `<line x1="${padding}" y1="${height-padding}" x2="${width-padding}" y2="${height-padding}" stroke="black" />`;
    svg += `<line x1="${padding}" y1="${padding}" x2="${padding}" y2="${height-padding}" stroke="black" />`;

    // Bars
    data.forEach((val, i) => {
        const h = (val / maxValue) * chartHeight;
        const x = padding + 10 + i * (barWidth + 10);
        const y = height - padding - h;
        svg += `<rect x="${x}" y="${y}" width="${barWidth}" height="${h}" fill="#3b82f6" />`;
        svg += `<text x="${x + barWidth/2}" y="${height - padding + 20}" font-size="12" text-anchor="middle">${i+1}</text>`;
        svg += `<text x="${x + barWidth/2}" y="${y - 5}" font-size="10" text-anchor="middle">${val}</text>`;
    });

    svg += '</svg>';
    return svg;
}

window.loadAnalytics = () => {
    const data = [45, 80, 55, 90, 30, 70];
    const container = document.getElementById('chart-container');
    container.innerHTML = createBarChart(data);
};
