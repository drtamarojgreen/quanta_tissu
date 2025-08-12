# TissDB Query Workbench: Design Document

## 1. Vision

The TissDB Query Workbench is a professional, web-based Integrated Development Environment (IDE) for data analysts, developers, and power users who interact with TissDB directly. It serves as the primary interface for writing, executing, debugging, and optimizing TissQL queries. The vision is to provide a tool that feels as powerful and intuitive as modern SQL IDEs, but is purpose-built for the unique features of TissQL and the TissDB graph data model.

## 2. Core Features

### 2.1. Multi-Tab TissQL Editor
*   **Syntax Highlighting**: Custom syntax highlighting for TissQL keywords, functions, and data types.
*   **Autocompletion**: Context-aware autocompletion for collection names, field names, and TissQL functions.
*   **Linting**: Real-time error checking and style suggestions to catch issues before a query is run.
*   **Query Formatting**: A "prettify" button to automatically format long queries according to a standard style guide.

### 2.2. Results Panel
*   **Multi-Format View**: Users can switch between different views for the query results:
    *   **Table View**: A standard, sortable, and filterable grid for tabular results.
    *   **JSON View**: A collapsible tree view for hierarchical JSON data.
    *   **XML View**: A formatted and syntax-highlighted view for XML documents.
    *   **Raw Text View**: The raw response from the API.
*   **Export Functionality**: Export query results to CSV, JSON, or XML files.

### 2.3. Query Management
*   **Query History**: A chronological list of all queries run by the user, allowing them to easily find and re-run past queries.
*   **Saved Queries/Snippets**: A personal library where users can save, name, and organize frequently used queries or snippets. These saved queries can be shared with the team and used as data sources in the Dashboard Builder.

### 2.4. Performance and Optimization Tools
*   **Execution Plan Visualizer**: Before running a query, the user can request its execution plan. The Workbench will visualize the plan as a tree, showing which indexes are being used, the order of operations (e.g., scan, filter, join), and the estimated cost of each step. This is crucial for optimizing slow queries.
*   **Query Stats**: After a query runs, the results panel will display key performance metrics, such as execution time, number of documents scanned, and data transfer size.

### 2.5. Quick Visualization
*   For tabular query results, a "Chart" tab will be available in the results panel.
*   This provides basic, non-persistent charting capabilities (bar, line, pie charts) to allow for quick visual exploration of the data without needing to build a full dashboard.

## 3. Technical Architecture

*   **Frontend**: A single-page application (SPA) built with a modern JavaScript framework (e.g., React, Vue.js).
    *   **Editor Component**: Integrate a powerful code editor component like Monaco Editor (used in VS Code) to provide a rich editing experience.
*   **Backend (TissDB API)**: The Query Workbench will interact with several TissDB API endpoints.
    *   `POST /<collection>/_query`: The primary endpoint for executing queries.
    *   `POST /<collection>/_explain`: A new endpoint that accepts a TissQL query and returns its execution plan in a structured JSON format.
    *   `GET /_schema/collections`: An endpoint to fetch a list of all collections and their fields to power the autocompletion feature.
    *   `POST /_bi/saved_queries`: New administrative endpoints for saving and retrieving user-created queries.

## 4. User Workflow Example

1.  A data analyst needs to investigate user activity.
2.  She opens the Query Workbench and starts a new query tab.
3.  The editor autocompletes the collection name as she types `SELECT * FROM user_activity`.
4.  She refines the query to `SELECT user_id, COUNT(*) as actions FROM user_activity WHERE timestamp > '24h_ago' GROUP BY user_id ORDER BY actions DESC LIMIT 20`.
5.  Before running it, she clicks the "Explain" button. The visualizer shows that the query will perform a full collection scan because `timestamp` is not indexed.
6.  She opens a new tab and runs `CREATE INDEX ON user_activity (timestamp)`.
7.  She goes back to her original query and clicks "Explain" again. The new plan shows it will use the index, which is much more efficient.
8.  She runs the query. The results appear in the table view.
9.  She clicks the "Chart" tab and selects a bar chart to quickly visualize the most active users.
10. Satisfied with the query, she clicks "Save," names it `daily_active_users`, and adds a description. This query is now available for her colleagues to use in the Dashboard Builder.