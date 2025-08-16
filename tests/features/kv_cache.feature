Feature: KV Cache
  Scenario: Generating with KV cache produces the same output as generating without it
    Given a model and tokenizer
    When I generate text with a prompt "hello" and 5 new tokens using the KV cache
    And I generate text with the same prompt and new tokens without the KV cache
    Then the generated tokens should be the same
