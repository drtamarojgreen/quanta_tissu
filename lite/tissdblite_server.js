const http = require('http');
const url = require('url');
const { TissDBLite } = require('./index.js'); // Assuming index.js is in the same directory

const PORT = 9877;
const db = new TissDBLite(); // Instantiate TissDBLite once

const server = http.createServer((req, res) => {
    const parsedUrl = url.parse(req.url, true);
    const path = parsedUrl.pathname;
    const method = req.method;

    res.setHeader('Content-Type', 'application/json');

    let body = '';
    req.on('data', chunk => {
        body += chunk.toString();
    });

    req.on('end', () => {
        let command;
        try {
            command = body ? JSON.parse(body) : {};
        } catch (e) {
            res.statusCode = 400;
            res.end(JSON.stringify({ status: 'error', message: 'Invalid JSON body.' }));
            return;
        }

        // Basic routing based on path and method
        if (path === '/command' && method === 'POST') {
            let result;
            try {
                switch (command.action) {
                    case 'createCollection':
                        db.createCollection(command.collectionName);
                        result = { status: 'success', message: 'Collection created.' };
                        break;
                    case 'insert':
                        // Ensure collection exists before inserting
                        if (!db.collections[command.collectionName]) {
                            db.createCollection(command.collectionName);
                        }
                        result = { status: 'success', data: db.insert(command.collectionName, command.item) };
                        break;
                    case 'find':
                        result = { status: 'success', data: db.find(command.collectionName, command.condition_string) };
                        break;
                    case 'update':
                        result = { status: 'success', data: db.update(command.collectionName, command.item) };
                        break;
                    case 'remove':
                        db.remove(command.collectionName, command.itemId);
                        result = { status: 'success', message: 'Item removed.' };
                        break;
                    case 'bulkInsert':
                        result = { status: 'success', data: db.bulkInsert(command.collectionName, command.items) };
                        break;
                    case 'exportCollection':
                        result = { status: 'success', data: db.exportCollection(command.collectionName) };
                        break;
                    case 'deleteDb':
                        db.collections = {};
                        result = { status: 'success', message: 'In-memory DB cleared.' };
                        break;
                    default:
                        throw new Error(`Unknown action: ${command.action}`);
                }
                res.statusCode = 200;
                res.end(JSON.stringify(result));
            } catch (e) {
                res.statusCode = 500;
                res.end(JSON.stringify({ status: 'error', message: e.message }));
            }
        } else {
            res.statusCode = 404;
            res.end(JSON.stringify({ status: 'error', message: 'Not Found' }));
        }
    });
});

server.listen(PORT, () => {
    console.log(`TissDBLite server running on port ${PORT}`);
});

// Handle graceful shutdown
process.on('SIGINT', () => {
    console.log('TissDBLite server shutting down...');
    server.close(() => {
        console.log('TissDBLite server closed.');
        process.exit(0);
    });
});
