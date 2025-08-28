# Nexus Flow: Proposed Enhancements

This document outlines 100 potential enhancements for the `nexus_flow` application, focusing on three key areas: integration with TissDB, integration with TissLM, and the development of a native graph analytics suite. These enhancements are designed to be implemented without requiring additional external libraries.

## 1. TissDB Integration Enhancements

This category focuses on building a robust persistence layer for `nexus_flow` using the TissDB database.

### Core CRUD and Configuration
1.  **Implement Graph List Fetch**: Implement `loadGraphsFromTissDB` to fetch a list of all graph IDs and names from a 'graphs' collection.
2.  **Implement Full Graph Load**: Implement loading a single graph's full JSON representation from TissDB by its ID.
3.  **Implement Graph Save**: Implement saving a new graph to TissDB, generating a new unique ID on the server or client side.
4.  **Implement Graph Update**: Implement updating and overwriting an existing graph in TissDB.
5.  **Implement Graph Deletion**: Implement deleting a graph from TissDB via the UI.
6.  **Instantiate TissDB Client**: Add a `TissuClient` instance as a member of the `GraphLogic` class to manage the connection.
7.  **Externalize Configuration**: Create and parse a `nexus_flow.conf` file for TissDB connection details (host, port) instead of hardcoding them.
8.  **Graceful Connection Handling**: Implement robust error handling for TissDB connection failures on application startup.
9.  **"Save As" Feature**: Add a "Save As..." feature in the UI to save a modified graph under a new name or ID in TissDB.
10. **Rename Graph Feature**: Add a menu option to rename a graph, updating its metadata in TissDB.

### Data Modeling and Advanced Querying
11. **Rich Metadata Storage**: Store extended graph metadata (e.g., name, description, creation_date, last_modified) in the TissDB document.
12. **Search by Name**: Implement TissQL queries to search for graphs by name.
13. **Search by Node Label**: Implement TissQL queries to find all graphs that contain a specific node label.
14. **Index Graph Names**: Create a secondary index in TissDB on the graph `name` field to accelerate searches.
15. **Preserve Layout**: Store node positions (`x`, `y`, `z`) in TissDB so that the visual layout is preserved across sessions.
16. **Graph Versioning**: Implement versioning by saving changes as a new version in a 'graph_history' collection instead of overwriting.
17. **Load Specific Version**: Implement a function to load a specific, older version of a graph.
18. **Revert to Version**: Implement a function to "revert" the primary graph document to a previous version.
19. **Store Visual Attributes**: Extend the data model to store visual attributes like node color or edge thickness in TissDB.
20. **Load Visual Attributes**: Implement the logic to load and apply stored visual attributes during rendering.
21. **Partial Fetch (Nodes)**: Implement a query to fetch only the nodes of a graph, excluding the edges, for lightweight previews.
22. **Partial Fetch (Edges)**: Implement a query to fetch only the edges of a graph.
23. **Transactional Updates**: Use TissDB's transactional capabilities (`beginTransaction`, `commit`, `rollback`) for complex updates.
24. **Bulk Import**: Implement a feature to import multiple graph JSON files from a local directory into TissDB.
25. **Bulk Export**: Implement a feature to export all graphs from TissDB into a directory of JSON files.
26. **In-Memory Caching**: Add a client-side cache for graph data to avoid redundant TissDB queries.
27. **Cache Invalidation**: Implement a "Refresh" option in the UI to invalidate the cache and reload all data from TissDB.
28. **Store Analytics Results**: Store the results of native graph analytics (e.g., centrality scores) back into the graph's document in TissDB.
29. **Optimistic Locking**: Implement optimistic locking using a revision field (`_rev`) to prevent conflicts from concurrent modifications.
30. **Comprehensive Logging**: Add detailed logging for all TissDB interactions using the `StdLogger` from `tissu_sinew`.

## 2. TissLM Integration Enhancements

This category focuses on making the interaction between `nexus_flow` and the `tisslm` Python model more robust, feature-rich, and performant.

### Robustness and Configuration
31. **Configurable Script Path**: Make the path to `generate_graph_from_prompt.py` configurable in `nexus_flow.conf`.
32. **Configurable Python Path**: Make the `python` executable path itself configurable.
33. **Control Node Count**: Pass a desired number of nodes as a command-line argument to the TissLM script.
34. **Control Graph Density**: Pass a desired graph density (edge-to-node ratio) as an argument to the TissLM script.
35. **Add Command Timeout**: Add a timeout to the `popen` call to prevent `nexus_flow` from hanging on a stalled TissLM script.
36. **Improve JSON Extraction**: In the Python script, use a more robust method (e.g., regex) to extract the JSON block from the model's output.
37. **Structured Logging (Python)**: Implement structured (JSON) logging in the Python script for easier parsing and debugging.
38. **Parse Python Logs (C++)**: In `nexus_flow`, parse the structured logs from the Python script to provide better error feedback to the user.

### New LLM-Powered Features
39. **Graph Modification**: Implement a workflow to send an existing graph's JSON *to* the TissLM script with a modification prompt (e.g., "add a node named 'D' connected to 'A'").
40. **Implement Modification Logic (Python)**: Add logic to the Python script to parse an input graph, modify it per the prompt, and return the new JSON.
41. **Graph Summarization**: Create a feature to send a graph's JSON to TissLM and request a natural language summary of its structure.
42. **Automated Node Renaming**: Implement a feature to rename all nodes based on a theme (e.g., "rename all nodes to be named after planets").
43. **Automated Clustering**: Ask TissLM to add a `cluster_id` attribute to each node based on its label or connectivity.
44. **Graph Q&A**: Implement a workflow to send graph JSON and a natural language question (e.g., "Which node is most central?").
45. **Display LLM Answers**: Create a UI component in `nexus_flow` to display the natural language answers from TissLM.
46. **Pathfinding with LLM**: Use TissLM's reasoning to find a "semantic" path between two nodes.
47. **LLM-Suggested Layouts**: Ask TissLM to suggest meaningful `x, y, z` coordinates for nodes based on the graph's structure.
48. **Implement Layout Logic (Python)**: Update the Python script to be able to return coordinates in the node objects.
49. **Apply LLM Layouts**: Update `nexus_flow` to use LLM-generated coordinates instead of random ones.

