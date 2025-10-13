import { TissDBLite } from './index.js';
import * as readline from 'readline';

/**
 * A simple parser for TissQL SELECT queries.
 * Extracts collection and the WHERE clause.
 * Example: "SELECT * FROM users WHERE age > 30"
 * @param {string} query - The TissQL query string.
 * @returns {{collection: string, whereClause: string}}
 */
function parseTissQL(query) {
    const fromMatch = query.match(/FROM\s+([a-zA-Z0-9_]+)/i);
    const whereMatch = query.match(/WHERE\s+(.*)/i);

    if (!fromMatch) {
        throw new Error('Invalid TissQL query: Missing FROM clause.');
    }

    const collection = fromMatch[1];
    const whereClause = whereMatch ? whereMatch[1].trim() : '';

    return { collection, whereClause };
}


/**
 * Main function to set up the DB, listen for a query, execute it, and print the result.
 */
async function main() {
    const db = new TissDBLite();

    // 1. Create and populate a sample collection
    try {
        db.createCollection('users');
        db.bulkInsert('users', [
            { name: 'Alice', age: 30, role: 'developer' },
            { name: 'Bob', age: 40, role: 'admin' },
            { name: 'Charlie', age: 30, status: 'active' },
            { name: 'David', age: 25, status: 'inactive' },
            { name: 'Eve', age: 35, role: 'developer', status: 'active' }
        ]);
    } catch (e) {
        // This should not happen in this controlled script
        console.error(JSON.stringify({ error: `Database setup failed: ${e.message}` }));
        return;
    }

    // 2. Read the TissQL query from stdin
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        terminal: false
    });

    rl.on('line', (line) => {
        const tissqlQuery = line.trim();
        if (tissqlQuery) {
            try {
                // 3. Parse the query
                const { collection, whereClause } = parseTissQL(tissqlQuery);

                // 4. Execute the query using db.find()
                const results = db.find(collection, whereClause);

                // 5. Print results to stdout as a JSON string
                console.log(JSON.stringify(results, null, 2));

            } catch (e) {
                console.error(JSON.stringify({ error: e.message }));
            } finally {
                rl.close();
            }
        }
    });
}

main();
