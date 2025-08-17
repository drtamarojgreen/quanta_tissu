Feature: Tokenizer
  Scenario: Tokenizing and detokenizing a simple string should result in the original string
    Given a tokenizer
    When I tokenize the string "hello world"
    And I detokenize the tokens
    Then the resulting string should be "hello world"

  Scenario: Tokenizing an empty string
    Given a tokenizer
    When I tokenize the string ""
    Then the result should be an empty list of tokens

  Scenario: Tokenizing and detokenizing a string with special characters
    Given a tokenizer
    When I tokenize the string "hello, world! 123? <test>"
    And I detokenize the tokens
    Then the resulting string should be "hello, world! 123? <test>"
