# TissDB Integration Strategy

## Introduction

TissDB is a purpose-built database designed to serve as a high-performance, flexible data backbone for the Quanta ecosystem. It utilizes a hybrid storage model supporting both **compressed XML** for complex, hierarchical documents and **CSV-style formats** for efficient, tabular knowledge storage.

Its core design features a two-layer storage method:
1.  An **unsecured, open knowledge layer** for sharing and integrating non-sensitive data between agents and services.
2.  A planned **secure, transactional layer** for robust, permissioned data management (to be developed later).

By combining a powerful query language (TissQL) with a novel graph-based algorithmic approach, TissDB aims to reduce data silos, simplify inter-service communication, and accelerate development cycles.

## Key Features Facilitating Integration

*   **Hybrid Data Storage:** TissDB's ability to natively handle both structured XML and tabular CSV allows services to use the best format for the job. Complex entities can be modeled in XML, while large datasets for knowledge harvesting can be efficiently stored and accessed as CSV, all within the same database.

*   **Graph-Powered TissQL Queries:** All data interactions are handled through TissQL. The query engine is built on algorithms that treat all data—whether from XML or CSV—as nodes in a 3D graph network. This allows for powerful, expressive queries that can traverse relationships between disparate data sources seamlessly.

*   **API-First Design:** TissDB is built with an API-first philosophy. All database features are exposed through a clean, well-documented RESTful API, making it easy for services written in any language or framework to connect and interact with the database.

*   **Layered Security Model:** The initial integration focuses on the unsecured knowledge layer, enabling rapid development and data sharing. This provides a "low-friction" environment for agents to collaborate. A future secure layer will add robust authentication and authorization for sensitive workloads.

## Integration Benefits

Adopting TissDB as a central data layer provides several key benefits for integrating services from different repositories:

1.  **Unified Data Access:** Services no longer need to implement bespoke clients or adapters to communicate with each other's data stores. They all communicate with a single, unified TissDB API.

3.  **Simplified Data Transformation:** TissQL can include built-in functions for transforming data into the specific formats required by different consumer services. This offloads complex data transformation logic from the application layer to the database layer.

4.  **Reduced Boilerplate Code:** By leveraging the expressive power of TissQL, developers can write less data access and manipulation code within their services, leading to cleaner, more focused application logic.

## Storage Architecture and Graph Algorithms

Conceptually, all data in TissDB is part of a unified graph.

*   **Nodes**: An XML document, or a row within a CSV file, is treated as a "node" in the graph.
*   **Edges**: Relationships, or "bonds," can be explicitly defined between nodes. This allows TissQL to traverse from a user document (XML) to their activity log (CSV), for example.

This architecture allows the query engine to perform complex joins and graph traversals that are not possible in traditional databases without significant effort. The algorithms are inspired by 3D network graphs, enabling queries that can find not just direct relationships, but also second- and third-order connections between data points.

## TissQL by Example

Consider two services in separate repositories: a `user-service` and a `product-service`.

*   The `user-service` is responsible for managing user profiles, preferences, and authentication. It stores all user data in TissDB.
*   The `product-service` is responsible for managing the product catalog and providing recommendations.

To personalize product recommendations, the `product-service` needs access to user preferences. Instead of creating a direct API dependency on the `user-service`, it can directly query TissDB using TissQL.

### Example 1: Basic Query on XML Data
// Retrieve preferences from a 'users' collection (stored as XML)
{
  "query": "SELECT preferences FROM users WHERE id = '12345'"
}

### Example 2: Aggregation on CSV Data
// Count how many times a user has viewed a product from a 'view_events' collection (stored as CSV)
{
  "query": "SELECT COUNT(*) FROM view_events WHERE user_id = '12345' AND event_type = 'product_view'"
}

### Example 3: Graph Traversal (Joining XML and CSV)
// Find product names that a user has viewed more than 5 times.
// This query traverses from the 'users' collection to the 'view_events' collection.
{
  "query": "SELECT T2.product_name, COUNT(T2.product_name) as views FROM users T1 JOIN view_events T2 ON T1.id = T2.user_id WHERE T1.id = '12345' GROUP BY T2.product_name HAVING views > 5"
}

