Feature: TissLang Directives

  Scenario: Parse program-level directive
    Given I have a TissLang script with content:
      """
      @persona "senior_developer"
      TASK "Test task" {}
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a directive named "persona" with value "senior_developer"

  Scenario: Parse in-step directive
    Given I have a TissLang script with content:
      """
      STEP "Test step with directive" {
        @output "json"
        PROMPT_AGENT "Generate JSON"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 2 commands
    And the command at index 0 should be of type DIRECTIVE
    And the DIRECTIVE command should have name "output"
    And the DIRECTIVE command should have value "json"