# QuantaTissu Framework Architecture: The Greenhouse Ecosystem

## 1. Introduction
This document describes the architectural transition of the QuantaTissu Web Platform from its current "piecemeal" modular implementation to a unified, professional orchestration framework. In the vision of the **Artsy Directora**, we view this framework as a **Greenhouse Ecosystem**:
*   **The Backend is the Root System**: A deep, interconnected network of managers that provides nutrients (data) and support (process lifecycle) to the rest of the organism.
*   **The Frontend is the Visible Foliage**: A vibrant, high-fidelity interface that allows users to interact with and observe the internal mental states being cultivated.

The goal is to eliminate state fragmentation, ensure UI/UX consistency, and provide robust process lifecycle management, ensuring every element has conceptual justification and contributes to the narrative of mental development.

---

## 2. Risks of Current Modular Design (The Fragmented Soil)
The current architecture evolved incrementally, resulting in several critical risks:
*   **State Fragmentation**: Modules (DB, Tests, Analyzer) manage their own internal states. Navigating between tabs often breaks polling loops or loses visual context (e.g., the "Stop" button disappearing).
*   **UI Inconsistency**: Design choices (colors, spacing, typography) are hardcoded within individual JavaScript modules, leading to a "stitched-together" aesthetic and poor legibility.
*   **Process Orphanage**: Background processes (TissDB, Test Runner) are handled in-memory within specific backend handlers. A server restart or request timeout can leave "ghost" processes running without UI visibility.
*   **Template Bloat**: Large strings of HTML embedded in JavaScript (`UIModule.views`) make the codebase difficult to audit, test, and refactor.

---

## 3. Change Impact & Feasibility
### Change Impact
*   **Frontend (The Foliage)**: Significant. Requires moving from independent module variables to a centralized `AppState` object and standardizing rendering via a shared component library.
*   **Backend (The Root System)**: Moderate. Requires a unified `TaskManager` class to replace fragmented process management in `testing_handler.py` and `db_lifecycle_handler.py`.
*   **Configuration**: High. Moves from individual file-based configuration to a structured JSON-driven system accessible by both C++ and Python components.

### Feasibility
The refactor is highly feasible because the framework is intentionally dependency-free. Leveraging vanilla JavaScript and Python's built-in libraries allows for a clean transition without the risk of version conflicts or third-party vulnerabilities.

---

## 4. Feature & Tab Matrix

| Tab | Current State | Future State | Implementation Risk |
| :--- | :--- | :--- | :--- |
| **Dashboard** | Static status display; fragmented stats fetch. | Real-time system health monitor with centralized state sync. | Low |
| **Data Explorer** | Direct TissQL query interface; simple results div. | Unified query console with pagination and schema visualization. | Medium |
| **Model Playground** | Basic inference trigger; hardcoded params. | Parameterized generation studio with RAG context visibility. | Low |
| **Analytics** | Isolated chart rendering; static signal engine. | Integrated performance dashboard with shared telemetry data. | Medium |
| **TissLang** | Simple text-to-JSON orchestrator. | Syntax-highlighted editor with step-by-step execution tracking. | Medium |
| **Analyzer** | Independent IPC monitor; inconsistent log streaming. | Centralized RMA controller integrated into task manager. | High (IPC complexity) |
| **Model Training** | Basic param updates; simulated progress bar. | Real-time training telemetry with weight-dist distribution graphs. | Medium |
| **DB Management** | Custom g++ build logic; basic lifecycle buttons. | Comprehensive server control suite with logs and versioning. | Medium |
| **Admin** | Fragmented administrative tasks (Migration, etc). | Unified system settings and collection orchestration. | Low |
| **Tests** | Background task with polling; recently improved UI. | Multi-process test suite with real-time log streaming and export. | Low |
| **Nexus Flow** | Independent canvas; isolated logic. | System-wide topology visualizer powered by AppState telemetry. | Medium |
| **Help** | Hardcoded documentation strings. | Searchable dynamic documentation hub. | Low |

---

## 5. Implementation Timeline

| Phase | Title | Focus Area | Timeline |
| :--- | :--- | :--- | :--- |
| **Phase 1** | **Foundation** | Implement centralized `AppState` (FE) and `TaskManager` (BE). | Days 1-2 |
| **Phase 2** | **UI Standardization** | CSS Variable refactor, high-contrast theme, shared UI components. | Day 3 |
| **Phase 3** | **Module Migration** | Migrating all 12 tabs to the new reactive state pattern. | Days 4-7 |
| **Phase 4** | **System Audit** | Stress testing process lifecycle and final UX polish. | Day 8 |

---

## 6. Success Criteria
*   Zero "is not a function" errors during tab navigation.
*   Consistent "Stop" button visibility for all background processes.
*   Centralized logging accessible from any part of the application.
*   High-contrast, professional design language applied system-wide.
