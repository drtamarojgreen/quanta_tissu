# TissLM C++ Program and Model Improvement Plan

This document outlines the current state of the C++ test suite and provides a strategic plan to address the identified issues and guide future development.

## 1. Executive Summary Reporting

*   **Issue:** The main test script (`tests/run_program_tests.sh`) executes the tests but does not provide a consolidated summary of results, making it difficult to get a quick overview of the suite's health.
*   **Strategy:** The `run_program_tests.sh` script will be modified to parse the output of each test executable. It will collect pass/fail counts and performance metrics (e.g., time comparisons from `kv_cache_test`). At the end of the run, it will print a final, formatted executive summary table.

## 2. Implement Skipped/Placeholder Tests

*   **Issue:** The `advanced_sampling_test` contains numerous skipped tests for unimplemented sampling methods (e.g., Beam Search, Contrastive Search, Mirostat).
*   **Strategy:** The unimplemented sampling methods will be implemented in the C++ `Generator` class. The implementation will be done incrementally, one method at a time. As each method is implemented, the corresponding placeholder test will be replaced with a full-fledged test case.

## 3. Thematic Prompts and Content (Cognitive Behavioral Therapy)

*   **Issue:** Test prompts are currently generic (e.g., "The quick brown fox") and not aligned with the project's focus on Cognitive Behavioral Therapy (CBT). A reference to "hugging face" in the tokenizer training test is also out of place.
*   **Strategy:**
    *   All generic test prompts in files like `kv_cache_test.cpp`, `full_fledged_workout_cpp.cpp`, and `standard_generation_test.cpp` will be replaced with prompts relevant to CBT concepts (e.g., "A common cognitive distortion is catastrophizing, which involves...").
    *   The "hugging face" string in `tokenizer_test.cpp` will be removed and replaced with a CBT-related sentence.

## 4. Corpus-Based Tokenizer Test

*   **Issue:** The `test_corpus/` directory exists but is not used in any tests, representing a missed opportunity for more robust testing.
*   **Strategy:** A new test case will be added to `tokenizer_test.cpp`. This test will:
    1.  Read a text file from the `test_corpus/` directory.
    2.  Use the content of this file to train a new, temporary tokenizer.
    3.  Verify that the tokenizer is trained correctly and can encode and decode text from the corpus.

## 5. Checkpoint Generation Tests

*   **Issue:** The model's ability to save and load training checkpoints is a critical feature that is not currently tested.
*   **Strategy:** A new test file, `checkpoint_test.cpp`, will be created. This test will:
    1.  Initialize a small transformer model.
    2.  Run a few training steps on a small dataset.
    3.  Save a checkpoint of the model's state.
    4.  Verify that the checkpoint file is created and appears to be valid.
    *This will be introduced as a new, separate test to be integrated incrementally.*

## 6. Detailed Final Test Summary

*   **Issue:** The final test summary is minimal. A more detailed report is needed to quickly identify problem areas.
*   **Strategy:** This expands on point 1. The final summary report in the `run_program_tests.sh` script will be enhanced to include:
    *   A list of all test executables that were run.
    *   For each executable, a high-level description of the functionality it tests.
    *   A clear breakdown of which specific sub-tests failed within each executable.
    *   Actionable recommendations, such as prioritizing critical bug fixes.

## 7. Database Test Failures

*   **Issue:** Two tests in `test_db_and_retrieval` are still failing: `Multiple document retrieval` and `JOIN query`.
*   **Strategy:** These failures point to bugs in the TissDB query engine. The investigation will proceed as follows:
    1.  **Analyze Test Code:** The C++ code for these two tests will be reviewed to understand the exact queries being sent and the expected results.
    2.  **Isolate the Query:** The failing queries will be extracted and run directly against the TissDB server to reproduce the error in isolation.
    3.  **Debug TissDB:** The TissDB server-side logic for handling `COUNT` with multiple documents and `JOIN` operations will be debugged to identify and fix the root cause.

## 8. Persistent Tokenizer `get_vocab_size` and `Encode` Error

*   **Issue:** The `tokenizer_test` continues to fail. The latest run shows that while the pathing is now correct, the test still fails because the vocabulary loaded from `test_tokenizer` does not match the test's hardcoded expectations. The test expects `"ab cd"` to encode to `{4, 3, 5}`, but it gets `{527, 269, 100}` from the 4000-token vocabulary. This is a critical, persistent bug blocking other tests.
*   **Strategy:** This issue will be definitively resolved. The current `test_tokenizer` is not suitable for this unit test. The plan is to:
    1.  **Create a Correct Test Tokenizer:** A new, minimal tokenizer will be created specifically for this test.
    2.  **Define a Minimal Vocabulary:** The `vocab.json` will be created to contain only the tokens required for the test to pass (e.g., `{"a": 0, "b": 1, "c": 2, "d": 3, "ab": 4, "cd": 5}`).
    3.  **Define Minimal Merges:** The `merges.txt` will be created with the corresponding merges (e.g., `a b`, `c d`).
    4.  **Update Test Files:** These new `vocab.json` and `merges.txt` files will replace the existing ones in the `test_tokenizer/` directory, ensuring the test loads a small, predictable vocabulary that matches its expectations.
