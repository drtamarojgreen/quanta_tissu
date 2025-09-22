# TissDB Migration Guide

This guide provides instructions and considerations for migrating data between `tissdb`, `tissdb-lite` (in-memory), and Wix Data collections.

## Overview of Data Stores

*   **`tissdb`**: A standalone database instance with a RESTful API. Data is persistent.
*   **`tissdb-lite` (in-memory)**: An ephemeral, in-memory database within the Wix Velo environment. Data is *not* persistent across Velo backend restarts.
*   **Wix Data Collections**: Persistent data storage provided by the Wix platform, accessible via the `wix-data` API in Velo.

## Migration Scenarios

### 1. Migrating from `tissdb` to `tissdb-lite` (In-Memory)

This scenario is useful for loading a subset of `tissdb` data into `tissdb-lite` for temporary processing or caching within a Velo backend function.

**Process:**

1.  **Export from `tissdb`**: Use `tissdb`'s REST API to fetch the desired data. You can use `GET /<collection_name>/_query` to retrieve documents, potentially with filters.
    *   **Example (Node.js using `axios` in a Velo backend function):**
        ```javascript
        import axios from 'axios';

        const TISSDB_HOST = "your_tissdb_host";
        const TISSDB_PORT = 8080; // Or your tissdb port
        const TISSDB_BASE_URL = `http://${TISSDB_HOST}:${TISSDB_PORT}`;

        async function fetchDataFromTissDB(collectionName, query = {}) {
            try {
                const response = await axios.post(`${TISSDB_BASE_URL}/${collectionName}/_query`, { query: query });
                return response.data;
            } catch (error) {
                console.error(`Error fetching from TissDB: ${error.message}`);
                return [];
            }
        }
        ```
2.  **Import into `tissdb-lite`**: Use `tissdb-lite`'s `bulkInsert` method to add the fetched data to an in-memory collection.
    *   **Example (Velo backend function):**
        ```javascript
        import { TissDBLite } from 'backend/tissdb-lite'; // Adjust path as needed

        const tissdbLite = new TissDBLite();
        tissdbLite.createCollection('temp_users');

        export async function migrateUsersToTissDBLite() {
            const usersFromTissDB = await fetchDataFromTissDB('users', { /* optional query */ });
            if (usersFromTissDB.length > 0) {
                tissdbLite.bulkInsert('temp_users', usersFromTissDB);
                console.log(`Migrated ${usersFromTissDB.length} users to tissdb-lite.`);
            }
        }
        ```

### 2. Migrating from `tissdb-lite` (In-Memory) to `tissdb`

This scenario is for persisting temporary data from `tissdb-lite` back into a `tissdb` instance.

**Process:**

1.  **Export from `tissdb-lite`**: Use `tissdb-lite`'s `exportCollection` method to retrieve all items from an in-memory collection.
    *   **Example (Velo backend function):**
        ```javascript
        import { TissDBLite } from 'backend/tissdb-lite'; // Adjust path as needed
        import axios from 'axios';

        const TISSDB_HOST = "your_tissdb_host";
        const TISSDB_PORT = 8080;
        const TISSDB_BASE_URL = `http://${TISSDB_HOST}:${TISSDB_PORT}`;

        const tissdbLite = new TissDBLite();
        // ... populate tissdbLite.collections['temp_data'] ...

        export async function migrateDataFromTissDBLiteToTissDB() {
            const dataToPersist = tissdbLite.exportCollection('temp_data');
            for (const item of dataToPersist) {
                try {
                    // Assuming POST creates a new document, PUT updates if _id exists
                    await axios.post(`${TISSDB_BASE_URL}/your_tissdb_collection`, item);
                } catch (error) {
                    console.error(`Error saving item to TissDB: ${error.message}`);
                }
            }
            console.log(`Migrated ${dataToPersist.length} items from tissdb-lite to TissDB.`);
        }
        ```
2.  **Import into `tissdb`**: Send each item to `tissdb`'s REST API (e.g., `POST /<collection_name>`).

### 3. Migrating from `tissdb` to Wix Data Collections

This is a common scenario for moving data from an external `tissdb` instance to Wix's native persistent storage. `tissdb-lite` acts as a query engine for Wix Data, but for bulk import, you'll directly use Wix Data's APIs.

**Process:**

1.  **Export from `tissdb`**: Fetch data from `tissdb` using its REST API (as shown in Scenario 1).
2.  **Import into Wix Data**: Use Wix Data's `bulkInsert` or `insert` methods.
    *   **Example (Velo backend function):**
        ```javascript
        import wixData from 'wix-data';
        import axios from 'axios'; // Assuming axios is available or similar HTTP client

        const TISSDB_HOST = "your_tissdb_host";
        const TISSDB_PORT = 8080;
        const TISSDB_BASE_URL = `http://${TISSDB_HOST}:${TISSDB_PORT}`;

        async function fetchDataFromTissDB(collectionName, query = {}) {
            // ... (implementation as in Scenario 1) ...
        }

        export async function migrateTissDBToWixData() {
            const productsFromTissDB = await fetchDataFromTissDB('products');
            if (productsFromTissDB.length > 0) {
                try {
                    // Wix Data bulkInsert is more efficient for many items
                    const results = await wixData.bulkInsert('YourWixCollectionName', productsFromTissDB);
                    console.log(`Migrated ${results.inserted} products to Wix Data.`);
                } catch (error) {
                    console.error(`Error bulk inserting into Wix Data: ${error.message}`);
                }
            }
        }
        ```

### 4. Migrating from Wix Data Collections to `tissdb`

This scenario involves exporting data from Wix's persistent storage to an external `tissdb` instance.

**Process:**

1.  **Export from Wix Data**: Use `wix-data`'s query API to retrieve data from your Wix collection.
    *   **Example (Velo backend function):**
        ```javascript
        import wixData from 'wix-data';
        import axios from 'axios';

        const TISSDB_HOST = "your_tissdb_host";
        const TISSDB_PORT = 8080;
        const TISSDB_BASE_URL = `http://${TISSDB_HOST}:${TISSDB_PORT}`;

        export async function migrateWixDataToTissDB() {
            try {
                const results = await wixData.query('YourWixCollectionName').find();
                const itemsToMigrate = results.items;

                for (const item of itemsToMigrate) {
                    // Remove Wix-specific fields if they conflict with TissDB schema
                    const { _id, _owner, _createdDate, _updatedDate, ...tissdbItem } = item;
                    await axios.post(`${TISSDB_BASE_URL}/your_tissdb_collection`, tissdbItem);
                }
                console.log(`Migrated ${itemsToMigrate.length} items from Wix Data to TissDB.`);
            } catch (error) {
                console.error(`Error migrating from Wix Data to TissDB: ${error.message}`);
            }
        }
        ```
2.  **Import into `tissdb`**: Send each item to `tissdb`'s REST API.

## Considerations for Migration

*   **Schema Mapping**: Ensure that the data schemas between the source and destination databases are compatible. You may need to transform data fields during migration.
*   **ID Management**: When migrating to `tissdb-lite` or `tissdb`, new `_id`s might be generated unless you explicitly manage them. When migrating from `tissdb` to Wix Data, Wix will generate its own `_id`s.
*   **Data Volume**: For large datasets, consider batching operations to avoid timeouts or memory issues.
*   **Error Handling**: Implement robust error handling and logging for migration scripts.
*   **Data Consistency**: Plan for potential data inconsistencies if migration is interrupted.
*   **Wix Data Limits**: Be aware of Wix Data API rate limits and item limits.
*   **Authentication**: Ensure proper authentication is handled when accessing `tissdb`'s REST API.
