Feature: TissLang SET_BUDGET Command

  Scenario: Parse SET_BUDGET with string value
    Given I have a TissLang script with content:
      """
      STEP "Budget test" {
        SET_BUDGET EXECUTION_TIME = "5m"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type SET_BUDGET
    And the SET_BUDGET command should have variable "EXECUTION_TIME"
    And the SET_BUDGET command should have value "5m"

  Scenario: Parse SET_BUDGET with number value
    Given I have a TissLang script with content:
      """
      STEP "Budget test" {
        SET_BUDGET API_CALLS = 100
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type SET_BUDGET
    And the SET_BUDGET command should have variable "API_CALLS"
    And the SET_BUDGET command should have value "100"

  Scenario: Parse SET_BUDGET with boolean value
    Given I have a TissLang script with content:
      """
      STEP "Budget test" {
        SET_BUDGET ENABLE_LOGGING = true
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type SET_BUDGET
    And the SET_BUDGET command should have variable "ENABLE_LOGGING"
    And the SET_BUDGET command should have value "True"

  Scenario: Parse SET_BUDGET with object value
    Given I have a TissLang script with content:
      """
      STEP "Budget test" {
        SET_BUDGET CONFIG = {"key": "value", "num": 123}
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type SET_BUDGET
    And the SET_BUDGET command should have variable "CONFIG"
    And the SET_BUDGET command should have value "{\"key\": \"value\", \"num\": 123}"

  Scenario: Parse SET_BUDGET with list value
    Given I have a TissLang script with content:
      """
      STEP "Budget test" {
        SET_BUDGET ITEMS = ["item1", 2, true]
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a STEP node at index 0
    And the STEP node should contain 1 commands
    And the command at index 0 should be of type SET_BUDGET
    And the SET_BUDGET command should have variable "ITEMS"
    And the SET_BUDGET command should have value "[\"item1\", 2, true]"