### Example 4: Finding Second-Order Connections
// Find other users who have purchased the same products as user '12345'
{
  "query": "SELECT T2.user_id FROM purchase_history T1 JOIN purchase_history T2 ON T1.product_id = T2.product_id WHERE T1.user_id = '12345' AND T2.user_id != '12345'"
}

---

## Integration Patterns

Services can integrate with TissDB using several common patterns:

1.  **Direct API Integration:** The simplest pattern. A service performs direct CRUD operations on documents it owns. For example, the `user-service` would use `POST /users` to create a new user and `GET /users/<id>` to retrieve one.

2.  **Shared Data Hub (Asynchronous Communication):** Two or more services communicate asynchronously by reading and writing to a shared collection. For example, a `payment-service` could write a `payment_receipt` document to a `receipts` collection, which an `email-service` then reads to send a confirmation email. This decouples the services, as neither needs to be aware of the other's operational status.

3.  **Data Aggregation Layer:** A dedicated service can be created to act as an aggregator. It queries multiple collections (e.g., `users`, `orders`, `products`), performs complex joins or aggregations, and writes a new, enriched document to a separate collection (e.g., `analytics_dashboard_data`). This pre-computes complex views for read-heavy applications.

## Step-by-Step Integration Guide

This guide provides a practical walkthrough for integrating a new service with TissDB.

### 1. Define Your Data Contract
Before writing any code, identify the structure of the data your service will read or write. For example, a new `inventory-service` might define a `product` document like this:

```xml
<document id="product_123">
  <sku>HD-WIDGET-01</sku>
  <stock_level>150</stock_level>
  <warehouse_location>Aisle 4, Bay 2</warehouse_location>
  <last_updated>2023-11-05T14:30:00Z</last_updated>
</document>
```

### 2. Implement an API Client
Your service will need a client to communicate with the TissDB REST API. Here is a basic example in Python using the `requests` library.

```python
import requests
import json

TISSDB_URL = "http://tissdb.internal:8080"
API_KEY = "your-secure-api-key"

def create_product(product_data):
    headers = {"Authorization": f"Bearer {API_KEY}", "Content-Type": "application/json"}
    response = requests.post(f"{TISSDB_URL}/inventory", headers=headers, data=json.dumps(product_data))
    response.raise_for_status()
    return response.json()

def find_low_stock_products(threshold):
    headers = {"Authorization": f"Bearer {API_KEY}", "Content-Type": "application/json"}
    query = f"SELECT sku, stock_level FROM inventory WHERE stock_level < {threshold}"
    response = requests.post(f"{TISSDB_URL}/inventory/_query", headers=headers, json={"query": query})
    response.raise_for_status()
    return response.json()
```

### 3. Handle API Responses and Errors
Be prepared to handle standard HTTP status codes:
-   **`200 OK`**: Successful `GET` or `_query` request.
-   **`201 Created`**: Successful `POST` request to create a document.
-   **`400 Bad Request`**: Your request body or TissQL query is malformed.
-   **`401 Unauthorized`**: Your API key is missing or invalid.
-   **`404 Not Found`**: The requested document or collection does not exist.
-   **`500 Internal Server Error`**: An error occurred on the TissDB server.

## Security and Authentication

All communication with TissDB must be secured.

*   **Encryption in Transit**: The API requires TLS 1.2+ for all connections.
*   **Authentication**: Services must authenticate using a bearer token (API Key) in the `Authorization` header of every request.
*   **Authorization**: Access is controlled via Role-Based Access Control (RBAC). Roles are defined with granular permissions (e.g., `read`, `write`, `delete`) on a per-collection basis. Ensure your service's API key is associated with a role that has the minimum required permissions.

## Conclusion

The development of TissDB is a strategic investment in our engineering ecosystem. By providing a flexible, powerful, and centralized data integration point, TissDB will break down data barriers, reduce architectural complexity, and empower our development teams to build more cohesive and capable applications.
