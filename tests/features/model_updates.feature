Feature: Model Updates Core Functionality

  As a developer, I want to ensure the new model features are integrated
  and behave as expected.

  Scenario: Initialize model with the new attention mechanism
    Given a model is configured to use the "new_attention" mechanism
    When the model is initialized
    Then the model should be configured to use the new attention mechanism

  Scenario: Perform a forward pass with the new activation function
    Given a model is configured with the "new_relu" activation function
    And the model is initialized
    When a forward pass is performed with a sequence of 10 tokens
    Then the model should return a valid logits matrix for the new activation function