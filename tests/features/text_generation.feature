Feature: Text Generation
  As a user, I want to be able to generate text from a prompt.

  Scenario: Generate the next word
    Given a trained QuantaTissu model
    And a prompt "hello"
    When I ask the model to predict the next token
    Then the model should return a single token ID

  Scenario: Generate a sequence of words
    Given a trained QuantaTissu model
    And a prompt "hello world"
    When I ask the model to generate a sequence of 5 tokens
    Then the model should return a sequence of 5 token IDs