### Performance and Architecture
50. **Long-Running Server Process**: Convert the TissLM Python script into a long-running server process that listens on a local socket to keep the model loaded in memory.
51. **Socket Communication**: Modify `nexus_flow` to communicate with the TissLM server via a socket instead of `popen`.
52. **Define RPC Protocol**: Define a simple JSON-RPC protocol for the C++/Python socket communication.
53. **Connection Management (C++)**: Implement connection management for the TissLM server socket in `nexus_flow`.
54. **Health Check Endpoint**: Add a "health check" endpoint to the TissLM server so `nexus_flow` can verify it's running.
55. **Server Management UI**: Add options in the `nexus_flow` menu to start or stop the TissLM server process.
56. **Batch Processing**: Add support for sending multiple prompts in a single batch request to the TissLM server.
57. **Request Cancellation**: Implement a mechanism to cancel a long-running TissLM request from the `nexus_flow` UI.
58. **Configurable Model Loading (Python)**: Allow the Python server to load different model files based on a startup argument.
59. **LLM Personas**: Allow `nexus_flow` to select different system prompts to use as "personas" for the LLM.
60. **Abstracted C++ Client**: Create a C++ class (`TissLM_Client`) that abstracts away the details of the socket communication.
61. **Simple Authentication**: Add a simple, optional API key/token for the socket server to prevent unauthorized use.
62. **Ensure UTF-8 Compliance**: Ensure UTF-8 encoding is handled correctly when passing string data between C++ and Python.

## 3. Native Graph Analytics Enhancements

This category focuses on implementing a suite of graph analysis algorithms directly in C++ within `nexus_flow` for high performance.

### Core Graph Properties
63. **Node/Edge Count**: Implement a function to calculate the total number of nodes and edges.
64. **Node Degree**: Implement a function to calculate the degree of any given node.
65. **Average Degree**: Implement a function to calculate the average degree of the entire graph.
66. **Find Isolated Nodes**: Implement a function to find all nodes with a degree of 0.
67. **Graph Density**: Implement a function to calculate the density of the graph.
68. **Statistics UI**: Create a "Graph Statistics" panel in the UI to display these computed properties.
69. **Directedness Check**: Implement a function to check if the graph is directed or undirected.
70. **Neighbor Listing**: Implement a function to efficiently retrieve all neighbors of a given node.

### Pathfinding and Connectivity
71. **Breadth-First Search (BFS)**: Implement BFS to find the shortest path between two nodes (in edge count).
72. **Dijkstra's Algorithm**: Implement Dijkstra's algorithm for weighted shortest path (requires adding a 'weight' attribute to edges).
73. **Connectivity Check**: Implement a function to check if the entire graph is connected.
74. **Connected Components**: Implement an algorithm to find all disjoint connected components.
75. **Path Visualization**: Add a UI workflow to select two nodes and visualize the shortest path between them.
76. **A* Search**: Implement the A* search algorithm for more efficient pathfinding (requires a heuristic function).
77. **Cycle Detection**: Implement an algorithm to detect the presence of cycles in the graph.
78. **List All Cycles**: Implement a function to list all elementary cycles (for small graphs).
79. **All-Pairs Shortest Paths**: Implement the Floyd-Warshall algorithm to compute all-pairs shortest paths.
80. **Query Shortest Paths**: Allow the user to query the pre-computed all-pairs shortest path matrix.

### Centrality Measures
81. **Degree Centrality**: Implement Degree Centrality calculation for all nodes.
82. **Closeness Centrality**: Implement Closeness Centrality calculation for all nodes.
83. **Betweenness Centrality**: Implement a basic version of Betweenness Centrality calculation.
84. **Visualize Centrality**: Add a UI mode to visualize centrality by scaling node size based on their score.
85. **Eigenvector Centrality**: Implement Eigenvector Centrality calculation.
86. **Node Ranking**: Add a "Rank Nodes" feature that lists all nodes sorted by a selected centrality metric.

### Advanced Algorithms and Data Structures
87. **Minimum Spanning Tree (MST)**: Implement Prim's or Kruskal's algorithm to find the MST of a weighted graph.
88. **Visualize MST**: Add a UI option to display only the MST of a graph.
89. **Topological Sort**: Implement a topological sort for Directed Acyclic Graphs (DAGs).
90. **DAG Check**: Add a function to verify if a graph is a DAG before attempting a topological sort.
91. **Adjacency List Representation**: Implement an adjacency list representation internally for more efficient algorithm performance.
92. **Adjacency Matrix Representation**: Implement an adjacency matrix representation.
93. **Representation Switching**: Allow the user to switch the internal graph representation to compare algorithm performance.
94. **Neighborhood Query**: Implement a function to find all nodes within N hops of a selected node.
95. **Community Detection**: Implement a simple community detection algorithm (e.g., Label Propagation).
96. **Visualize Communities**: Add a UI mode to color nodes based on their detected community ID.
97. **Create Subgraph**: Implement a function to create a new graph object from a user's selection of nodes.
98. **Merge Graphs**: Implement a function to merge two separate graph objects into one.
99. **Isomorphism Check**: Implement a basic graph isomorphism check (for small graphs).
100. **Standard Format Export**: Implement a feature to export the graph data as a string in a standard format like GraphML or GEXF.
