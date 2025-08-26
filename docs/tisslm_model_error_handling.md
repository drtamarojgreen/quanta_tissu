# TissLM Model Error Handling Strategy

This document outlines the strategy for improving error handling within the TissLM model and its associated testing framework. The goal is to create a more robust system that provides clear, actionable feedback by distinguishing between different types of failures.

## 1. Introduction of Custom Error Classes

To enable more precise error handling, we will introduce two new custom exception classes. These will be defined in `quanta_tissu/tisslm/parser/errors.py`.

-   `SystemError`: This exception will be used for failures related to the environment, file system, or external processes. Examples include a required binary not being found, file I/O errors, or subprocess execution failures.
-   `ModelError`: This exception will be used for failures originating from within the model's own algorithmic or numerical logic. Examples include tensor shape mismatches, numerical instability (NaN/inf values), or invalid configurations during model inference.

## 2. Decoupling Tests from the Database

To ensure tests are hermetic and do not rely on external services, all database dependencies will be removed from the testing process.

-   **BDD Test Runner (`tests/bdd_runner.py`)**: The `start_db` function will be modified to prevent it from launching the `tissdb` binary. It will immediately return, ensuring no database process is started.
-   **Core Model (`quanta_tissu/tisslm/core/model.py`)**: The `QuantaTissu` class constructor will be updated to default to `use_db=False`. This prevents the model from attempting to initialize its `KnowledgeBase` with a database connection unless explicitly instructed.

## 3. Refactoring Test Runner Error Handling

The test runners will be updated to recognize and report the new specific error types, providing clearer test failure analysis.

-   **Main Test Runner (`tests/run_tests.py`)**: The main `try...except` block will be refactored to include specific `except` clauses for `ModelError` and `SystemError`. Failures will be logged with explicit labels (e.g., "MODEL ERROR", "SYSTEM ERROR").
-   **BDD Test Runner (`tests/bdd_runner.py`)**: The `_execute_step` method will be similarly refactored to catch the new exceptions, allowing for more granular reporting of BDD step failures.

## 4. Integration of New Error Types

The new error types will be integrated into the core application logic to be raised at appropriate points.

-   **Execution Engine (`quanta_tissu/tisslm/core/execution_engine.py`)**: The `run_command` method will be enhanced to catch specific `subprocess` exceptions and re-raise them as `SystemError`.
-   **Core Model (`quanta_tissu/tisslm/core/model.py`)**:
    -   The core `forward` and `generate` methods will be wrapped in `try...except` blocks to catch numerical or array-based exceptions (e.g., `ValueError`, `FloatingPointError`) and re-raise them as `ModelError`.
    -   The `load_weights` method will be modified to raise a `SystemError` on file I/O failures instead of printing a warning and continuing.
