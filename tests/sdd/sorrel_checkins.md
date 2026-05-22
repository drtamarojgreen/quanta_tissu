# Sorrel Checkins - Process Management Unified Orchestrator

## 2023-10-27 - Initial Baseline Capture
- **Symptom**: Analyzer module uses local `setInterval` and local state for polling logs/status. Switching tabs or reloading loses this local state, leading to "lost process" visibility even if the backend process is still running.
- **Symptom**: `task_manager.py` only returns the last 50 lines of logs, leading to gaps if polling is interrupted.
- **Symptom**: No unified `task_id` for analyzer vs generic tasks.
- **Constraint**: Java Orchestrator exists but currently only proxies back to Python for task listing.
- **Goal**: Move canonical lifecycle ownership to Java, provide cursor-based logs, and unified API.

## 2024-05-20 - C++ Trainer Compilation Failure
- **Symptom**: `run_integration_api.py` fails at step [5/8] with "undefined reference to TissLM::Training::TokenDataset::TokenDataset".
- **Root Cause**: `quanta_tissu/tisslm/program/training/dataset.cpp` is missing from the `g++` compilation command in `run_integration_api.py`.
- **Goal**: Fix the compilation command to include the missing source file and verify full integration.
- **Restrictions**:
  - **Tooling**: Must use `g++` with `-std=c++17`.
  - **Dependencies**: All TissLM program source files used in `train_model.cpp` must be explicitly listed in the compilation command.
  - **Minimalism**: The compilation command must only include necessary files and flags to achieve a successful build.
