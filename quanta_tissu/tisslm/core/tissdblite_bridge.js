const { TissDBLite } = require('../../../lite/index.js'); // Adjust path as needed
const db = new TissDBLite();

process.stdin.on('data', (data) => {
    try {
        const command = JSON.parse(data.toString());
        let result;
        switch (command.action) {
            case 'createCollection':
                db.createCollection(command.collectionName);
                result = { status: 'success', message: 'Collection created.' };
                break;
            case 'insert':
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
            case 'deleteDb': // For cleanup, though TissDBLite is in-memory
                db.collections = {};
                result = { status: 'success', message: 'In-memory DB cleared.' };
                break;
            default:
                throw new Error(`Unknown action: ${command.action}`);
        }
        process.stdout.write(JSON.stringify(result) + '\n').flush();
    } catch (e) {
        process.stderr.write(JSON.stringify({ status: 'error', message: e.message }) + '\n');
    }
});

process.stdin.on('end', () => {
    // Optional: perform cleanup or final logging
});