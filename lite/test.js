import { TissDBLite } from './index.js';
import http from 'http'; // Using Node.js built-in http module

// --- TissDB Configuration (for actual server) --- //
const TISSDB_HOST = process.env.TISSDB_HOST || 'localhost';
const TISSDB_PORT = process.env.TISSDB_PORT || 9876;
const TISSDB_AUTH_TOKEN = "static_test_token"; // Replicating BDD test authorization
const TISSDB_DB_NAME = "testdb"; // Database name used in BDD tests
const TISSDB_BASE_URL = `http://${TISSDB_HOST}:${TISSDB_PORT}`;

// --- Simple HTTP Client for TissDB Interaction --- //
const tissdbHttpClient = {
    request: (method, path, data = null) => {
        return new Promise((resolve, reject) => {
            const headers = {
                'Content-Type': 'application/json',
            };

            if (TISSDB_AUTH_TOKEN) {
                headers['Authorization'] = `Bearer ${TISSDB_AUTH_TOKEN}`;
            }

            let requestBody = null;
            if (data) {
                requestBody = JSON.stringify(data);
                headers['Content-Length'] = Buffer.byteLength(requestBody);
            }

            const options = {
                hostname: TISSDB_HOST,
                port: TISSDB_PORT,
                path: path,
                method: method,
                headers: headers,
            };

            const req = http.request(options, (res) => {
                let responseData = '';
                res.on('data', (chunk) => {
                    responseData += chunk;
                });
                res.on('end', () => {
                    let parsedData = null;
                    if (responseData) {
                        try {
                            parsedData = JSON.parse(responseData);
                        } catch (e) {
                            // If not JSON, keep as raw string or null if empty
                            parsedData = responseData;
                        }
                    }

                    // Special handling for query results (expect array)
                    if (path.endsWith('/_query')) {
                        if (Array.isArray(parsedData)) {
                            // If it's already an array, use it directly
                            parsedData = parsedData;
                        } else if (typeof parsedData === 'object' && parsedData !== null && (parsedData.items || parsedData.data)) {
                            // If it's an object with 'items' or 'data' property, use that
                            parsedData = parsedData.items || parsedData.data;
                        } else {
                            // Otherwise, assume empty array for query results
                            parsedData = [];
                        }
                    } else if (method === 'DELETE' && (parsedData === null || parsedData === '')) {
                        parsedData = null; // Indicate successful deletion with no content
                    }

                    resolve({ statusCode: res.statusCode, data: parsedData });
                });
            });

            req.on('error', (e) => {
                console.error(`TissDB Request Error: ${method} ${path}`, e);
                reject(e);
            });

            if (requestBody) {
                req.write(requestBody);
            }
            req.end();
        });
    },

    post: (path, data) => tissdbHttpClient.request('POST', path, data),
    get: (path) => tissdbHttpClient.request('GET', path),
    put: (path, data) => tissdbHttpClient.request('PUT', path, data),
    delete: (path) => tissdbHttpClient.request('DELETE', path),
};

