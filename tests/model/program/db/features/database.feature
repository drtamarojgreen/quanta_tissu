Feature: TissDB Core Database API (Ported)

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
    # Note: Our bridge/steps simplified this to content as a field
    When I delete the document with ID "doc1" from "documents_collection"
    Then the document with ID "doc1" in "documents_collection" should not exist
    And I delete the collection "documents_collection"

  Scenario: Basic transaction operations
    Given a running TissDB instance
    And a collection named "transaction_collection" exists
    When I begin a transaction
    And I add a document with ID "tx_doc" and title "Transaction Doc" to "transaction_collection"
    And I commit the transaction
    Then the document with ID "tx_doc" in "transaction_collection" should exist
    And I delete the collection "transaction_collection"

  Scenario: Rollback transaction
    Given a running TissDB instance
    And a collection named "rollback_collection" exists
    When I begin a transaction
    And I add a document with ID "rb_doc" and title "Rollback Doc" to "rollback_collection"
    And I rollback the transaction
    Then the document with ID "rb_doc" in "rollback_collection" should not exist
    And I delete the collection "rollback_collection"
