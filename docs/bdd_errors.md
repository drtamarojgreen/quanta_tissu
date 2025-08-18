BDD Runner: All steps registered.
Running feature: database.feature
  Running Scenario: Create and delete a collection
    Executing step: Given a running TissDB instance
    Executing step: When I create a collection named "my_test_collection"
    Executing step: Then the collection "my_test_collection" should exist
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 30, in collection_should_exist
    assert collection_name in collections
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: When I delete the collection "my_test_collection"
    Skipping step due to previous failure: Then the collection "my_test_collection" should not exist
  Running Scenario: Create, retrieve, update, and delete a document
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "documents_collection" exists
    Executing step: When I create a document with ID "doc1" and content {"name": "Test Document", "value": 123} in "documents_collection"
    Executing step: Then the document with ID "doc1" in "documents_collection" should have content {"name": "Test Document", "value": 123}
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 65, in document_should_have_content
    assert actual_content == expected_content
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: When I update the document with ID "doc1" with content {"name": "Updated Document", "value": 456} in "documents_collection"
    Skipping step due to previous failure: Then the document with ID "doc1" in "documents_collection" should have content {"name": "Updated Document", "value": 456}
    Skipping step due to previous failure: When I delete the document with ID "doc1" from "documents_collection"
    Skipping step due to previous failure: Then the document with ID "doc1" in "documents_collection" should not exist
    Skipping step due to previous failure: And I delete the collection "documents_collection"
  Running Scenario: Execute TissQL query
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "query_collection" exists
    Executing step: And a document with ID "doc_a" and content {"item": "apple", "price": 1.0} in "query_collection"
    Executing step: And a document with ID "doc_b" and content {"item": "banana", "price": 0.5} in "query_collection"
    Executing step: When I execute the TissQL query "SELECT item FROM query_collection WHERE price > 0.7" on "query_collection"
    Executing step: Then the query result should contain "apple"
    Executing step: And the query result should not contain "banana"
    Executing step: And I delete the collection "query_collection"
  Running Scenario: Basic transaction operations
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "transaction_collection" exists
    Executing step: When I begin a transaction
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 119, in begin_transaction
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And I create a document with ID "tx_doc" and content {"status": "pending"} in "transaction_collection"
    Skipping step due to previous failure: And I commit the transaction
    Skipping step due to previous failure: Then the document with ID "tx_doc" in "transaction_collection" should have content {"status": "pending"}
    Skipping step due to previous failure: And I delete the collection "transaction_collection"
  Running Scenario: Rollback transaction
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "rollback_collection" exists
    Executing step: When I begin a transaction
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 119, in begin_transaction
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And I create a document with ID "rb_doc" and content {"status": "temp"} in "rollback_collection"
    Skipping step due to previous failure: And I rollback the transaction
    Skipping step due to previous failure: Then the document with ID "rb_doc" in "rollback_collection" should not exist
    Skipping step due to previous failure: And I delete the collection "rollback_collection"
Running feature: extended_database_tests.feature
  Running Scenario: Handle various data types in documents
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "data_types_collection" exists
    Executing step: When I create a document with ID "doc_types" and content {"string": "hello", "integer": 123, "float": 45.67, "boolean": true, "null_value": null, "array": [1, "two", false], "nested_object": {"key": "value"}} in "data_types_collection"
    Executing step: Then the document with ID "doc_types" in "data_types_collection" should have content {"string": "hello", "integer": 123, "float": 45.67, "boolean": true, "null_value": null, "array": [1, "two", false], "nested_object": {"key": "value"}}
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 65, in document_should_have_content
    assert actual_content == expected_content
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And I delete the collection "data_types_collection"
  Running Scenario: Query with OR predicate
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "or_query_collection" exists
    Executing step: And a document with ID "item1" and content {"category": "A", "value": 10} in "or_query_collection"
    Executing step: And a document with ID "item2" and content {"category": "B", "value": 20} in "or_query_collection"
    Executing step: And a document with ID "item3" and content {"category": "C", "value": 30} in "or_query_collection"
    Executing step: When I execute the TissQL query "SELECT category FROM or_query_collection WHERE category = 'A' OR category = 'C'" on "or_query_collection"
    Executing step: Then the query result should contain "A"
    WARNING - No step definition found for line: And the query result should contain "C"
    Executing step: And the query result should not contain "B"
    Executing step: And I delete the collection "or_query_collection"
  Running Scenario: Query with no matching results
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "no_results_collection" exists
    Executing step: And a document with ID "data1" and content {"value": 100} in "no_results_collection"
    Executing step: When I execute the TissQL query "SELECT value FROM no_results_collection WHERE value < 50" on "no_results_collection"
    Executing step: Then the query result should be empty
    Executing step: And I delete the collection "no_results_collection"
