# TissDB Dashboard Builder: Design Document

## 1. Vision

The TissDB Dashboard Builder is a web-based, drag-and-drop tool designed to empower business analysts, team leads, and executives to create, customize, and share interactive dashboards without writing any code. It bridges the gap between the powerful TissQL query language and the need for accessible, at-a-glance business insights. The primary goal is to democratize data access, allowing non-technical users to explore and visualize data from TissDB in a self-service manner.

## 2. Core Features

### 2.1. Interactive Canvas
*   **Grid-Based Layout**: A responsive grid system where users can drag, drop, and resize visualization widgets.
*   **Widget Library**: A sidebar containing all available visualization types that can be dragged onto the canvas.
*   **Dashboard Properties**: Settings for the entire dashboard, such as auto-refresh interval and global theme (light/dark).

### 2.2. Visualization Widgets
A comprehensive library of standard charting widgets will be available:
*   **KPI / Single Number**: For displaying key metrics (e.g., "Total Revenue," "Active Users").
*   **Tables**: For displaying raw, tabular data with sorting and pagination.
*   **Line Charts**: For time-series data.
*   **Bar Charts**: For comparing categorical data.
*   **Pie/Donut Charts**: For showing proportions.
*   **Scatter Plots**: For visualizing relationships between two numerical variables.
*   **Geographical Maps**: For plotting data with location information.

### 2.3. Data Source Configuration
Each widget will be linked to a TissDB data source.
*   **Direct TissQL Query**: Users can write or paste a TissQL query directly into a widget's configuration. The UI will provide basic validation.
*   **Saved Queries**: Users can select from a list of pre-approved, saved queries created in the Query Workbench. This promotes reusability and governance.
*   **Materialized Views**: Users can select a materialized view as a data source for high-performance dashboards.

### 2.4. Interactivity
*   **Dashboard-Level Filters**: Users can add filter widgets (e.g., a date range picker, a dropdown for product categories) to the canvas. These filters will apply to all relevant widgets on the dashboard simultaneously.
*   **Drill-Downs**: For hierarchical data, users can click on a segment of a chart (e.g., a bar in a bar chart) to drill down and see a more detailed view.
*   **Cross-Filtering**: Clicking on a data point in one chart can filter the data shown in other charts on the dashboard.

### 2.5. Sharing and Exporting
*   **Secure Sharing**: Generate unique, read-only URLs for sharing dashboards with other users.
*   **Embedding**: Provide HTML snippets to embed dashboards in other web applications or wikis.
*   **Scheduled Exports**: Configure dashboards to be automatically exported as a PDF or CSV and emailed to a list of recipients on a schedule (e.g., daily, weekly).

## 3. Technical Architecture

*   **Frontend**: A single-page application (SPA) built with a modern JavaScript framework like **React** or **Vue.js**.
    *   **State Management**: A robust state management library (e.g., Redux, Pinia) to manage the state of dashboards, widgets, and data.
    *   **Charting Library**: A powerful and flexible charting library like **D3.js**, **ECharts**, or **Chart.js**.
    *   **Drag-and-Drop**: A library like `react-grid-layout` to handle the interactive canvas.
*   **Backend (TissDB API)**: The Dashboard Builder will interact with the existing TissDB REST API.
    *   It will primarily use the `POST /<collection>/_query` endpoint to fetch data for visualizations.
    *   It will need new administrative endpoints in TissDB for managing saved dashboards and user permissions.
        *   `POST /_bi/dashboards`: Save a new dashboard's configuration (widget layout, queries, etc.).
        *   `GET /_bi/dashboards/<id>`: Retrieve a dashboard's configuration.
        *   `PUT /_bi/dashboards/<id>`: Update an existing dashboard.

## 4. User Workflow Example

1.  A business analyst wants to create a dashboard to monitor weekly sales performance.
2.  She opens the Dashboard Builder and creates a new, blank dashboard.
3.  She drags a "KPI" widget onto the canvas. In its configuration, she writes a TissQL query: `SELECT SUM(price) FROM orders WHERE order_date >= 'this_week_start'`. She titles the widget "Weekly Revenue."
4.  She drags a "Line Chart" widget onto the canvas. She uses a saved query called `daily_sales_last_30_days` as its data source.
5.  She adds a "Table" widget and configures it with the query `SELECT product_name, SUM(quantity) as units_sold FROM orders GROUP BY product_name ORDER BY units_sold DESC LIMIT 10` to show the top 10 selling products.
6.  She adds a "Date Range" filter widget to the top of the dashboard, linking it to the "Weekly Revenue" and "Top 10 Products" widgets.
7.  She saves the dashboard and shares the read-only link with her team manager.
8.  The manager opens the link and uses the date range filter to view the performance for the previous month.