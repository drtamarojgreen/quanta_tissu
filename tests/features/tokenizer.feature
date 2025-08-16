Feature: Tokenizer
  Scenario: Tokenizing and detokenizing a string should result in the original string
    Given a tokenizer
    When I tokenize the string "hello world"
    And I detokenize the tokens
    Then the resulting string should be "hello world"