Running feature: generate.feature
  Running Scenario: Generating text with a prompt using greedy decoding
    Executing step: Given a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I generate 5 new tokens with the prompt "hello" using the "greedy" method
    Skipping step due to previous failure: Then the generated tokens should be a list of 5 integers
  Running Scenario: Generating text with a prompt using sampling
    Executing step: Given a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I generate 5 new tokens with the prompt "hello" using the "sampling" method
    Skipping step due to previous failure: Then the generated tokens should be a list of 5 integers
Running feature: integration.feature
  Running Scenario: TissLM KnowledgeBase retrieves context from TissDB for a user prompt
    Executing step: Given a running TissDB instance
    WARNING - No step definition found for line: And a TissDB collection named "knowledge_articles" is available for TissLM
    WARNING - No step definition found for line: And the "knowledge_articles" collection contains a document with ID "lsm_doc" and content {"title": "LSM Tree", "body": "A Log-Structured Merge-Tree is a data structure designed for efficient writes."}
    WARNING - No step definition found for line: And the "knowledge_articles" collection contains a document with ID "b_tree_doc" and content {"title": "B-Tree", "body": "A B-Tree is a self-balancing tree data structure that maintains sorted data."}
    WARNING - No step definition found for line: When the TissLM receives a user prompt "Tell me about LSM Trees"
    WARNING - No step definition found for line: And the TissLM KnowledgeBase formulates a TissQL query "SELECT body FROM knowledge_articles WHERE title = 'LSM Tree'"
    WARNING - No step definition found for line: And the KnowledgeBase executes the query against the "knowledge_articles" collection
    WARNING - No step definition found for line: Then the query result for the KnowledgeBase should contain "A Log-Structured Merge-Tree is a data structure designed for efficient writes."
    WARNING - No step definition found for line: And the query result for the KnowledgeBase should not contain "A B-Tree is a self-balancing tree data structure that maintains sorted data."
  Running Scenario: TissLM generates an augmented prompt using retrieved context
    WARNING - No step definition found for line: Given a user prompt "What is a Log-Structured Merge-Tree?"
    WARNING - No step definition found for line: And a retrieved context from TissDB: "A Log-Structured Merge-Tree (LSM-Tree) is a data structure with performance characteristics that make it attractive for write-heavy workloads."
    WARNING - No step definition found for line: When the TissLM augments the prompt with the retrieved context
    WARNING - No step definition found for line: Then the final prompt sent to the language model should be:
  Running Scenario: Sinew C++ client interaction pattern with TissDB
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "cpp_app_data" exists
    WARNING - No step definition found for line: When a simulated Sinew client creates a document with ID "sinew_doc" and content {"source": "C++ App", "value": 42} in "cpp_app_data"
    Executing step: Then the document with ID "sinew_doc" in "cpp_app_data" should have content {"source": "C++ App", "value": 42}
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 62, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: When a simulated Sinew client deletes the document with ID "sinew_doc" from "cpp_app_data"
    Skipping step due to previous failure: Then the document with ID "sinew_doc" in "cpp_app_data" should not exist
    Skipping step due to previous failure: And I delete the collection "cpp_app_data"
