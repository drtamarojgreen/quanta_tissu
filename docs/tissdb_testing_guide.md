# TissDB Testing Guide

This document provides a guide to running the unit and Behavior-Driven Development (BDD) tests for TissDB.

## C++ Unit Tests

The C++ unit tests are designed to test the individual components of the TissDB storage engine in isolation. They cover data structures, serialization, storage components, and query execution.

### Testing Framework

The C++ tests use a simple, custom testing framework defined in `tests/db/test_framework.h`. This framework provides basic assertion macros and a test runner.

- **Test Cases:** Tests are defined using the `TEST_CASE(name)` macro.
- **Assertions:** Assertions are made using macros like `ASSERT_TRUE(condition)`, `ASSERT_EQ(expected, actual)`, etc.

### How to Run Unit Tests

The C++ unit tests are compiled and run using the Makefile located in the `tissdb` directory.

1.  Navigate to the `tissdb` directory:
    ```bash
    cd tissdb
    ```

2.  Build and run the tests:
    ```bash
    make test
    ```

This command will compile the test sources and create an executable named `test_tissdb`, which it then runs. The output will show the status of each test (PASSED or FAILED).

### Example Unit Test

Here is an example from `tests/db/test_collection.cpp` that tests basic CRUD (Create, Read, Update, Delete) operations for a collection.

```cpp
#include "test_framework.h"
#include "../../tissdb/storage/collection.h"
#include "../../tissdb/common/document.h"
#include <filesystem>

TEST_CASE(CollectionBasicCRUD) {
    // Setup: Create a temporary directory for the collection
    std::string collection_path = "test_collection_crud";
    if (std::filesystem::exists(collection_path)) {
        std::filesystem::remove_all(collection_path);
    }

    TissDB::Storage::Collection collection(collection_path);

    // Create a document
    TissDB::Document doc1;
    doc1.id = "doc1";
    TissDB::Element elem1; elem1.key = "name"; elem1.value = std::string("Alice");
    doc1.elements.push_back(elem1);

    // PUT operation
    collection.put("doc1", doc1);

    // GET operation and assertion
    auto retrieved_doc_opt = collection.get("doc1");
    ASSERT_TRUE(retrieved_doc_opt.has_value());
    ASSERT_EQ("doc1", retrieved_doc_opt->id);

    // ... more operations and assertions for Update and Delete ...

    // Teardown: Clean up the test directory
    collection.shutdown();
    std::filesystem::remove_all(collection_path);
}
```

## Behavior-Driven Development (BDD) Tests

The BDD tests are designed to test the behavior of the system from a user's perspective. They are written in a human-readable format that describes the behavior of the system in different scenarios. These tests primarily focus on the TissLang parser and the QuantaTissu model.

### BDD Framework

- **Features:** Scenarios are written in Gherkin syntax in `.feature` files located in `tests/features/`.
- **Step Definitions:** The implementation for the Gherkin steps (Given, When, Then) is written in Python in `tests/test_bdd.py`. A custom BDD runner is used, which maps regex patterns from the `@step` decorator to Python functions.

### How to Run BDD Tests

The BDD tests are run using the main test runner script located in the project root. This script also runs any other Python-based tests.

1.  Navigate to the project root directory.
2.  Run the test script:
    ```bash
    python run_tests.py
    ```

The script will discover and run all `.feature` files in the `tests/features/` directory.

### Example BDD Test

Here is a scenario from `tests/features/tisslang_parser.feature`:

```gherkin
Feature: TissLang Parser

  Scenario: Parsing a simple TissLang script
    Given a TissLang script:
      """
      TASK "A simple task"
      """
    When I parse the script
    Then the AST should be:
      """
      [
        {
          "type": "TASK",
          "description": "A simple task"
        }
      ]
      """
```

And here are the corresponding step definitions from `tests/test_bdd.py`:

```python
# tests/test_bdd.py

@step(r'Given a TissLang script:\s*"""\s*([\s\S]*?)"""')
def given_a_tisslang_script(context, script):
    context['script'] = script.strip()

@step(r'When I parse the script')
def when_i_parse_the_script(context):
    parser = TissLangParser()
    try:
        context['ast'] = parser.parse(context['script'])
        context['error'] = None
    except TissLangParserError as e:
        context['ast'] = None
        context['error'] = e

@step(r'Then the AST should be:\s*"""\s*([\s\S]*?)\s*"""')
def then_the_ast_should_be(context, expected_ast_json):
    expected_ast = json.loads(expected_ast_json)
    assert context['ast'] is not None, "AST is None, an error probably occurred"
    assert context['ast'] == expected_ast
```
