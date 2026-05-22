# Unified Process Management & Analyzer Updates

## Overview
The QuantaTissu Platform has been updated to provide a durable, orchestrator-backed process lifecycle management system. This ensures that long-running tasks like model training and analyzers remain visible and controllable even when the user navigates away or refreshes the page.

## Architectural Changes

### 1. Java Orchestrator: Canonical Process Manager
- Implemented `ProcessManager.java` to handle process spawning, status tracking, and log accumulation.
- Expose `/api/orchestrator/processes` for unified lifecycle control.
- Configured to use **PostgreSQL** as the primary datastore for persistence.
- Added `@Transient` annotations to `MultipartFile` fields in entities (e.g., `Video.java`, `Lecture.java`) to prevent JPA mapping errors.

### 2. Python Backend: Orchestrator Proxy
- `TaskManager` refactored to proxy all requests to the Java Orchestrator.
- `analyzer_handler.py` updated to use the unified `TaskManager` with a canonical `analyzer` task ID.

### 3. Frontend: Unified State & Rehydration
- `AppState.tasks` is now the single source of truth for all background processes.
- Added `rehydrateTasks()` to `state.js` to fetch and resume process tracking on page load.
- `AnalyzerModule` refactored to remove local polling and bind directly to `AppState.tasks`.

## Configuration
The Java Orchestrator now expects the following environment variables for PostgreSQL connection:
- `SPRING_DATASOURCE_URL`: JDBC URL (default: `jdbc:postgresql://localhost:5432/quantatissu`)
- `SPRING_DATASOURCE_USERNAME`: (default: `postgres`)
- `SPRING_DATASOURCE_PASSWORD`: (default: `postgres`)

## Verification
- Java Unit Tests: `ProcessManagerTest.java` and `OrchestratorControllerTest.java`.
- SDD Verification Cards implemented in `tests/sorrel/sdd/cards/ProcessVerificationClass.cpp`.
- Verifies process continuity after simulated "reloads" and log integrity across re-attachments.