// --- Test Runner --- //
const runTests = async () => {
    const db = new TissDBLite();
    let assertions = 0;
    let failures = 0;

    const assert = (description, condition) => {
        assertions++;
        if (!condition) {
            failures++;
            console.error(`❌ FAILURE: ${description}`);
        } else {
            console.log(`✅ SUCCESS: ${description}`);
        }
    };

    const assertThrows = (description, func, expectedError) => {
        assertions++;
        try {
            func();
            failures++;
            console.error(`❌ FAILURE: ${description} - Expected to throw, but did not.`);
        } catch (error) {
            if (error.message.includes(expectedError)) {
                console.log(`✅ SUCCESS: ${description}`);
            } else {
                failures++;
                console.error(`❌ FAILURE: ${description} - Threw wrong error: ${error.message}`);
            }
        }
    };

    console.log('--- Running TissDB-Lite Unit Tests ---');

    // Test Collection Creation
    db.createCollection('users');
    assert('Should create a new collection', db.collections['users'] !== undefined);
    assert('Newly created collection should be empty', db.collections['users'].length === 0);

    // Test Insert
    const user1 = db.insert('users', { name: 'Alice', age: 30 });
    assert('Should insert a user', db.collections['users'].length === 1);
    assert('Inserted user should have an _id', user1._id !== undefined);
    assert('Inserted user should have _createdDate', user1._createdDate !== undefined);
    assert('Inserted user should have _updatedDate', user1._updatedDate !== undefined);

    const user2 = db.insert('users', { name: 'Bob', age: 40, role: 'admin' });
    const user3 = db.insert('users', { name: 'Charlie', age: 30, status: 'active' });
    const user4 = db.insert('users', { name: 'David', age: 25, status: 'inactive' });

    // Test bulkInsert
    db.createCollection('products');
    const newProducts = [
        { name: 'Laptop', price: 1200, category: 'Electronics' },
        { name: 'Mouse', price: 25, category: 'Electronics' },
        { name: 'Keyboard', price: 75, category: 'Electronics' },
    ];
    const insertedProducts = db.bulkInsert('products', newProducts);
    assert('Should bulk insert products', db.collections['products'].length === 3);
    assert('Bulk inserted products should have _id', insertedProducts[0]._id !== undefined);

    // Test basic find
    const allUsers = db.find('users');
    assert('Should find all users when no query is given', allUsers.length === 4);

    // Test find with simple query
    const thirtyYearOlds = db.find('users', 'age = 30');
    assert('Should find users with age = 30', thirtyYearOlds.length === 2);
    assert('Find with age = 30 should return Alice', thirtyYearOlds.some(u => u.name === 'Alice'));
    assert('Find with age = 30 should return Charlie', thirtyYearOlds.some(u => u.name === 'Charlie'));

    // Test find with more complex query (AND)
    const adminUser = db.find('users', 'age > 35 AND role = "admin"');
    assert('Should find users with age > 35 AND role = "admin"', adminUser.length === 1);
    assert('The admin user should be Bob', adminUser[0].name === 'Bob');
    
    // Test find with more complex query (OR)
    const thirtyOrAdmin = db.find('users', 'age = 30 OR role = "admin"');
    assert('Should find users with age = 30 OR role = "admin"', thirtyOrAdmin.length === 3);
    assert('thirtyOrAdmin should include Alice', thirtyOrAdmin.some(u => u.name === 'Alice'));
    assert('thirtyOrAdmin should include Bob', thirtyOrAdmin.some(u => u.name === 'Bob'));
    assert('thirtyOrAdmin should include Charlie', thirtyOrAdmin.some(u => u.name === 'Charlie'));

    // Test find with different operators
    const olderThan30 = db.find('users', 'age > 30');
    assert('Should find users older than 30', olderThan30.length === 1);
    assert('User older than 30 should be Bob', olderThan30[0].name === 'Bob');

    const notAdmin = db.find('users', 'role != "admin"');
    assert('Should find users not admin', notAdmin.length === 3);
    assert('Not admin should include Alice', notAdmin.some(u => u.name === 'Alice'));
    assert('Not admin should include Charlie', notAdmin.some(u => u.name === 'Charlie'));
    assert('Not admin should include David', notAdmin.some(u => u.name === 'David'));

    const activeUsers = db.find('users', 'status = "active"');
    assert('Should find active users', activeUsers.length === 1);
    assert('Active user should be Charlie', activeUsers[0].name === 'Charlie');

    // Test find with parentheses
    const complexQuery = db.find('users', '(age = 30 OR age = 25) AND status = "active"');
    assert('Should find users with complex query (age = 30 OR age = 25) AND status = "active"', complexQuery.length === 1);
    assert('Complex query result should be Charlie', complexQuery[0].name === 'Charlie');

    // Test update
    const updatedUser1 = { ...user1, age: 31, status: 'active' };
    db.update('users', updatedUser1);
    const foundUpdated = db.find('users', `_id = "${user1._id}"`);
    assert('Should update a user\'s age', foundUpdated[0].age === 31);
    assert('Should update a user\'s status', foundUpdated[0].status === 'active');
    assert('Updated user should have new _updatedDate', foundUpdated[0]._updatedDate !== user1._updatedDate);

    // Test remove
    db.remove('users', user2._id);
    assert('Should remove a user', db.collections['users'].length === 3);
    const foundRemoved = db.find('users', `_id = "${user2._id}"`);
    assert('Should not find a removed user', foundRemoved.length === 0);

    // Test exportCollection
    const exportedProducts = db.exportCollection('products');
    assert('Should export all products', exportedProducts.length === 3);
    assert('Exported products should be a deep copy', exportedProducts !== db.collections['products']);
    assert('Exported products should contain Laptop', exportedProducts.some(p => p.name === 'Laptop'));

    // Test error handling for non-existent collection
    assertThrows(
        'Should throw error when inserting into non-existent collection',
        () => db.insert('nonExistent', { data: 'test' }),
        'Collection \'nonExistent\' does not exist.'
    );
    assertThrows(
        'Should throw error when finding in non-existent collection',
        () => db.find('nonExistent', 'field = "value"'),
        'Collection \'nonExistent\' does not exist.'
    );
    assertThrows(
        'Should throw error when updating in non-existent collection',
        () => db.update('nonExistent', { _id: '123', data: 'test' }),
        'Collection \'nonExistent\' does not exist.'
    );
    assertThrows(
        'Should throw error when removing from non-existent collection',
        () => db.remove('nonExistent', '123'),
        'Collection \'nonExistent\' does not exist.'
    );
    assertThrows(
        'Should throw error when bulk inserting into non-existent collection',
        () => db.bulkInsert('nonExistent', [{ data: 'test' }]),
        'Collection \'nonExistent\' does not exist.'
    );
    assertThrows(
        'Should throw error when exporting non-existent collection',
        () => db.exportCollection('nonExistent'),
        'Collection \'nonExistent\' does not exist.'
    );

    // Test error handling for updating non-existent item
    assertThrows(
        'Should throw error when updating non-existent item',
        () => db.update('users', { _id: 'nonExistentId', name: 'Ghost' }),
        'Item with id \'nonExistentId\' not found in collection \'users\'.' 
    );

    console.log('--- Running Migration Tests (Actual TissDB) ---');

    const TISSDB_TEST_COLLECTION_1 = 'tissdb_migration_test_1';
    const TISSDB_TEST_COLLECTION_2 = 'tissdb_migration_test_2';

    // Cleanup previous test runs on TissDB
    console.log(`Cleaning up TissDB collection: ${TISSDB_TEST_COLLECTION_1}`);
    const delete1Response = await tissdbHttpClient.delete(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_1}`);
    console.log(`Delete ${TISSDB_TEST_COLLECTION_1} Status: ${delete1Response.statusCode}, Data:`, delete1Response.data);
    console.log(`Cleaning up TissDB collection: ${TISSDB_TEST_COLLECTION_2}`);
    const delete2Response = await tissdbHttpClient.delete(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_2}`);
    console.log(`Delete ${TISSDB_TEST_COLLECTION_2} Status: ${delete2Response.statusCode}, Data:`, delete2Response.data);
    // TissDB returns 204 for successful delete, or 404 if not found. Both are fine for cleanup.

    // Create collections on TissDB
    console.log(`Creating TissDB collection: ${TISSDB_TEST_COLLECTION_1}`);
    const put1Response = await tissdbHttpClient.put(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_1}`);
    console.log(`PUT ${TISSDB_TEST_COLLECTION_1} Status: ${put1Response.statusCode}, Data:`, put1Response.data);
    console.log(`Creating TissDB collection: ${TISSDB_TEST_COLLECTION_2}`);
    const put2Response = await tissdbHttpClient.put(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_2}`);
    console.log(`PUT ${TISSDB_TEST_COLLECTION_2} Status: ${put2Response.statusCode}, Data:`, put2Response.data);

    // Populate TissDB
    const tissdbItems = [
        { name: 'TissDB Item 1', value: 100 },
        { name: 'TissDB Item 2', value: 200 },
        { name: 'TissDB Item 3', value: 150 }
    ];
    // Assign unique _id to each item before sending to TissDB
    tissdbItems.forEach(item => item._id = db._generateId());

    console.log(`Populating TissDB collection: ${TISSDB_TEST_COLLECTION_1} with ${tissdbItems.length} items`);
    for (const item of tissdbItems) {
        // TissDB expects PUT for documents with specified IDs
        const response = await tissdbHttpClient.put(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_1}/${item._id}`, item);
        console.log(`TissDB PUT response for item:`, response.statusCode, response.data);
    }
    const initialTissDBData = (await tissdbHttpClient.post(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_1}/_query`, { query: `SELECT * FROM ${TISSDB_TEST_COLLECTION_1}` })).data;
    console.log(`Initial TissDB Data for ${TISSDB_TEST_COLLECTION_1}:`, initialTissDBData);
    assert('Actual TissDB should have 3 items initially', initialTissDBData.length === 3);

    // Migration: TissDB to TissDB-Lite
    db.createCollection('migrated_from_tissdb');
    const fetchedTissDBData = (await tissdbHttpClient.post(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_1}/_query`, { query: `SELECT * FROM ${TISSDB_TEST_COLLECTION_1}` })).data;
    console.log(`Fetched TissDB Data for migration:`, fetchedTissDBData);
    db.bulkInsert('migrated_from_tissdb', fetchedTissDBData);
    assert('TissDB-Lite should have 3 items after migration from TissDB', db.collections['migrated_from_tissdb'].length === 3);
    assert('Migrated item should have original name', db.collections['migrated_from_tissdb'][0].name === 'TissDB Item 1');

    // Migration: TissDB-Lite to TissDB
    const liteItemsToMigrate = [
        { product: 'Lite Gadget X', price: 50 },
        { product: 'Lite Gadget Y', price: 75 }
    ];
    db.createCollection('migrated_to_tissdb');
    db.bulkInsert('migrated_to_tissdb', liteItemsToMigrate);

    const dataToExport = db.exportCollection('migrated_to_tissdb');
    console.log(`Exported TissDB-Lite Data for migration:`, dataToExport);
    for (const item of dataToExport) {
        // TissDB expects PUT for documents with specified IDs
        const response = await tissdbHttpClient.put(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_2}/${item._id}`, item);
        console.log(`TissDB PUT response for item:`, response.statusCode, response.data);
    }
    const finalTissDBData = (await tissdbHttpClient.post(`/${TISSDB_DB_NAME}/${TISSDB_TEST_COLLECTION_2}/_query`, { query: `SELECT * FROM ${TISSDB_TEST_COLLECTION_2}` })).data;
    console.log(`Final TissDB Data for ${TISSDB_TEST_COLLECTION_2}:`, finalTissDBData);
    assert('Actual TissDB should have 2 items after migration from TissDB-Lite', finalTissDBData.length === 2);
    assert('Migrated item to TissDB should have product name', finalTissDBData.some(item => item.product === 'Lite Gadget X'));

    // Final Cleanup on TissDB
    console.log(`Final cleanup of TissDB collection: ${TISSDB_TEST_COLLECTION_1}`);
    await tissdbHttpClient.delete(`/${TISSDB_TEST_COLLECTION_1}`);
    console.log(`Final cleanup of TissDB collection: ${TISSDB_TEST_COLLECTION_2}`);
    await tissdbHttpClient.delete(`/${TISSDB_TEST_COLLECTION_2}`);

    console.log('--- Test Summary ---');
    console.log(`Total Assertions: ${assertions}`);
    console.log(`Failures: ${failures}`);

    if (failures > 0) {
        console.error('--- TESTS FAILED ---');
        process.exit(1);
    } else {
        console.log('--- ALL TESTS PASSED ---');
    }
};

runTests();