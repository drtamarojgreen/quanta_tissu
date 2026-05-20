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
### [2026-05-20 17:41:20] Fix: TokenDataset undefined reference in integration compilation
- Symptom: C++ Trainer Compilation failed with undefined reference to TokenDataset.
- Cause: 'quanta_tissu/tisslm/program/training/dataset.cpp' was missing from the compilation command in 'run_integration_api.py'.
- Fix: Added the missing source file to 'compile_cmd' and fixed a 'NameError: os' in 'run_integration_api.py'.
- Verification: 'python3 run_integration_api.py' successfully compiles and runs the C++ trainer (Step 5 & 6).
