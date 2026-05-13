Feature: Web Platform CLI API
  As a developer
  I want to interact with the QuantaTissu platform via a CLI API
  To automate testing and debugging of process management

  Scenario: Verify state persistence during tab navigation
    Given the platform is initialized
    When I switch to the "playground" tab
    And I generate a response with prompt "Hello greenhouse"
    And I switch to the "config" tab
    And I switch back to the "playground" tab
    Then the playground should still show "Hello greenhouse" as the last prompt
