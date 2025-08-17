Feature: TissDB Extended Database Operations

  Scenario: Handle various data types in documents
    Given a running TissDB instance
    And a collection named "data_types_collection" exists
    When I create a document with ID "doc_types" and content {"string": "hello", "integer": 123, "float": 45.67, "boolean": true, "null_value": null, "array": [1, "two", false], "nested_object": {"key": "value"}} in "data_types_collection"
    Then the document with ID "doc_types" in "data_types_collection" should have content {"string": "hello", "integer": 123, "float": 45.67, "boolean": true, "null_value": null, "array": [1, "two", false], "nested_object": {"key": "value"}}
    And I delete the collection "data_types_collection"

  Scenario: Query with OR predicate
    Given a running TissDB instance
    And a collection named "or_query_collection" exists
    And a document with ID "item1" and content {"category": "A", "value": 10} in "or_query_collection"
    And a document with ID "item2" and content {"category": "B", "value": 20} in "or_query_collection"
    And a document with ID "item3" and content {"category": "C", "value": 30} in "or_query_collection"
    When I execute the TissQL query "SELECT category FROM or_query_collection WHERE category = 'A' OR category = 'C'" on "or_query_collection"
    Then the query result should contain "A"
    And the query result should contain "C"
    And the query result should not contain "B"
    And I delete the collection "or_query_collection"

  Scenario: Query with no matching results
    Given a running TissDB instance
    And a collection named "no_results_collection" exists
    And a document with ID "data1" and content {"value": 100} in "no_results_collection"
    When I execute the TissQL query "SELECT value FROM no_results_collection WHERE value < 50" on "no_results_collection"
    Then the query result should be empty
    And I delete the collection "no_results_collection"
