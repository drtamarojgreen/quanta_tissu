Feature: TissLang REQUEST_REVIEW Command

  Scenario: Parse a REQUEST_REVIEW command
    Given I have a TissLang script with content:
      """
      STEP "Review test" {
        REQUEST_REVIEW "Please review this code change."
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type REQUEST_REVIEW
    And the REQUEST_REVIEW command should have message "Please review this code change."