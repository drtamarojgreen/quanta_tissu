# Test Results Summary

## Overall Status
- **Execution Date:** 2025-08-26
- **Total Tests Run:** 27
- **Passed:** 19
- **Failed:** 8

## Summary of Failures

The test suite was executed in a no-compilation environment (`NO_COMPILE=1`). As expected, all BDD tests requiring the TissDB C++ database failed with connection errors. The remaining failures point to several issues within the Python codebase, primarily related to incorrect return types from core model functions, import errors, and missing model/tokenizer artifacts.

---

### Unit Test Failures (5 tests)

-   **`test_layers.py` (4 failures):**
    -   `test_feed_forward`, `test_layer_norm`, `test_multi_head_attention`: These tests failed with `AttributeError: 'tuple' object has no attribute 'shape'`. This indicates that core neural network layers are returning tuples instead of the expected NumPy arrays.
    -   `test_layer_norm_zero_variance`: This failed with a `ValueError: setting an array element with a sequence`, which also points to an issue with the data shape or type being returned by the `layer_norm` function.

-   **`test_tokenizer.py` (1 failure):**
    -   **CRITICAL ERROR:** Failed to import tests due to `ModuleNotFoundError: No module named 'quanta_tissu.tisslm.tokenizer'`. This suggests a problem with the Python path or the project's structure.

-   **`test_execution_engine.py` (1 failure):**
    -   **CRITICAL ERROR:** Failed to import tests due to `ImportError: cannot import name 'TissSecurityError'`. An object is missing from the specified module.

-   **`test_tisslm_parser.py` (1 failure):**
    -   **CRITICAL ERROR:** Failed to import tests due to `ModuleNotFoundError: No module named 'matcher'`. Another import or path issue.

### BDD Test Failures (1 test suite with multiple scenario failures)

The BDD test suite is counted as one failed test in the summary line but contains numerous scenario failures.

-   **Database Scenarios (Expected Failures):**
    -   All scenarios in `database.feature`, `extended_database_tests.feature`, `more_database_tests.feature`, `select_queries.feature`, and `update_delete_queries.feature` failed with `requests.exceptions.ConnectionError`. This was expected as the TissDB executable was not compiled and the server was not running.

-   **Model & Tokenizer Scenarios (Unexpected Failures):**
    -   **Missing Tokenizer:** Many BDD scenarios across multiple features (`generate.feature`, `tokenizer.feature`, etc.) started with the warning: `BPE tokenizer files not found at /app/models/trained_tokenizer`. This is a prerequisite for running the language model tests.
    -   **`generate.feature`:** All scenarios failed with `TypeError: tuple indices must be integers or slices, not tuple` inside the model's `generate` method. This is likely a direct consequence of the same issue causing the `test_layers.py` unit test failures.
    -   **`tokenizer.feature`:** Scenarios failed with `AssertionError` because the detokenized output did not match the original input, which is a direct result of the missing tokenizer file.
    -   **`predict.feature`:** Failed with `AttributeError: 'QuantaTissu' object has no attribute 'predict'`. This suggests the model's public API has changed or is incomplete.
    -   **`knowledge_base.feature`:** Failed with `AttributeError: 'QuantaTissu' object has no attribute 'embeddings'`. Another case of a missing or changed attribute on the model object.
    -   **`kv_cache.feature`:** Failed with an `IndexError`, likely related to the missing tokenizer and subsequent incorrect vocabulary size.

## Conclusion

The repository's tests are in a broken state. While the database-related failures are expected in this test environment, there are fundamental issues in the Python language model implementation. Key problems to address are:
1.  **Core Layer Return Types:** Fix the functions in `quanta_tissu/tisslm/core/layers.py` to return NumPy arrays, not tuples.
2.  **Import Errors:** Resolve the `ModuleNotFoundError` and `ImportError` issues in the unit tests. This may require fixing the application's internal imports or adjusting the system path.
3.  **Missing Artifacts:** A trained BPE tokenizer needs to be generated and placed in the `/app/models/` directory for the BDD tests to run correctly.
4.  **API Mismatches:** The `QuantaTissu` model object is missing `predict` and `embeddings` attributes that the tests expect.
