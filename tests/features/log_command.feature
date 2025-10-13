Feature: TissLang LOG Command

  Scenario: Parse a simple LOG command
    Given I have a TissLang script with content
      """
      STEP "Log test" {
        LOG "This is a log message"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should have a description "Log test"
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "This is a log message"