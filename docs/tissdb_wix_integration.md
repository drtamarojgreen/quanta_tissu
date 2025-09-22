# Integrating TissDB with Wix

This document provides a detailed guide on how to integrate the TissDB database with a Wix website.

## 1. Introduction

This guide will walk you through the process of connecting a TissDB database to a Wix site. This is achieved by creating a "Wix Adaptor," which is a web service that acts as a bridge between Wix and TissDB. The adaptor translates Wix Data API requests into TissDB API requests, allowing you to use TissDB as a backend for your Wix site.

## 2. Prerequisites

Before you begin, you will need the following:

*   A running instance of TissDB.
*   A Wix account with a site that has Dev Mode enabled.
*   Node.js and npm installed on your local machine.
*   A Heroku account (or another PaaS/container hosting solution).

## 3. TissDB API Overview

TissDB provides a RESTful API for interacting with the database. The key endpoints are:

*   **Collections:**
    *   `PUT /<collection_name>`: Create a collection.
    *   `DELETE /<collection_name>`: Delete a collection.
*   **Documents:**
    *   `POST /<collection_name>`: Create a document.
    *   `GET /<collection_name>/<document_id>`: Retrieve a document.
    *   `PUT /<collection_name>/<document_id>`: Update a document.
    *   `DELETE /<collection_name>/<document_id>`: Delete a document.
*   **Querying:**
    *   `POST /<collection_name>/_query`: Execute a TissQL query.

## 4. Wix Adaptor Design

The Wix adaptor is a Node.js application built with the Express framework. It implements the Wix External Database API and translates the requests to the TissDB API.

### Technology Stack

*   **Language:** Node.js
*   **Framework:** Express.js
*   **HTTP Client:** axios

### API Mapping

The adaptor maps the Wix Data API to the TissDB API as described in the implementation section.

### Data Transformation

The adaptor is responsible for managing the Wix system fields (`_id`, `_createdDate`, `_updatedDate`, `_owner`).

### Configuration

The adaptor is configured using environment variables:

*   `TISSDB_HOST`: The hostname of the TissDB server.
*   `TISSDB_PORT`: The port of the TissDB server.

## 5. Implementation Details

Here is a brief guide to implementing the Wix adaptor.

### Project Setup

```bash
mkdir wix-tissdb-adaptor
cd wix-tissdb-adaptor
npm init -y
npm install express axios dotenv uuid
```

### Code

Create a file named `index.js` with the following content:

```javascript
require('dotenv').config();
const express = require('express');
const axios = require('axios');
const { v4: uuidv4 } = require('uuid');

const app = express();
app.use(express.json());

const TISSDB_HOST = process.env.TISSDB_HOST || 'localhost';
const TISSDB_PORT = process.env.TISSDB_PORT || 8080;
const TISSDB_BASE_URL = `http://${TISSDB_HOST}:${TISSDB_PORT}`;

// Get item
app.get('/items/:collectionId/:itemId', async (req, res, next) => {
  try {
    const { collectionId, itemId } = req.params;
    const response = await axios.get(`${TISSDB_BASE_URL}/${collectionId}/${itemId}`);
    res.json(response.data);
  } catch (error) {
    next(error);
  }
});

// Create item
app.post('/items/:collectionId', async (req, res, next) => {
  try {
    const { collectionId } = req.params;
    const newItem = {
      ...req.body,
      _id: uuidv4(),
      _createdDate: new Date().toISOString(),
      _updatedDate: new Date().toISOString(),
    };
    await axios.post(`${TISSDB_BASE_URL}/${collectionId}`, newItem);
    res.status(201).json(newItem);
  } catch (error) {
    next(error);
  }
});

// Update item
app.put('/items/:collectionId/:itemId', async (req, res, next) => {
    try {
        const { collectionId, itemId } = req.params;
        const updatedItem = {
            ...req.body,
            _updatedDate: new Date().toISOString(),
        };
        await axios.put(`${TISSDB_BASE_URL}/${collectionId}/${itemId}`, updatedItem);
        res.json(updatedItem);
    } catch (error) {
        next(error);
    }
});

// Delete item
app.delete('/items/:collectionId/:itemId', async (req, res, next) => {
    try {
        const { collectionId, itemId } = req.params;
        await axios.delete(`${TISSDB_BASE_URL}/${collectionId}/${itemId}`);
        res.status(204).send();
    } catch (error) {
        next(error);
    }
});


