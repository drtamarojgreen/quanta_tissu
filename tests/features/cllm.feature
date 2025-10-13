Feature: C++ Language Model (cllm) Core Functionality

  As a developer using the cllm library, I want to ensure the core model
  initializes correctly and performs a forward pass as expected.

  Scenario: Initialize model with default configuration
    Given the cllm library
    When a model is initialized with the default configuration
    Then the model should be created successfully
    And its configuration should match the default values

  Scenario: Initialize model with a custom configuration
    Given the cllm library
    When a model is initialized with a custom configuration:
      | d_model     | 256 |
      | n_heads     | 4   |
      | n_layers    | 2   |
      | vocab_size  | 1000|
      | max_seq_len | 128 |
    Then the model should be created successfully
    And its configuration should match the custom values

  Scenario: Perform a forward pass
    Given a cllm model is initialized
    When a forward pass is performed with a sequence of 10 tokens
    Then the model should return a logits matrix
    And the logits matrix should have 10 rows
    And the logits matrix should have a number of columns equal to the vocab_size

  Scenario: Perform a forward pass with a long sequence
    Given a cllm model is initialized
    When a forward pass is performed with a sequence of 512 tokens
    Then the model should return a logits matrix
    And the logits matrix should have 512 rows
