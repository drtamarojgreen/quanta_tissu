Feature: TissLM Parser

  Scenario: Parse a simple TissLM script
    Given I have a TissLM script with content
      """
      PRINT "Hello, TissLM!"
      """
    When I parse the script
    Then the parser should produce an Abstract Syntax Tree (AST)
    And the AST should contain a print statement with the text "Hello, TissLM!"