// Query items
app.post('/items/:collectionId/query', async (req, res, next) => {
    try {
        const { collectionId } = req.params;
        // This is a simplified implementation. A real implementation would need to
        // parse the Wix query and translate it to a TissQL query.
        const response = await axios.post(`${TISSDB_BASE_URL}/${collectionId}/_query`, {
            query: `SELECT * FROM ${collectionId}`
        });
        res.json(response.data);
    } catch (error) {
        next(error);
    }
});


// Error handling middleware
app.use((error, req, res, next) => {
  console.error(error);
  const status = error.response ? error.response.status : 500;
  const message = error.response ? error.response.data : 'Internal Server Error';
  res.status(status).json({ message });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Wix TissDB Adaptor listening on port ${PORT}`);
});
```

## 6. Deployment

You can deploy the adaptor to a PaaS like Heroku or using Docker.

### Heroku

1.  Create a `Procfile`: `web: node index.js`
2.  `heroku create <app_name>`
3.  `heroku config:set TISSDB_HOST=<your_tissdb_host> TISSDB_PORT=<your_tissdb_port>`
4.  `git push heroku main`

### Docker

1.  Create a `Dockerfile`.
2.  Build and push the image to a container registry.
3.  Deploy the container to a cloud provider.

### AWS EC2 (IaaS)

Deploying to Amazon EC2 provides full control over the server environment but requires more manual setup. This is an Infrastructure-as-a-Service (IaaS) approach.

1.  **Launch an EC2 Instance:**
    *   From the AWS Management Console, navigate to the EC2 dashboard.
    *   Launch a new instance. A good choice for the Amazon Machine Image (AMI) is a recent version of Ubuntu Server.
    *   Choose an instance type (e.g., `t2.micro` for a small application).
    *   Configure the instance details, storage, and tags as needed.

2.  **Configure Security Group:**
    *   Create a new security group or modify the existing one.
    *   Add an inbound rule to allow TCP traffic on the port your application will use (e.g., port 3000). For security, you can restrict the source to your IP address or, if it needs to be public, `0.0.0.0/0`. Also, ensure port 22 is open for SSH access.

3.  **Connect to the Instance:**
    *   Once the instance is running, connect to it using SSH and your private key file.
        ```bash
        ssh -i /path/to/your-key.pem ubuntu@<your_ec2_public_ip>
        ```

4.  **Install Dependencies:**
    *   Update the package manager and install Node.js and npm.
        ```bash
        sudo apt-get update
        sudo apt-get install -y nodejs npm
        ```
    *   Install a process manager like `pm2` to keep your application running in the background.
        ```bash
        sudo npm install -g pm2
        ```

5.  **Deploy and Run the Application:**
    *   Clone your application code from your Git repository.
        ```bash
        git clone <your_repository_url>
        cd wix-tissdb-adaptor
        ```
    *   Install the application dependencies.
        ```bash
        npm install
        ```
    *   Set the required environment variables. You can do this by creating a `.env` file or by passing them to the `pm2` start command.
        ```bash
        export TISSDB_HOST=<your_tissdb_host>
        export TISSDB_PORT=<your_tissdb_port>
        ```
    *   Start your application with `pm2`.
        ```bash
        pm2 start index.js --name wix-adaptor
        ```
    *   Your adaptor is now running on the EC2 instance and accessible via its public IP address and the port you configured.

### Google App Engine (PaaS)

Google App Engine is a Platform-as-a-Service (PaaS) that abstracts away the underlying infrastructure, similar to Heroku.

1.  **Prerequisites:**
    *   A Google Cloud Platform project.
    *   The `gcloud` command-line tool installed and authenticated.

2.  **Create `app.yaml`:**
    *   In the root of your project directory, create a file named `app.yaml`. This file tells App Engine how to run your application.
        ```yaml
        runtime: nodejs16  # or nodejs18, etc.
        instance_class: F1 # The smallest, most cost-effective instance class

        # Set environment variables
        env_variables:
          TISSDB_HOST: '<your_tissdb_host>'
          TISSDB_PORT: '<your_tissdb_port>'
        ```

3.  **Add a `start` script:**
    *   Ensure your `package.json` file has a `start` script, which App Engine uses to start your application.
        ```json
        "scripts": {
          "start": "node index.js",
          "test": "echo \"Error: no test specified\" && exit 1"
        },
        ```

4.  **Deploy the Application:**
    *   Navigate to your project's root directory in your terminal.
    *   Run the deployment command:
        ```bash
        gcloud app deploy
        ```
    *   The `gcloud` tool will package your application, upload it to Google Cloud, and deploy it.
    *   Once the deployment is complete, it will provide you with a public URL where your adaptor is accessible.

## 7. Connecting to Wix

1.  Open the Wix Editor and enable Dev Mode.
2.  Go to the "Databases" section.
3.  Click "Connect to External Database".
4.  Enter the URL of your deployed adaptor.
5.  Give the collection a name.
6.  Click "Save".

You can now use the external collection in your Wix site.

## 8. Detailed TissQL Queries

While the Wix Adaptor can handle basic queries, a more robust implementation would need to translate Wix's query object into a detailed TissQL query. TissDB's query endpoint is powerful and supports parameterized queries to prevent injection attacks.

### Basic Filtering

To filter results, you use a `WHERE` clause.

**Request Body for `POST /<collection_name>/_query`**:
```json
{
  "query": "SELECT user_id, post_text, view_count FROM posts WHERE author_id = ?",
  "params": ["user123"]
}
```

### Advanced Filtering

You can use standard logical operators like `AND`, `OR`, and comparison operators like `>`, `<`, `>=`, `<=`.

**Request Body for `POST /<collection_name>/_query`**:
```json
{
  "query": "SELECT * FROM products WHERE (category = ? OR category = ?) AND price < ?",
  "params": ["electronics", "appliances", 500.00]
}
```

### Sorting Results

You can sort results using `ORDER BY`.

**Request Body for `POST /<collection_name>/_query`**:
```json
{
  "query": "SELECT * FROM articles ORDER BY publication_date DESC",
  "params": []
}
```

### Limiting Results and Pagination

You can limit the number of results and implement pagination using `LIMIT` and `OFFSET`.

**Request Body for `POST /<collection_name>/_query`**:
```json
{
  "query": "SELECT * FROM comments LIMIT ? OFFSET ?",
  "params": [20, 40]
}
```
This query would skip the first 40 comments and return the next 20, which is useful for implementing page 3 of a comment section where each page has 20 comments.

## 9. Conceptual Design: `tissdb-lite` for Wix Velo

This section outlines a conceptual design for `tissdb-lite`, a hypothetical, lightweight, in-memory database implemented purely in JavaScript for use within the Wix Velo environment. This would eliminate the need for an external server and adaptor.

### Core Principles

*   **In-Memory:** The database would exist in the memory of the Velo backend instance. Data would be non-persistent and would reset when the Wix site's backend restarts.
*   **Pure JavaScript:** No native dependencies. It would be a standard npm package that can be installed in a Velo project.
*   **API Compatibility:** It would expose a subset of the Wix Data API, making it easy to use as a replacement for the standard Wix Data collections in many scenarios.
*   **JSON-based:** Collections would be simple arrays of JSON objects.

### Architecture

The `tissdb-lite` package would export a single class, `TissDBLite`.

```javascript
// In a backend .jsw file in your Velo site
import { TissDBLite } from 'tissdb-lite';

const db = new TissDBLite();

// Create a collection
db.createCollection('products');
```

### Key Methods

The `TissDBLite` class would have methods that mimic the Wix Data API:

*   `createCollection(name)`: Creates an in-memory array to act as a collection.
*   `insert(collectionName, item)`: Inserts a new JSON object into a collection. It would automatically add `_id`, `_createdDate`, and `_updatedDate` fields.
*   `find(collectionName, query)`: Finds items in a collection. The `query` object would be a simplified version of the Wix Data query object, supporting basic filtering.
*   `update(collectionName, item)`: Updates an item in a collection.
*   `remove(collectionName, itemId)`: Removes an item from a collection.

### Example Usage in Wix Velo

Here's how it could be used in a backend web module (`.jsw` file) in Wix:

```javascript
// backend/my-database.jsw
import { TissDBLite } from 'tissdb-lite';

const db = new TissDBLite();
db.createCollection('visitors');

export function recordVisit(visitorInfo) {
  const visitor = {
    ...visitorInfo,
    visitTime: new Date()
  };
  return db.insert('visitors', visitor);
}

export function getRecentVisitors() {
  const query = { /* ... simplified query ... */ };
  return db.find('visitors', query);
}
```

### Limitations

*   **No Persistence:** This is the most significant limitation. The database would be ephemeral. This makes it suitable for caching, temporary data, or session-specific data, but not for permanent storage.
*   **Limited Querying:** A full TissQL parser would be too complex for a "lite" version. It would support only basic filtering and sorting.
*   **Single-Instance:** The database would be tied to a single Velo backend instance and not shared across multiple instances.

This `tissdb-lite` concept presents a trade-off: it simplifies the architecture by removing the external server, but at the cost of persistence and advanced query capabilities.

## 10. Expanding TissLang for Queries with `tissdb-lite`

Based on the user's request to expand `tisslm` for queries and to integrate with Wix collections for persistence, this section proposes a new high-level `QUERY` command for TissLang.

This design leverages the `tissdb-lite` concept, re-imagining it as a TissLang-to-Wix-Data query translation layer that runs within the Velo environment.

### The `QUERY` Command

The core of this expansion is a new command for TissLang, designed to be both powerful and easy to read.

#### Syntax

```tisslang
QUERY <collectionName> WHERE <condition> INTO <variableName>
```

*   **`QUERY`**: The new keyword that initiates a data query.
*   **`<collectionName>`**: The name of the Wix Data collection to be queried.
*   **`WHERE <condition>`**: A filter clause. The `<condition>` would support a simple grammar of field names, operators (`=`, `!=`, `<`, `>`, `<=`, `>=`), and logical connectors (`AND`, `OR`).
*   **`INTO <variableName>`**: The name of the TissLang variable where the array of results will be stored.

#### Example TissLang Script

```tisslang
TASK "Find and log expensive electronic products"

STEP "Query for products" {
    # Query the 'Products' Wix Collection for items where the category is 'electronics'
    # and the price is greater than 1000. Store the results in the 'expensive_products' variable.
    QUERY Products WHERE category = "electronics" AND price > 1000 INTO expensive_products

    # Log the results to the console. The variable is accessed using the $ prefix.
    LOG "Found products: $expensive_products"
}
```

### Implementation Details

This feature would require modifications to both the TissLang parser and the `tissdb-lite` module.

#### 1. TissLang Parser (`tisslang_parser.py`)

The parser would need to be updated to recognize and parse the `QUERY` command. A new pattern would be added to `_PATTERNS`, and a new handler would be added to the `_handle_in_step_state` method.

The handler would parse the command into a structured AST node, like this:

```json
{
  "type": "QUERY",
  "collection": "Products",
  "condition_string": "category = \"electronics\" AND price > 1000",
  "variable": "expensive_products"
}
```

#### 2. `tissdb-lite` as a Query Engine in Velo

The `tissdb-lite` module, running in Wix's backend, would be responsible for executing these query commands.

It would need a new method, `executeQuery`, which would be called by the TissLang runtime.

```javascript
// In the tissdb-lite module in Velo
import wixData from 'wix-data';

class TissDBLite {

  // ... other methods like insert, update ...

  async executeQuery(command) {
    const { collection, condition_string } = command;

    // 1. Parse the condition string into a wix-data filter object.
    // This is the most complex part. It requires a small parser.
    const filter = this.parseCondition(condition_string);

    // 2. Build and execute the wix-data query.
    let query = wixData.query(collection);
    if (filter) {
      query = query.filter(filter);
    }

    const results = await query.find();
    return results.items;
  }

  parseCondition(condition_string) {
    // This method would contain the logic to parse the condition string.
    // For example, "category = \"electronics\" AND price > 1000"
    // would be transformed into:
    //
    // wixData.filter()
    //   .eq("category", "electronics")
    //   .gt("price", 1000);
    //
    // This would require a robust implementation to handle various
    // operators and logical connectors.

    // Placeholder for compiled filter
    let compiledFilter = wixData.filter();

    // Simplified parsing logic for the example
    const parts = condition_string.split(' AND ');
    parts.forEach(part => {
        if (part.includes('=')) {
            const [field, value] = part.split('=').map(s => s.trim().replace(/"/g, ''));
            compiledFilter = compiledFilter.eq(field, value);
        } else if (part.includes('>')) {
            const [field, value] = part.split('>').map(s => s.trim());
            compiledFilter = compiledFilter.gt(field, Number(value));
        }
    });

    return compiledFilter;
  }
}
```

### Summary of the New Architecture

With this design, `tissdb-lite` evolves from an in-memory database into a powerful **TissLang query engine for Wix**. It provides a high-level, domain-specific language for data interaction, while leveraging the power and persistence of Wix's native data collections. This architecture successfully integrates TissLang with Wix Data, providing a seamless experience for developers working in the Tiss ecosystem.
