# TissDB BDD Test Execution Report

- **Date:** 2025-08-19 00:12:19
- **Duration:** 2.65 seconds
- **Overall Result:** FAIL

## Summary

| Metric             | Count |
| ------------------ | ----- |
| Scenarios Run      | 44      |
| Scenarios Passed   | 21    |
| Scenarios Failed   | 23    |
| Steps Run          | 160          |
| Steps Passed       | 67        |
| Steps Failed       | 23        |
| Steps Skipped      | 65        |

## Details

### Environment
- **Compilation Skipped:** No

### Errors

**Failed Steps:**

- **Step:** `Then the query result should contain "apple"`
  - **Feature:** `database.feature`
  - **Scenario:** `Execute TissQL query`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 117, in query_result_should_contain
    assert found
           ^^^^^
AssertionError

    ```

- **Step:** `When I begin a transaction`
  - **Feature:** `database.feature`
  - **Scenario:** `Basic transaction operations`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 131, in begin_transaction
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `When I begin a transaction`
  - **Feature:** `database.feature`
  - **Scenario:** `Rollback transaction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 131, in begin_transaction
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the document with ID "doc_types" in "data_types_collection" should have content {"string": "hello", "integer": 123, "float": 45.67, "boolean": true, "null_value": null, "array": [1, "two", false], "nested_object": {"key": "value"}}`
  - **Feature:** `extended_database_tests.feature`
  - **Scenario:** `Handle various data types in documents`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 74, in document_should_have_content
    assert actual_content[key] == value
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the query result should contain "A"`
  - **Feature:** `extended_database_tests.feature`
  - **Scenario:** `Query with OR predicate`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 117, in query_result_should_contain
    assert found
           ^^^^^
AssertionError

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `generate.feature`
  - **Scenario:** `Generating text with a prompt using sampling`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `predict.feature`
  - **Scenario:** `Predicting the next token`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `When I execute the TissQL query "UPDATE products SET price = 1250 WHERE name = 'Laptop'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update a single document using a WHERE clause`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 106, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
                                           ~~~~~~~^^^^^^^^^^^
KeyError: 'db_name'

    ```

- **Step:** `When I execute the TissQL query "UPDATE products SET price = price + 5 WHERE category = 'Furniture'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update multiple documents using a WHERE clause`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 106, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
                                           ~~~~~~~^^^^^^^^^^^
KeyError: 'db_name'

    ```

- **Step:** `When I execute the TissQL query "UPDATE products SET category = 'Office Supplies' WHERE price > 2000" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update with a WHERE clause that matches no documents`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 106, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
                                           ~~~~~~~^^^^^^^^^^^
KeyError: 'db_name'

    ```

- **Step:** `When I execute the TissQL query "DELETE FROM products WHERE name = 'Keyboard'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete a single document using a WHERE clause`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 106, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
                                           ~~~~~~~^^^^^^^^^^^
KeyError: 'db_name'

    ```

- **Step:** `When I execute the TissQL query "DELETE FROM products WHERE category = 'Furniture'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete multiple documents using a WHERE clause`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 106, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
                                           ~~~~~~~^^^^^^^^^^^
KeyError: 'db_name'

    ```

- **Step:** `When I execute the TissQL query "DELETE FROM products WHERE price > 2000" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete with a WHERE clause that matches no documents`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 106, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{context['db_name']}/{collection_name}/_query", json=data)
                                           ~~~~~~~^^^^^^^^^^^
KeyError: 'db_name'

    ```

- **Step:** `Then the query result should be empty`
  - **Feature:** `select_queries.feature`
  - **Scenario:** `Query with no results`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_extended_database_steps.py", line 10, in query_result_should_be_empty
    assert isinstance(context['query_result'], list), f"Query result is not a list: {context['query_result']}"
                      ~~~~~~~^^^^^^^^^^^^^^^^
KeyError: 'query_result'

    ```

- **Step:** `Then the document with ID "sinew_doc" in "cpp_app_data" should have content {"source": "C++ App", "value": 42}`
  - **Feature:** `integration.feature`
  - **Scenario:** `Sinew C++ client interaction pattern with TissDB`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 69, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Adding and retrieving a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 17, in knowledge_base_context
    context['knowledge_base'] = KnowledgeBase(model.embeddings.value, tokenizer.tokenize)
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Adding feedback to a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 12, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Self-updating from interaction with user correction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 12, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Self-updating from interaction without user correction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 12, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Getting knowledge base statistics`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 12, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 42, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb/knowledge

    ```

- **Step:** `Then the query result should contain "Alice"`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Query with multiple predicates`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_database_steps.py", line 117, in query_result_should_contain
    assert found
           ^^^^^
AssertionError

    ```

- **Step:** `Then the operation should be successful with status code 200`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Attempt to create a collection that already exists`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_more_database_steps.py", line 46, in operation_should_be_successful
    assert context['response_status_code'] == int(status_code)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the operation should fail with status code 404`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Attempt to delete a non-existent document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 304, in run
    func(context, *args)
  File "/app/tests/features/steps/test_more_database_steps.py", line 55, in operation_should_fail
    assert context['response_status_code'] == int(status_code)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```
\n## Conclusion\n\nThe test run failed. See error details above.