Running feature: knowledge_base.feature
  Running Scenario: Adding and retrieving a document
    Executing step: Given a knowledge base with a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I add the document "The quick brown fox jumps over the lazy dog."
    Skipping step due to previous failure: And I retrieve documents for the query "quick fox"
    Skipping step due to previous failure: Then the retrieved documents should contain "The quick brown fox jumps over the lazy dog."
  Running Scenario: Adding feedback to a document
    Executing step: Given a knowledge base with a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: And I add the document "This is a test document."
    Skipping step due to previous failure: When I retrieve documents for the query "test"
    Skipping step due to previous failure: And I add feedback with score 5 and text "Very relevant" for the retrieved documents
    Skipping step due to previous failure: Then the knowledge base stats should show 1 feedback entry
  Running Scenario: Self-updating from interaction with user correction
    Executing step: Given a knowledge base with a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I self-update from interaction with query "What is the capital of France?" generated response "London" and user correction "Paris"
    Skipping step due to previous failure: Then the knowledge base should contain "Query: What is the capital of France? Correct Answer: Paris"
  Running Scenario: Self-updating from interaction without user correction
    Executing step: Given a knowledge base with a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I self-update from interaction with query "What is the capital of France?" generated response "Paris" and no user correction
    Skipping step due to previous failure: Then the knowledge base should contain "Query: What is the capital of France? Response: Paris"
  Running Scenario: Getting knowledge base statistics
    Executing step: Given a knowledge base with a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I add the document "Document 1"
    Skipping step due to previous failure: And I add the document "Document 2"
    Skipping step due to previous failure: And I retrieve documents for the query "Document 1"
    Skipping step due to previous failure: Then the knowledge base stats should show 2 total documents
    Skipping step due to previous failure: And the knowledge base stats should show 1 total access
Running feature: kv_cache.feature
  Running Scenario: Generating with KV cache produces the same output as generating without it
    Executing step: Given a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I generate text with a prompt "hello" and 5 new tokens using the KV cache
    Skipping step due to previous failure: And I generate text with the same prompt and new tokens without the KV cache
    Skipping step due to previous failure: Then the generated tokens should be the same
Running feature: more_database_tests.feature
  Running Scenario: Batch document insertion
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "batch_collection" exists
    Executing step: When I insert the following documents into "batch_collection":
      ERROR executing step: register_steps.<locals>.insert_documents() missing 1 required positional argument: 'table'
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
TypeError: register_steps.<locals>.insert_documents() missing 1 required positional argument: 'table'

    Skipping step due to previous failure: | id   | content                               |
    Skipping step due to previous failure: | doc1 | {"name": "First Document", "value": 1}  |
    Skipping step due to previous failure: | doc2 | {"name": "Second Document", "value": 2} |
    Skipping step due to previous failure: | doc3 | {"name": "Third Document", "value": 3}  |
    Skipping step due to previous failure: Then the document with ID "doc1" in "batch_collection" should have content {"name": "First Document", "value": 1}
    Skipping step due to previous failure: And the document with ID "doc2" in "batch_collection" should have content {"name": "Second Document", "value": 2}
    Skipping step due to previous failure: And the document with ID "doc3" in "batch_collection" should have content {"name": "Third Document", "value": 3}
    Skipping step due to previous failure: And I delete the collection "batch_collection"
  Running Scenario: List all documents in a collection
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "list_collection" exists
    Executing step: And a document with ID "doc_a" and content {"item": "apple"} in "list_collection"
    Executing step: And a document with ID "doc_b" and content {"item": "banana"} in "list_collection"
    Executing step: When I list all documents in "list_collection"
    Executing step: Then the document list should contain "doc_a"
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_more_database_steps.py", line 31, in document_list_should_contain
    assert doc_id in context['document_list']
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And the document list should contain "doc_b"
    Skipping step due to previous failure: And I delete the collection "list_collection"
  Running Scenario: Query with multiple predicates
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "multi_predicate_query" exists
    Executing step: And a document with ID "q_doc1" and content {"name": "Alice", "age": 30, "city": "New York"} in "multi_predicate_query"
    Executing step: And a document with ID "q_doc2" and content {"name": "Bob", "age": 25, "city": "Los Angeles"} in "multi_predicate_query"
    Executing step: And a document with ID "q_doc3" and content {"name": "Charlie", "age": 30, "city": "New York"} in "multi_predicate_query"
    Executing step: When I execute the TissQL query "SELECT name FROM multi_predicate_query WHERE age = 30 AND city = 'New York'" on "multi_predicate_query"
    Executing step: Then the query result should contain "Alice"
    WARNING - No step definition found for line: And the query result should contain "Charlie"
    Executing step: And the query result should not contain "Bob"
    Executing step: And I delete the collection "multi_predicate_query"
  Running Scenario: Attempt to create a collection that already exists
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "existing_collection" exists
    Executing step: When I attempt to create a collection named "existing_collection"
    Executing step: Then the operation should be successful with status code 200
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_more_database_steps.py", line 40, in operation_should_be_successful
    assert context['response_status_code'] == int(status_code)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And I delete the collection "existing_collection"
  Running Scenario: Attempt to delete a non-existent document
    Executing step: Given a running TissDB instance
    Executing step: And a collection named "no_such_doc_collection" exists
    Executing step: When I attempt to delete the document with ID "non_existent_doc" from "no_such_doc_collection"
    Executing step: Then the operation should fail with status code 404
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_more_database_steps.py", line 49, in operation_should_fail
    assert context['response_status_code'] == int(status_code)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And I delete the collection "no_such_doc_collection"
