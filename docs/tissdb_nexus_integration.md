# TissDB and Nexus Flow Integration Guide

This document outlines how to integrate the `TissDB` database with the `nexus_flow` graph visualization tool. It covers the data structure, how to create and populate collections, and strategies for creating a more dynamic, real-time viewing experience.

## 1. Data Structure and Collection Setup

The `nexus_flow` viewer expects graph data to be structured as a single JSON document. This document contains two main arrays: `nodes` and `edges`.

### Graph Document Schema

Here is an example of a valid graph document:

```json
{
  "graph_name": "My First Graph",
  "nodes": [
    { "id": 1, "label": "Hello" },
    { "id": 2, "label": "World" }
  ],
  "edges": [
    { "from": 1, "to": 2 }
  ]
}
```

- **`nodes`**: An array of node objects.
  - `id`: A unique integer identifier for the node.
  - `label`: The text to be displayed for the node.
- **`edges`**: An array of edge objects that connect the nodes.
  - `from`: The `id` of the starting node.
  - `to`: The `id` of the ending node.

### Creating a Collection in TissDB

To store these graph documents, you first need to create a collection in TissDB. Using the TissDB REST API, you can create a collection named `nexus_graphs` with a simple `PUT` request.

**Example using `curl`:**
```bash
curl -X PUT http://localhost:8080/nexus_graphs
```
This command creates an empty collection ready to store your graph documents.

### Inserting a Graph Document

Once the collection is created, you can insert graph documents into it using a `POST` request.

**Example using `curl`:**
```bash
curl -X POST http://localhost:8080/nexus_graphs \
-H "Content-Type: application/json" \
-d '{
  "graph_name": "My First Graph",
  "nodes": [
    { "id": 1, "label": "Hello" },
    { "id": 2, "label": "World" }
  ],
  "edges": [
    { "from": 1, "to": 2 }
  ]
}'
```
TissDB will automatically assign a unique ID to this document.

## 2. Automating Data Creation

Manually creating graph data can be tedious. Here are some strategies to automate this process.

### Option A: Adapt the Python Generation Script

The repository includes a Python script that uses a language model to generate graph data from a text prompt: `quanta_tissu/tisslm/generate_graph_from_prompt.py`.

Currently, this script prints the JSON output to the console. It could be easily modified to send this output directly to the TissDB REST API.

**Proposed Modification (`generate_graph_from_prompt.py`):**

```python
# ... (existing code to generate the graph_json) ...

import requests
import json
import sys

# Instead of printing, post the data to TissDB
try:
    # The generated JSON is a string, so we need to parse it first
    graph_data = json.loads(graph_json)

    response = requests.post(
        "http://localhost:8080/nexus_graphs",
        json=graph_data
    )
    response.raise_for_status() # Raise an exception for bad status codes
    print("Successfully saved graph to TissDB.")

except requests.exceptions.RequestException as e:
    print(f"Error saving graph to TissDB: {e}", file=sys.stderr)
except json.JSONDecodeError:
    print("Error: The generated output was not valid JSON.", file=sys.stderr)

```

### Option B: Create a Dedicated Data Seeder

For bulk imports, you could create a separate "seeder" script. This script could read data from other formats (e.g., CSV, GraphViz DOT files, XML) and convert it into the required JSON structure before inserting it into TissDB.

## 3. Dynamic Viewer Updates

The current `nexus_flow` application loads graph data from TissDB only once when the user selects the menu option. To make the experience more dynamic, the viewer could be updated to reflect changes in the database in real-time.

### Strategy 1: Polling

The simplest approach is **short polling**. The `nexus_flow` application could be modified to re-fetch the graph data from the TissDB API at a regular interval (e.g., every 5 seconds).

**Implementation Sketch (`graph_logic.cpp`):**

In `runTissDBWorkflow`, instead of loading once, a loop could be implemented.

```cpp
void GraphLogic::runTissDBWorkflow() {
    // This would be in a loop with a sleep interval
    // while(!_kbhit()) { // Loop until a key is pressed
        // 1. Make an HTTP GET request to http://localhost:8080/_query
        //    with body {"query": "SELECT * FROM nexus_graphs"}
        // 2. Parse the resulting JSON array of graph documents.
        // 3. For each graph, clear the canvas and call animateGraph(graph).
        // 4. Sleep(5000); // Wait for 5 seconds
    // }
}
```
**Pros:** Easy to implement.
**Cons:** Can be inefficient and may result in a noticeable delay between a data change and the viewer update.

### Strategy 2: WebSockets (Future Enhancement)

A more advanced and efficient solution would be to use **WebSockets**. This would require an enhancement to the TissDB API server.

1.  **TissDB Enhancement**: The TissDB server could expose a WebSocket endpoint (e.g., `/ws/nexus_graphs`).
2.  **Nexus Flow Client**: The `nexus_flow` application would connect to this endpoint.
3.  **Real-time Push**: When a graph document is updated, created, or deleted in the `nexus_graphs` collection, the TissDB server would push a notification with the new data over the WebSocket connection to all connected clients.

This approach provides instantaneous updates and is much more efficient than continuous polling.
