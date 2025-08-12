# TissDB Reporting & Alerting Service: Design Document

## 1. Vision

The TissDB Reporting & Alerting Service is a backend system designed to automate data delivery and enable proactive monitoring of data within TissDB. It moves beyond on-demand querying to a "push" model, where insights and critical alerts are delivered automatically to stakeholders. This service is crucial for operationalizing the data insights discovered in the Query Workbench and visualized in the Dashboard Builder, turning passive data into active intelligence.

## 2. Core Components

The service is composed of two primary, tightly integrated functionalities: Reporting and Alerting.

### 2.1. Reporting Engine
*   **Purpose**: To schedule and deliver the results of TissQL queries in various formats.
*   **Features**:
    *   **Report Configuration**: A web UI where users can define reports by specifying a TissQL query, a delivery schedule (using cron syntax), a format (CSV, JSON, PDF), and one or more delivery channels.
    *   **Scheduling**: A robust, cron-based scheduling engine that reliably executes report queries at their specified times.
    *   **Formatting**: A rendering engine that can format the query results into the chosen format. The PDF renderer will generate a simple, clean table of the data.
    *   **Delivery**: A dispatch system that can send the formatted report to multiple channels.

### 2.2. Alerting Engine
*   **Purpose**: To continuously monitor data and trigger notifications when specific conditions are met.
*   **Features**:
    *   **Alert Configuration**: A web UI for defining alerts. An alert consists of a TissQL query, a check frequency (e.g., every 5 minutes), a trigger condition, and a notification channel.
    *   **Trigger Conditions**: Simple, configurable logic applied to the query result (e.g., `result > 10`, `result CONTAINS "ERROR"`, `result IS_EMPTY`).
    *   **State Management**: The engine must be stateful to avoid sending repeated notifications for the same ongoing condition (e.g., it should only alert once when stock first drops below 10, and then again when the condition is resolved and re-triggered).
    *   **Notification**: A dispatch system to send concise alert messages to the appropriate channels.

### 2.3. Supported Channels
Both engines will support a common set of delivery/notification channels:
*   **Email**: Send reports as attachments or alerts as formatted emails.
*   **Slack**: Post messages to a specified Slack channel via a webhook.
*   **Generic Webhook**: Send a POST request with a JSON payload to any specified URL, allowing for integration with custom systems.

## 3. Technical Architecture

*   **Frontend**: The configuration UI will be a dedicated section within the main TissDB BI Suite web application.
*   **Backend**: A standalone, long-running daemon process (e.g., written in Python or Go).
    *   **Scheduler**: A library like `APScheduler` (Python) or a similar cron implementation will manage the execution of report and alert queries.
    *   **Worker Pool**: A pool of worker threads/processes to execute the TissQL queries against the TissDB API, preventing a single long-running query from blocking others.
*   **API Endpoints**: The service will require new administrative endpoints in TissDB to store its configurations.
    *   `POST /_bi/reports`: Create a new report configuration.
    *   `GET /_bi/reports`: List all configured reports.
    *   `PUT /_bi/reports/<id>`: Update a report configuration.
    *   `DELETE /_bi/reports/<id>`: Delete a report.
    *   (Similar CRUD endpoints for `/_bi/alerts`)

## 4. User Workflow Examples

### Reporting Workflow
1.  A sales manager wants a weekly report of all high-value orders.
2.  She navigates to the "Reporting" section of the BI suite.
3.  She creates a new report named "Weekly High-Value Orders."
4.  She enters the TissQL query: `SELECT customer_id, order_date, total FROM orders WHERE total > 5000 AND order_date >= 'last_7_days'`.
5.  She sets the format to "CSV".
6.  She sets the schedule to `0 9 * * 1` (every Monday at 9 AM).
7.  She adds the email addresses for the sales team to the delivery channel.
8.  The service now automatically runs this query every Monday morning and emails a CSV of the results to the team.

### Alerting Workflow
1.  An operations engineer needs to be notified immediately if the application starts logging errors.
2.  He navigates to the "Alerting" section.
3.  He creates a new alert named "Error Log Spike."
4.  He enters the TissQL query: `SELECT COUNT(*) FROM app_logs WHERE level = 'ERROR' AND timestamp > 'last_5_minutes'`.
5.  He sets the check frequency to "Every 5 minutes."
6.  He sets the trigger condition to `result > 0`.
7.  He configures the notification channel with the DevOps team's Slack webhook URL.
8.  The service now checks for new errors every five minutes and sends an immediate Slack notification if any are found.