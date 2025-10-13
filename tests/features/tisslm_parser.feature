Feature: TissLM Parser

  Scenario: Parse a simple TissLM script
    Given I have a TissLang script with content:
      """
      STEP "Log Hello" {
        LOG "Hello, TissLM!"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "Hello, TissLM!"
