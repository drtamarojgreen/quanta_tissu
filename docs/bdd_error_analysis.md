# BDD Test Error Analysis and Fixes

This document summarizes the errors found in the BDD test suite and provides steps to fix them.

## 1. Database Connection Errors

### Summary
A large number of tests are failing with `AssertionError` or `requests.exceptions.ConnectionError`. These errors are all related to the TissDB database not being available. The BDD test runner tries to start the database, but it fails because the database executable `tissdb/tissdb` does not exist.

### Fix
The `tissdb` executable needs to be compiled from the source code in the `tissdb/` directory. This requires running `make`.

**Action:** Run the following command in the root directory of the project:
```bash
make -C tissdb
```
**Note:** This action requires permission to compile code.

## 2. Model and Tokenizer Errors

### Summary
Several tests related to the TissLM model and tokenizer are failing with errors like `TypeError: 'NoneType' object cannot be interpreted as an integer` and warnings about missing BPE tokenizer files. This indicates that the pre-trained model and tokenizer are not available.

### Fix
The `quanta_tissu/tisslm/train_bpe.py` script can be used to train the tokenizer and generate the required files.

However, this script requires a corpus text file at `corpus/resiliency_research.txt`. This directory and file do not exist in the repository.

**Action:**
1.  Create a `corpus/` directory in the root of the project.
2.  Add a text file named `resiliency_research.txt` inside the `corpus/` directory with some text data.
3.  Run the training script:
    ```bash
    python3 quanta_tissu/tisslm/train_bpe.py
    ```

## 3. BDD Runner Bug: Missing Argument for Table Steps

### Summary
The "Batch document insertion" scenario fails with `TypeError: register_steps.<locals>.insert_documents() missing 1 required positional argument: 'table'`.

### Fix
This is a bug in the BDD test runner (`tests/bdd_runner.py`). The runner is not correctly parsing Gherkin feature files that contain multiline table arguments. The `bdd_runner.py` script needs to be updated to handle these tables and pass them as an argument to the step definition function.

**Action:** The `run` method in `tests/bdd_runner.py` needs to be modified to parse and pass table data to the step functions. This is a more involved fix that requires modifying the test runner's logic.

## 4. Other Assertion Errors

### Summary
There are several other `AssertionError`s throughout the test suite.

### Fix
These are likely cascading failures caused by the aformentioned root causes (no database, no model). Once the database is running and the model is trained, these errors should be resolved. If they persist, they will need to be investigated individually.
