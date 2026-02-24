Feature: More Database Tests (Ported)

  Scenario: Connection stability
    Given a TissDB server is running on "127.0.0.1" at port 9876
    Then the server should process all requests without failure

  Scenario: Database cleanup
    Given a running TissDB instance
    When I delete the database "bdd_test_db"
    Then the command should succeed
