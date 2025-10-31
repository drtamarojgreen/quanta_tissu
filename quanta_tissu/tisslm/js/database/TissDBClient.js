// TissDB Client for Node.js
const http = require('http');
const { URL } = require('url');

class TissDBClient {
    constructor(baseUrl, token) {
        if (!baseUrl) {
            throw new Error("TissDB base URL is required.");
        }
        this.baseUrl = new URL(baseUrl);
        this.token = token || "static_test_token"; // Default to test token
    }

    async _request(method, path, body = null) {
        return new Promise((resolve, reject) => {
            const options = {
                hostname: this.baseUrl.hostname,
                port: this.baseUrl.port,
                path: path,
                method: method,
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${this.token}`,
                },
            };

            const req = http.request(options, (res) => {
                let data = '';
                res.on('data', (chunk) => {
                    data += chunk;
                });
                res.on('end', () => {
                    if (res.statusCode >= 200 && res.statusCode < 300) {
                        try {
                            resolve(JSON.parse(data));
                        } catch (e) {
                            resolve({ message: "Received non-JSON response.", body: data });
                        }
                    } else {
                        reject(new Error(`HTTP Error: ${res.statusCode} - ${data}`));
                    }
                });
            });

            req.on('error', (e) => {
                reject(new Error(`Request failed: ${e.message}`));
            });

            if (body) {
                req.write(JSON.stringify(body));
            }
            req.end();
        });
    }

    // Generic request methods
    async get(path) {
        return this._request('GET', path);
    }

    async post(path, body) {
        return this._request('POST', path, body);
    }

    async put(path, body) {
        return this._request('PUT', path, body);
    }

    async del(path) {
        return this._request('DELETE', path);
    }

    // High-level database operations

    async createCollection(dbName, collectionName) {
        // Assuming an API endpoint like POST /{db_name}/{collection_name}
        // This is a guess; actual API might differ.
        return this.post(`/${dbName}/${collectionName}`, { action: "create" });
    }

    async addDocument(dbName, collectionName, document, docId = null) {
        if (docId) {
            // PUT to a specific resource URL to create/replace with a known ID
            return this.put(`/${dbName}/${collectionName}/${docId}`, document);
        } else {
            // POST to a collection URL to create with a server-generated ID
            return this.post(`/${dbName}/${collectionName}`, document);
        }
    }

    async getDocument(dbName, collectionName, docId) {
        return this.get(`/${dbName}/${collectionName}/${docId}`);
    }

    async deleteDocument(dbName, collectionName, docId) {
        return this.del(`/${dbName}/${collectionName}/${docId}`);
    }

    async query(dbName, collectionName, query) {
         // Assuming query is a JSON object sent via POST
        return this.post(`/${dbName}/${collectionName}/_query`, query);
    }
}

module.exports = { TissDBClient };
