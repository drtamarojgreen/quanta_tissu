Feature: TissDB Advanced Database Operations

  Scenario: Batch document insertion
    Given a running TissDB instance
    And a collection named "batch_collection" exists
    And I insert the following documents into "batch_collection":
      | id   | content                               |
      | doc1 | {"name": "First Document", "value": 1}  |
      | doc2 | {"name": "Second Document", "value": 2} |
      | doc3 | {"name": "Third Document", "value": 3}  |
    Then the document with ID "doc1" in "batch_collection" should have content '{"name": "First Document", "value": 1}'
    And the document with ID "doc2" in "batch_collection" should have content '{"name": "Second Document", "value": 2}'
    And the document with ID "doc3" in "batch_collection" should have content '{"name": "Third Document", "value": 3}'
    And I delete the collection "batch_collection"

  Scenario: List all documents in a collection
    Given a running TissDB instance
    And a collection named "list_collection" exists
    And a document with ID "doc_a" and content {"item": "apple"} in "list_collection"
    And a document with ID "doc_b" and content {"item": "banana"} in "list_collection"
    When I list all documents in "list_collection"
    Then the document list should contain "doc_a"
    And the document list should contain "doc_b"
    And I delete the collection "list_collection"

  Scenario: Query with multiple predicates
    Given a running TissDB instance
    And a collection named "multi_predicate_query" exists
    And a document with ID "q_doc1" and content {"name": "Alice", "age": 30, "city": "New York"} in "multi_predicate_query"
    And a document with ID "q_doc2" and content {"name": "Bob", "age": 25, "city": "Los Angeles"} in "multi_predicate_query"
    And a document with ID "q_doc3" and content {"name": "Charlie", "age": 30, "city": "New York"} in "multi_predicate_query"
    When I execute the TissQL query "SELECT name FROM multi_predicate_query WHERE age = 30 AND city = 'New York'" on "multi_predicate_query"
    Then the query result should contain "Alice"
    And the query result should contain "Charlie"
    And the query result should not contain "Bob"
    And I delete the collection "multi_predicate_query"

  Scenario: Attempt to create a collection that already exists
    Given a running TissDB instance
    And a collection named "existing_collection" exists
    When I attempt to create a collection named "existing_collection"
    Then the operation should be successful with status code 200
    And I delete the collection "existing_collection"

  Scenario: Attempt to delete a non-existent document
    Given a running TissDB instance
    And a collection named "no_such_doc_collection" exists
    When I attempt to delete the document with ID "non_existent_doc" from "no_such_doc_collection"
    Then the operation should fail with status code 404
    And I delete the collection "no_such_doc_collection"
