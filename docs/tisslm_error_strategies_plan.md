# TissLM Error Handling Strategy

This document outlines a revised error handling strategy for the TissLM execution engine to provide more robust and granular error management, particularly during automated testing.

## 1. Core Problem

The current error handling mechanism is too broad. It catches generic exceptions and halts execution immediately (`state.is_halted = True`). This makes it difficult to run comprehensive test suites, as a single failure in one test case stops the entire process. It also fails to distinguish between critical system-level failures and recoverable, logic-level errors within a TissLang script.

## 2. Proposed Solution: Differentiated Error Types

To address this, we will introduce a new exception hierarchy to differentiate between two fundamental types of errors: **System Errors** and **Model Errors**.

### 2.1. System Errors (`TissSystemError`)

- **Definition:** Errors that originate from the execution environment, filesystem, or a violation of core security/operational constraints. These are generally not recoverable from within the script's logic.
- **Examples:**
  - `IOError` when writing or reading a file.
  - A required binary (like an external tool) is not found.
  - An attempt to access a file outside the designated project root (`TissSecurityError`).
- **Behavior:** When a `TissSystemError` is caught, the execution engine **must halt**. This is the correct behavior as the environment is unstable or compromised, and further execution is unreliable.

### 2.2. Model Errors (`TissModelError`)

- **Definition:** Errors that originate from the TissLang script's own logic or its interaction with the data. These are specific to the logic being tested and should not necessarily stop other, unrelated tests from running.
- **Examples:**
  - A failed `ASSERT` condition.
  - A `RUN` command that executes a script, which then returns a non-zero exit code, indicating a logical failure in that script.
  - Invalid parameters passed to a command that are logically incorrect (but not a syntax error).
- **Behavior:** When a `TissModelError` is caught, the execution engine should:
  1. Log the error with its details (command, line number, error message).
  2. Mark the specific step or command as "FAILED" in the execution log.
  3. **Continue execution** with the next command or step in the script. `state.is_halted` will **not** be set to `True`.

## 3. Implementation Plan

1.  **Create New Exception Classes:** In `quanta_tissu/tisslm/parser/errors.py`, define:
    - `TissError(Exception)`: A new base class for all TissLM errors.
    - `TissSystemError(TissError)`: For system-level failures.
    - `TissModelError(TissError)`: For script/logic-level failures.

2.  **Refactor Execution Engine:** In `quanta_tissu/tisslm/core/execution_engine.py`:
    - Modify the `_execute_command` method's `try...except` block to catch `TissSystemError` and `TissModelError` separately.
    - Update tool implementations (`run_command`, `assert_condition`, etc.) to raise the appropriate new exception type based on the nature of the failure. For instance, a failed assertion will raise a `TissModelError`. An `IOError` during `write_file` will raise a `TissSystemError`.

This approach will make the TissLM engine more resilient for testing, allowing a full suite to run to completion while still capturing detailed information about logical errors in the models or scripts under test.
