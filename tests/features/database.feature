Feature: Database
  Scenario: Putting and getting a key-value pair
    Given a database
    When I put the key "hello" with the value "world"
    And I get the value for the key "hello"
    Then the value should be "world"

  Scenario: Deleting a key-value pair
    Given a database
    And I put the key "hello" with the value "world"
    When I delete the key "hello"
    And I get the value for the key "hello"
    Then the value should be None
