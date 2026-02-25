Feature: Extended Database Tests (Ported)

  Scenario: Bulk operations simulation
    Given a running TissDB instance
    And a collection named "bulk_test" exists
    When I make 10 rapid sequential requests to "bulk_test"
    Then the server should process all requests without failure
    And I delete the collection "bulk_test"

  Scenario: Large value handling
    Given a running TissDB instance
    And a collection named "large_val_test" exists
    When I send a large payload of 5000 bytes to "large_val_test"
    Then the document should be stored successfully
    And I delete the collection "large_val_test"
