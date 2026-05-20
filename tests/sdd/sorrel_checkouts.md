# Sorrel Checkouts - Process Management Unified Orchestrator

## 2023-10-27 - Unified Lifecycle Verified
- **Implementation**: Canonical process lifecycle moved to Java Orchestrator.
- **Implementation**: Unified `/api/processes` API implemented in Java and proxied via Python.
- **Implementation**: `AppState.js` now handles rehydration on tab switch and page reload.
- **Implementation**: `analyzer_module.js` and `model_module.js` migrated to unified state.
- **Verification**: `mvn compile` passed for Java components.
- **Verification**: Python proxy routing verified with `test_task_proxy.py`.
- **Verification**: Cursor-based log retrieval implemented to ensure no gaps.
- **Verification**: Executed SDD card `verify_process_lifecycle`; Result: `process_orchestrated = true`.
- **Result**: "Lost process" symptoms resolved by centralizing state in a durable backend orchestrator and reactive frontend rehydration.

## 2024-05-20 - C++ Trainer Compilation Fixed
- **Implementation**: Added `quanta_tissu/tisslm/program/training/dataset.cpp` to the `compile_cmd` in `run_integration_api.py`.
- **Implementation**: Added `import os` to `run_integration_api.py` to fix `os.path` reference in cleanup.
- **Verification**: Executed `./run_integration_api.py`; all 8 steps completed successfully, including the C++ Trainer compilation and training process.
- **Result**: Linker error "undefined reference to TissLM::Training::TokenDataset::TokenDataset" resolved.
