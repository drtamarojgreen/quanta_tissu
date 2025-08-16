Feature: Predict
  Scenario: Predicting the next token
    Given a model and tokenizer
    When I predict the next token for the prompt "hello"
    Then the next token should be a valid token id
