Feature: TissDB Core Database API

  Scenario: Create and delete a collection
    Given a running TissDB instance
    When I create a collection named "my_test_collection"
    Then the collection "my_test_collection" should exist
    When I delete the collection "my_test_collection"
    Then the collection "my_test_collection" should not exist

  Scenario: Create, retrieve, update, and delete a document
    Given a running TissDB instance
    And a collection named "documents_collection" exists
    When I create a document with ID "doc1" and content '{"name": "Test Document", "value": 123}' in "documents_collection"
    Then the document with ID "doc1" in "documents_collection" should have content '{"name": "Test Document", "value": 123}'
    When I update the document with ID "doc1" with content '{"name": "Updated Document", "value": 456}' in "documents_collection"
    Then the document with ID "doc1" in "documents_collection" should have content '{"name": "Updated Document", "value": 456}'
    When I delete the document with ID "doc1" from "documents_collection"
    Then the document with ID "doc1" in "documents_collection" should not exist
    And I delete the collection "documents_collection"

  Scenario: Execute TissQL query
    Given a running TissDB instance
    And a collection named "query_collection" exists
    And a document with ID "doc_a" and content {"item": "apple", "price": 1.0} in "query_collection"
    And a document with ID "doc_b" and content {"item": "banana", "price": 0.5} in "query_collection"
    When I execute the TissQL query "SELECT item FROM query_collection WHERE price > 0.7" on "query_collection"
    Then the query result should contain "apple"
    And the query result should not contain "banana"
    And I delete the collection "query_collection"

  Scenario: Basic transaction operations
    Given a running TissDB instance
    And a collection named "transaction_collection" exists
    When I begin a transaction
    And I create a document with ID "tx_doc" and content {"status": "pending"} in "transaction_collection"
    And I commit the transaction
    Then the document with ID "tx_doc" in "transaction_collection" should have content {"status": "pending"}
    And I delete the collection "transaction_collection"

  Scenario: Rollback transaction
    Given a running TissDB instance
    And a collection named "rollback_collection" exists
    When I begin a transaction
    And I create a document with ID "rb_doc" and content {"status": "temp"} in "rollback_collection"
    And I rollback the transaction
    Then the document with ID "rb_doc" in "rollback_collection" should not exist
    And I delete the collection "rollback_collection"
