Feature: Tissu Sinew C++ Connector for TissDB

  As a C++ developer, I want to use Tissu Sinew to reliably connect to and interact with TissDB,
  so that I can build applications on top of the database.

  Scenario: Successfully establishing a connection
    Given a TissDB server is running
    When a C++ application configures a TissuClient with the correct host and port
    And the application creates a TissuClient
    Then the TissuClient should be created successfully
    And the client should be able to get a session from the connection pool

  Scenario: Failing to establish a connection
    Given no TissDB server is running
    When a C++ application configures a TissuClient with the correct host and port
    And the application attempts to create a TissuClient
    Then the client creation should fail with a TissuConnectionException

  Scenario: Executing a simple query
    Given an application has a valid TissuSession
    When the application runs the query "PING"
    Then it should receive a TissuResult
    And the result should contain the string "PONG"

  Scenario: Executing a parameterized query
    Given an application has a valid TissuSession
    When the application runs the query "INSERT INTO users (name, age) VALUES ($name, $age)" with parameters:
      | name   | John Doe |
      | age    | 30       |
    Then the query should execute successfully
    And a log should indicate that the user "John Doe" was inserted

  Scenario: Successful transaction
    Given an application has a valid TissuSession
    When the application begins a transaction
    And runs the query "UPDATE accounts SET balance = balance - 100 WHERE id = 1"
    And runs the query "UPDATE accounts SET balance = balance + 100 WHERE id = 2"
    And commits the transaction
    Then the transaction should be committed successfully

  Scenario: Transaction rollback on error
    Given an application has a valid TissuSession
    When the application begins a transaction
    And runs the query "UPDATE accounts SET balance = balance - 100 WHERE id = 1"
    And runs a query that causes a TissuQueryException
    Then the transaction should be automatically rolled back
