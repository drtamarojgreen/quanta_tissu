Feature: C++ Tiss Language Model (ctisslm) High-Level Functionality

  As a developer, I want to use the ctisslm library to load models
  and generate text from prompts.

  Scenario: Successfully load model and tokenizer
    Given a valid model file exists at "models/ctisslm.bin"
    And a valid tokenizer file exists at "models/tokenizer.json"
    When an instance of ctisslm is created
    And the model is loaded from "models/ctisslm.bin"
    And the tokenizer is loaded from "models/tokenizer.json"
    Then the model and tokenizer should be loaded successfully

  Scenario: Fail to load a non-existent model
    Given no model file exists at "models/nonexistent.bin"
    When an instance of ctisslm is created
    And the application attempts to load the model from "models/nonexistent.bin"
    Then the model loading should fail

  Scenario: Generate text from a prompt
    Given a ctisslm instance with a loaded model and tokenizer
    When the application generates text from the prompt "Hello, world"
    Then it should return a non-empty generated string

  Scenario: Generate text with a custom max length
    Given a ctisslm instance with a loaded model and tokenizer
    When the application generates text from the prompt "Once upon a time" with a max_length of 10
    Then the generated text should have at most 10 tokens

  Scenario: Tokenizer encodes and decodes text
    Given a loaded ctisslm tokenizer
    When the text "hello" is encoded
    Then it should produce a sequence of token IDs
    When the token IDs are decoded
    Then the result should be the original text "hello"
