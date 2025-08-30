Feature: Security - Auditing

  Background:
    Given a running TissDB instance
    And the audit log file is cleared

  Scenario: A failed authentication attempt is audited
    When I make a GET request to the protected endpoint "/_databases" with an invalid token
    Then the audit log should contain an event with type "AuthFailure"

  Scenario: A forbidden access attempt is audited
    Given a user with a valid "read_only" token
    When I attempt to DELETE the database
    Then the audit log should contain an event with type "PermissionCheckFailure"

  Scenario: A successful critical action is audited
    Given a user with a valid "admin" token
    When I delete the collection "a_collection_to_delete"
    Then the audit log should contain an event with type "CollectionDelete"