Running feature: predict.feature
  Running Scenario: Predicting the next token
    Executing step: Given a model and tokenizer
      ERROR executing step: 'NoneType' object cannot be interpreted as an integer
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    Skipping step due to previous failure: When I predict the next token for the prompt "hello"
    Skipping step due to previous failure: Then the next token should be a valid token id
Running feature: select_queries.feature
  Running Scenario: Select all data from a collection
    WARNING - No step definition found for line: When I execute the TissQL query "SELECT * FROM products"
    WARNING - No step definition found for line: Then the query result should have 5 documents
    WARNING - No step definition found for line: And the query result should contain a document with "name" = "Laptop"
    WARNING - No step definition found for line: And the query result should contain a document with "price" = 20
  Running Scenario: Select specific fields from a collection
    WARNING - No step definition found for line: When I execute the TissQL query "SELECT name, price FROM products"
    WARNING - No step definition found for line: Then the query result should have 5 documents
    WARNING - No step definition found for line: And each document in the result should have the fields ["name", "price"]
    WARNING - No step definition found for line: And each document in the result should not have the fields ["category", "stock"]
    WARNING - No step definition found for line: And the query result should contain a document with "name" = "Keyboard" and "price" = 75
  Running Scenario: Filter data with a WHERE clause
    WARNING - No step definition found for line: When I execute the TissQL query "SELECT * FROM products WHERE category = 'Tech'"
    WARNING - No step definition found for line: Then the query result should have 3 documents
    WARNING - No step definition found for line: And the query result should only contain documents where "category" is "Tech"
  Running Scenario: Select specific fields with a WHERE clause
    WARNING - No step definition found for line: When I execute the TissQL query "SELECT name FROM products WHERE stock > 250"
    WARNING - No step definition found for line: Then the query result should have 2 documents
    WARNING - No step definition found for line: And the query result should contain a document with "name" = "Mouse"
    WARNING - No step definition found for line: And the query result should contain a document with "name" = "T-Shirt"
    WARNING - No step definition found for line: And the query result should not contain a document with "name" = "Laptop"
  Running Scenario: Complex query with multiple conditions
    WARNING - No step definition found for line: When I execute the TissQL query "SELECT name, price FROM products WHERE category = 'Tech' AND price < 100"
    WARNING - No step definition found for line: Then the query result should have 2 documents
    WARNING - No step definition found for line: And the query result should contain a document with "name" = "Keyboard" and "price" = 75
    WARNING - No step definition found for line: And the query result should contain a document with "name" = "Mouse" and "price" = 25
    WARNING - No step definition found for line: And the query result should not contain a document with "name" = "Laptop"
  Running Scenario: Query with no results
    WARNING - No step definition found for line: When I execute the TissQL query "SELECT * FROM products WHERE price > 2000"
    Executing step: Then the query result should be empty
      ERROR executing step: 'query_result'
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_extended_database_steps.py", line 10, in query_result_should_be_empty
    assert isinstance(context['query_result'], list), f"Query result is not a list: {context['query_result']}"
                      ~~~~~~~^^^^^^^^^^^^^^^^
KeyError: 'query_result'

Running feature: tokenizer.feature
  Running Scenario: Tokenizing and detokenizing a simple string should result in the original string
    Executing step: Given a tokenizer
    Executing step: When I tokenize the string "hello world"
    Executing step: And I detokenize the tokens
    Executing step: Then the resulting string should be "hello world"
  Running Scenario: Tokenizing an empty string
    Executing step: Given a tokenizer
    Executing step: When I tokenize the string ""
    Executing step: Then the result should be an empty list of tokens
      ERROR executing step: The truth value of an empty array is ambiguous. Use `array.size > 0` to check that an array is not empty.
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_tokenizer_steps.py", line 23, in check_empty_list
    assert context['tokens'] == []
           ^^^^^^^^^^^^^^^^^^^^^^^
