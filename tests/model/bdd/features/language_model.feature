Feature: Comprehensive Language Model Tests

  # --- Core Functionality ---

  Scenario: Using the KV cache provides a performance improvement without changing the output
    Given a trained language model and tokenizer
    And the prompt "The laws of physics state that"
    When I generate 20 tokens without the KV cache
    And I generate 20 tokens with the KV cache
    Then the generated text with and without the cache should be identical
    And the generation with the KV cache should be faster than without it

  Scenario Outline: The model can generate text using different standard sampling methods
    Given a trained language model and tokenizer
    When I generate <length> tokens from the prompt "<prompt>" using the "<method>" sampling method
    Then the generated text should not be empty
    And the generated text should contain a plausible number of words based on the token length

    Examples:
      | prompt                                   | method  | length |
      | The meaning of life is                   | greedy  | 30     |
      | The future of humanity is                | nucleus | 30     |
      | In a shocking turn of events, scientists | top_k   | 30     |

  Scenario: The RuleEnforcer can clean up generated text
    Given a trained language model and tokenizer
    And the prompt "this is a test. it has some repeated words. words words. and missing punctuation"
    When I generate 50 tokens from the prompt
    And I apply the RuleEnforcer to the generated text
    Then the cleaned text should be different from the raw generated text
    And the cleaned text should not contain "words words"

  # --- RAG and Knowledge Base ---

  Scenario: The model can answer a question using information from the knowledge base
    Given a trained language model and tokenizer
    And a running TissDB instance for the knowledge base
    And an empty knowledge base collection named "bdd_rag_knowledge"
    And the knowledge base contains a document with id "mars_mission" and content "The first manned mission to Mars, named 'Ares 1', is scheduled for 2035."
    When I ask the model the question "What is the name of the first Mars mission?"
    Then the model should generate an answer containing "Ares 1"

  Scenario: The model can learn from a user interaction and update the knowledge base
    Given a trained language model and tokenizer
    And a running TissDB instance for the knowledge base
    And an empty knowledge base collection named "bdd_rag_knowledge_update"
    When I ask the model the question "What is the capital of the fictional country Eldoria?"
    And the model generates the answer "The capital of Eldoria is Silverhaven."
    And the model stores this interaction in the knowledge base
    Then the knowledge base should contain a document about "Eldoria"

  # --- Advanced Sampling ---

  Scenario Outline: The model can generate text with a specific sentiment
    Given a trained language model and tokenizer
    And a sentiment analyzer
    When I generate 30 tokens from the prompt "<prompt>" with a "<sentiment>" sentiment of strength 0.8
    Then the generated text should have a "<sentiment>" sentiment

    Examples:
      | prompt              | sentiment |
      | I feel very         | positive  |
      | This is a very      | negative  |
      | The situation is    | neutral   |
