Feature: TissDB Core Connectivity
  As a system component, I want to ensure that TissDB and TissDBLite clients can reliably manage documents and connect to the server.

  Scenario: Full Document Lifecycle
    Given a TissDB server is running on "127.0.0.1" at port 9876
    And the database "core_bdd_db" is created
    When I ensure collection setup for "test_docs"
    And I add a document with title "BDD Test Doc" to "test_docs"
    And I add a document with ID "client_001" and title "Manual ID Doc" to "test_docs"
    Then I should be able to retrieve the document "client_001" from "test_docs"
    And the retrieved document should have title "Manual ID Doc"
    When I query "test_docs" with "SELECT * FROM test_docs WHERE title = 'BDD Test Doc'"
    Then the query results should contain "BDD Test Doc"
    When I submit feedback with rating 5 and comment "BDD Success"
    Then the feedback should be successfully stored
    And I delete the database "core_bdd_db"

  Scenario: TissDBLite Reset
    Given a TissDBLite server is running on "127.0.0.1" at port 9877
    When I send a TissDBLite command "deleteDb"
    Then the command should succeed
