async function initCanvas() {
    const canvas = document.getElementById('nexus-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    let nodes = [];
    let links = [];

    try {
        const res = await fetch('/api/nexus/graph', { method: 'POST' });
        const data = await res.json();
        nodes = data.nodes;
        links = data.links;
    } catch (e) {
        console.error('Failed to fetch graph', e);
    }

    function draw() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Draw links
        ctx.strokeStyle = '#475569';
        ctx.lineWidth = 2;
        links.forEach(link => {
            const s = nodes.find(n => n.id === link.source);
            const t = nodes.find(n => n.id === link.target);
            if (s && t) {
                ctx.beginPath();
                ctx.moveTo(s.x, s.y);
                ctx.lineTo(t.x, t.y);
                ctx.stroke();
            }
        });

        // Draw nodes
        nodes.forEach(node => {
            ctx.beginPath();
            ctx.arc(node.x, node.y, 30, 0, Math.PI * 2);
            ctx.fillStyle = '#3b82f6';
            ctx.fill();
            ctx.strokeStyle = 'white';
            ctx.lineWidth = 3;
            ctx.stroke();

            ctx.fillStyle = 'white';
            ctx.font = 'bold 14px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText(node.label, node.x, node.y + 5);
        });
    }

    draw();
}

window.initCanvas = initCanvas;
