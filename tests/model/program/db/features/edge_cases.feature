Feature: TissDB Edge Cases
  As a robust database client, I want to ensure that various edge cases like different data types, large payloads, and rapid requests are handled correctly.

  Scenario: Supported Value Types
    Given a TissDB server is running on "127.0.0.1" at port 9876
    And the database "edge_bdd_db" is created
    And a collection named "edge_test" exists
    When I add a document to "edge_test" with:
      | field      | value  | type   |
      | string_v   | hello  | string |
      | number_v   | 3.14   | number |
      | bool_v     | true   | bool   |
    Then the document should correctly store and retrieve all field types

  Scenario: ID Generation and Idempotency
    Given a TissDB server is running on "127.0.0.1" at port 9876
    And a collection named "id_test" exists
    When I add two documents to "id_test" without IDs
    Then the server should return two distinct IDs
    And a collection named "idemp_test" exists
    When I perform a PUT operation on "idemp_test/id1" twice
    Then the operation should be idempotent

  Scenario: Robustness and Limits
    Given a TissDB server is running on "127.0.0.1" at port 9876
    And a collection named "limit_test" exists
    When I send a large payload of 10000 bytes to "limit_test"
    Then the document should be stored successfully
    And a collection named "load_test" exists
    When I make 50 rapid sequential requests to "load_test"
    Then the server should process all requests without failure
    And I delete the database "edge_bdd_db"
