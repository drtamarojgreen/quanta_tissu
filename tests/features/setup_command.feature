Feature: TissLang SETUP Command

  Scenario: Parse a simple SETUP command
    Given I have a TissLang script with content:
      """
      SETUP "Initial setup for project" {
        RUN "npm install"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a SETUP node at index 0
    And the SETUP node should have a description "Initial setup for project"
    And the SETUP node should contain 1 commands
    And the command at index 0 should be of type RUN
    And the RUN command should have command "npm install"