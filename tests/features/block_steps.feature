Feature: TissLang Block Steps (PARALLEL, CHOOSE, ESTIMATE_COST)

  Scenario: Parse a PARALLEL step
    Given I have a TissLang script with content:
      """
      PARALLEL {
        RUN "command1"
        RUN "command2"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a PARALLEL node at index 0
    And the PARALLEL node should contain 2 commands
    And the command at index 0 should be of type RUN
    And the RUN command should have command "command1"
    And the command at index 1 should be of type RUN
    And the RUN command should have command "command2"

  Scenario: Parse a CHOOSE step
    Given I have a TissLang script with content:
      """
      CHOOSE {
        STEP "Option A" { RUN "option_a_cmd" }
        STEP "Option B" { RUN "option_b_cmd" }
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a CHOOSE node at index 0
    And the CHOOSE node should contain 2 commands
    And the command at index 0 should be of type STEP
    And the STEP node should have a description "Option A"
    And the command at index 1 should be of type STEP
    And the STEP node should have a description "Option B"

  Scenario: Parse an ESTIMATE_COST step
    Given I have a TissLang script with content:
      """
      ESTIMATE_COST {
        PROMPT_AGENT "Estimate cost of this task"
      }
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a ESTIMATE_COST node at index 0
    And the ESTIMATE_COST node should contain 1 commands
    And the command at index 0 should be of type PROMPT_AGENT
    And the PROMPT_AGENT command should have prompt "Estimate cost of this task"