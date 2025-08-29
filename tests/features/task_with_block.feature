Feature: TissLang TASK with Block

  Scenario: Parse a TASK with an associated block
    Given I have a TissLang script with content:
      """
      TASK "My Task" {
        STEP "First step" {
          LOG "Starting task"
        }
        STEP "Second step" {
          RUN "do_something"
        }
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a TASK node at index 0
    And the TASK node should have a description "My Task"
    And the TASK node should contain 2 commands
    And the command at index 0 should be of type STEP
    And the STEP node should have a description "First step"
    And the command at index 1 should be of type STEP
    And the STEP node should have a description "Second step"