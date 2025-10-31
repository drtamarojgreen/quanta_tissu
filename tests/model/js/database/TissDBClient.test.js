// NOTE: This is a test file. Per instructions, it should not be executed.
const http = require('http');
const { TissDBClient } = require('../../../../quanta_tissu/tisslm/js/database/TissDBClient.js');

// A simple assertion function for testing purposes.
function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

// Mocking Node's http.request to avoid actual network calls
// This is a simplified mock for demonstration.
let mockResponse = {};
let mockRequestOptions = null;
let mockRequestBody = null;

http.request = (options, callback) => {
    mockRequestOptions = options;
    const res = {
        statusCode: mockResponse.statusCode || 200,
        on: (event, handler) => {
            if (event === 'data') {
                handler(JSON.stringify(mockResponse.body || {}));
            }
            if (event === 'end') {
                handler();
            }
        },
    };
    callback(res);
    return {
        on: () => {},
        write: (body) => { mockRequestBody = body; },
        end: () => {},
    };
};


async function testClientInitialization() {
    console.log("Test: Client Initialization");
    const client = new TissDBClient('http://localhost:8080', 'my-token');
    assert(client.baseUrl.hostname === 'localhost', "Hostname should be parsed correctly.");
    assert(client.baseUrl.port === '8080', "Port should be parsed correctly.");
    assert(client.token === 'my-token', "Token should be set.");
}

async function testGetDocument() {
    console.log("Test: GET request for a document");
    const client = new TissDBClient('http://localhost:8080');
    mockResponse = { statusCode: 200, body: { _id: '123', content: 'hello' } };

    const doc = await client.getDocument('testdb', 'testcol', '123');

    assert(mockRequestOptions.method === 'GET', "Request method should be GET.");
    assert(mockRequestOptions.path === '/testdb/testcol/123', "Request path should be correct.");
    assert(mockRequestOptions.headers['Authorization'] === 'Bearer static_test_token', "Auth header should be correct.");
    assert(doc.content === 'hello', "Response body should be parsed correctly.");
}

async function testAddDocumentWithId() {
    console.log("Test: PUT request to add a document with ID");
    const client = new TissDBClient('http://localhost:8080');
    mockResponse = { statusCode: 201, body: { _id: 'abc', status: 'created' } };

    const doc = { content: "new doc" };
    await client.addDocument('testdb', 'testcol', doc, 'abc');

    assert(mockRequestOptions.method === 'PUT', "Request method should be PUT.");
    assert(mockRequestOptions.path === '/testdb/testcol/abc', "Request path should be correct.");
    assert(mockRequestBody === JSON.stringify(doc), "Request body should be correct.");
}

async function testAddDocumentWithoutId() {
    console.log("Test: POST request to add a document without ID");
    const client = new TissDBClient('http://localhost:8080');
    mockResponse = { statusCode: 201, body: { _id: 'server-generated-id', status: 'created' } };

    const doc = { content: "another new doc" };
    await client.addDocument('testdb', 'testcol', doc);

    assert(mockRequestOptions.method === 'POST', "Request method should be POST.");
    assert(mockRequestOptions.path === '/testdb/testcol', "Request path should be correct.");
    assert(mockRequestBody === JSON.stringify(doc), "Request body should be correct.");
}


async function runAllClientTests() {
    try {
        await testClientInitialization();
        await testGetDocument();
        await testAddDocumentWithId();
        await testAddDocumentWithoutId();
        console.log("-> All TissDBClient tests loaded successfully.");
    } catch (e) {
        console.error("-> A client test definition failed:", e.message);
    }
}

runAllClientTests();
