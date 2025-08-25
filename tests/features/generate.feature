Feature: Generate
  Scenario: Generating text with a prompt using greedy decoding
    Given a model and tokenizer
    When I generate 5 new tokens with the prompt "hello" using the "greedy" method
    Then the generated tokens should be a list of 5 integers

  Scenario: Generating text with a prompt using sampling
    Given a model and tokenizer
    When I generate 5 new tokens with the prompt "hello" using the "sampling" method
    Then the generated tokens should be a list of 5 integers

  Scenario: Generating text with a temperature
    Given a model and tokenizer
    When I generate 5 new tokens with the prompt "hello" using the "sampling" method and temperature 0.8
    Then the generated tokens should be a list of 5 integers

  Scenario: Generating zero tokens
    Given a model and tokenizer
    When I generate 0 new tokens with the prompt "hello" using the "greedy" method
    Then the generated tokens should be a list of 0 integers
