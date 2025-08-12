# TissDB Graph Explorer: Design Document

## 1. Vision

The TissDB Graph Explorer is a visual, interactive tool designed to allow users to "think in graphs." It moves beyond tabular data and charts to represent the underlying relationships within the TissDB data model directly. Users can navigate the 3D graph network, discover hidden connections, and understand the structure of their data in a way that is impossible with traditional query tools.

## 2. Key Features

### 2.1. Interactive Canvas
*   **Navigation**: The user can pan, zoom, and rotate the graph visualization.
*   **Layouts**: The user can switch between different layout algorithms, such as force-directed (for organic clustering) and hierarchical (for tree-like structures).
*   **Node Selection**: Users can click on nodes and edges to select them, making them the focus of the inspector panel.

### 2.2. Data-Driven Rendering
*   **Node Style**: Nodes representing XML documents will have a different shape or color from nodes representing CSV rows.
*   **Edge Style**: The thickness or color of an edge can represent the strength or type of a relationship.
*   **Labels**: Node labels will be displayed, with intelligent truncation and culling at high zoom levels to prevent clutter.

### 2.3. Inspector Panel
*   When a node or edge is selected, a side panel will display its full data payload (e.g., the full XML document or CSV row) and metadata.
*   The panel will include action buttons, such as "Show Neighbors" or "Find Path to...".

### 2.4. Query-Driven Exploration
*   The initial view of the graph can be populated by a TissQL query. For example, a user could start with the result of a `JOIN` query to see the initial set of connected nodes.
*   A search bar will allow users to find and focus on specific nodes by their ID or a field value.

### 2.5. Lazy Loading
*   To handle massive graphs, the explorer will not load the entire graph at once. It will initially show a subset of nodes.
*   As the user clicks a node and requests to see its neighbors, the explorer will make background API calls to fetch and render the connected nodes on demand.

### 2.6. Analytical Overlays
*   The explorer will have the ability to run graph algorithms on the visualized data and display the results as a visual overlay.
*   **Examples**:
    *   **Community Detection**: Color-code nodes based on their cluster or community.
    *   **Centrality Analysis**: Increase the size of nodes based on their centrality score (e.g., PageRank) to highlight influential nodes.

## 3. Technical Architecture

*   **Frontend**: A web-based application built using a powerful WebGL rendering library like **Three.js** or a dedicated graph visualization library like **D3.js** or **Cytoscape.js**.
*   **Backend (TissDB API)**: The TissDB server will expose new API endpoints optimized for graph exploration.
    *   `POST /_graph/query`: Executes a TissQL query and returns a list of nodes and edges that match. The response will be in a standard graph format like JSON Graph Format.
    *   `GET /_graph/node/<id>`: Retrieves a single node's data.
    *   `GET /_graph/node/<id>/neighbors`: Retrieves the immediate neighbors of a given node (for lazy loading).
    *   `POST /_graph/path`: An endpoint to run a pathfinding algorithm between two specified nodes.

## 4. User Workflow Example

1.  An analyst wants to investigate potential fraud.
2.  She starts by running a TissQL query in the Graph Explorer: `SELECT * FROM users u JOIN transactions t ON u.id = t.user_id WHERE t.is_flagged = true`.
3.  The explorer renders a graph of all users who have made flagged transactions.
4.  She notices two users are connected to the same suspicious shipping address. She clicks on the address node.
5.  In the inspector panel, she clicks "Show Neighbors" to see what other transactions or users are linked to that address, visually uncovering a fraud ring.