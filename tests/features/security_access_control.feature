Feature: Security - Access Control

  Background:
    Given a running TissDB instance

  Scenario: Attempt to access a protected endpoint without a token
    When I make a GET request to the protected endpoint "/_databases" without a token
    Then the response status code should be 401
    And the response body should contain "Authorization header missing"

  Scenario: Attempt to access a protected endpoint with an invalid token
    When I make a GET request to the protected endpoint "/_databases" with an invalid token
    Then the response status code should be 401
    And the response body should contain "Invalid or missing bearer token"

  Scenario: Successfully access a protected endpoint with a valid admin token
    When I make a GET request to the protected endpoint "/_databases" with a valid "admin" token
    Then the response status code should be 200

  Scenario: A non-admin user is forbidden from deleting a database
    Given a user with a valid "read_only" token
    When I attempt to DELETE the database
    Then the response status code should be 403
    And the response body should contain "You do not have permission to delete a database"
