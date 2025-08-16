Feature: Generate
  Scenario: Generating text with a prompt
    Given a model and tokenizer
    When I generate 5 new tokens with the prompt "hello"
    Then the generated tokens should be a list of 5 integers