ValueError: The truth value of an empty array is ambiguous. Use `array.size > 0` to check that an array is not empty.

  Running Scenario: Tokenizing and detokenizing a string with special characters
    Executing step: Given a tokenizer
    Executing step: When I tokenize the string "hello, world! 123? <test>"
    Executing step: And I detokenize the tokens
    Executing step: Then the resulting string should be "hello, world! 123? <test>"
Running feature: update_delete_queries.feature
  Running Scenario: Update a single document using a WHERE clause
    Executing step: When I execute the TissQL query "UPDATE products SET price = 1250 WHERE name = 'Laptop'" on "products"
    Executing step: Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1250}
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 62, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And the document with ID "prod2" in "products" should have content {"name": "Keyboard", "category": "Electronics", "price": 75}
  Running Scenario: Update multiple documents using a WHERE clause
    Executing step: When I execute the TissQL query "UPDATE products SET price = price + 5 WHERE category = 'Furniture'" on "products"
    Executing step: Then the document with ID "prod4" in "products" should have content {"name": "Desk Chair", "category": "Furniture", "price": 155}
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 62, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And the document with ID "prod5" in "products" should have content {"name": "Desk Lamp", "category": "Furniture", "price": 40}
    Skipping step due to previous failure: And the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1200}
  Running Scenario: Update with a WHERE clause that matches no documents
    Executing step: When I execute the TissQL query "UPDATE products SET category = 'Office Supplies' WHERE price > 2000" on "products"
    Executing step: Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1200}
      ERROR executing step:
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_database_steps.py", line 62, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    Skipping step due to previous failure: And the document with ID "prod2" in "products" should have content {"name": "Keyboard", "category": "Electronics", "price": 75}
    Skipping step due to previous failure: And the document with ID "prod3" in "products" should have content {"name": "Mouse", "category": "Electronics", "price": 25}
    Skipping step due to previous failure: And the document with ID "prod4" in "products" should have content {"name": "Desk Chair", "category": "Furniture", "price": 150}
    Skipping step due to previous failure: And the document with ID "prod5" in "products" should have content {"name": "Desk Lamp", "category": "Furniture", "price": 35}
  Running Scenario: Delete a single document using a WHERE clause
    Executing step: When I execute the TissQL query "DELETE FROM products WHERE name = 'Keyboard'" on "products"
    Executing step: Then the document with ID "prod2" in "products" should not exist
    WARNING - No step definition found for line: And the document with ID "prod1" in "products" should exist
  Running Scenario: Delete multiple documents using a WHERE clause
    Executing step: When I execute the TissQL query "DELETE FROM products WHERE category = 'Furniture'" on "products"
    Executing step: Then the document with ID "prod4" in "products" should not exist
    WARNING - No step definition found for line: And the document with ID "prod5" in "products" should not exist
    WARNING - No step definition found for line: And the document with ID "prod1" in "products" should exist
  Running Scenario: Delete with a WHERE clause that matches no documents
    Executing step: When I execute the TissQL query "DELETE FROM products WHERE price > 2000" on "products"
    Executing step: Then the document with ID "prod1" in "products" should exist
      ERROR executing step: Expected document 'prod1' to exist, but it does not (status code: 404).
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 249, in run
    func(context, *match.groups())
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_update_delete_steps.py", line 31, in document_should_exist
    assert response.status_code == 200, f"Expected document '{doc_id}' to exist, but it does not (status code: {response.status_code})."
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError: Expected document 'prod1' to exist, but it does not (status code: 404).

    Skipping step due to previous failure: And the document with ID "prod2" in "products" should exist
    Skipping step due to previous failure: And the document with ID "prod3" in "products" should exist
    Skipping step due to previous failure: And the document with ID "prod4" in "products" should exist
    Skipping step due to previous failure: And the document with ID "prod5" in "products" should exist
    Skipping step due to previous failure: Scenario Teardown:
    Skipping step due to previous failure: When I delete the collection "products"
    Skipping step due to previous failure: Then the collection "products" should not exist
BDD Runner: Leaving existing database server running.
BDD Runner: Report generated at C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\docs\tissdb_bdd_implementation_plan.md
