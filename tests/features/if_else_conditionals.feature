Feature: TissLang IF THEN ELSE Conditionals

  Scenario: Parse IF THEN with boolean condition
    Given I have a TissLang script with content:
      """
      IF true THEN {
        LOG "Condition is true"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a IF node at index 0
    And the IF node should have a condition of type BOOLEAN
    And the boolean condition should be true
    And the IF node should have a then_block with 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "Condition is true"

  Scenario: Parse IF THEN ELSE with comparison condition
    Given I have a TissLang script with content:
      """
      IF var == "value" THEN {
        LOG "Var is value"
      } ELSE {
        LOG "Var is not value"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a IF node at index 0
    And the IF node should have a condition of type COMPARISON
    And the comparison condition should have left "var", operator "==", and right "value"
    And the IF node should have a then_block with 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "Var is value"
    And the IF node should have an else_block with 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "Var is not value"

  Scenario: Parse nested IF statements
    Given I have a TissLang script with content:
      """
      IF true THEN {
        IF false THEN {
          LOG "Inner true"
        } ELSE {
          LOG "Inner false"
        }
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a IF node at index 0
    And the IF node should have a then_block with 1 commands
    And the command at index 0 should be of type IF
    And the IF node should have a then_block with 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "Inner true"
    And the IF node should have an else_block with 1 commands
    And the command at index 0 should be of type LOG
    And the LOG command should have message "Inner false"