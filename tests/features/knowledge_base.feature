Feature: Knowledge Base
  Scenario: Adding and retrieving a document
    Given a knowledge base with a model and tokenizer
    When I add the document "The quick brown fox jumps over the lazy dog."
    And I retrieve documents for the query "quick fox"
    Then the retrieved documents should contain "The quick brown fox jumps over the lazy dog."

  Scenario: Adding feedback to a document
    Given a knowledge base with a model and tokenizer
    And I add the document "This is a test document."
    When I retrieve documents for the query "test"
    And I add feedback with score 5 and text "Very relevant" for the retrieved documents
    Then the knowledge base stats should show 1 feedback entry

  Scenario: Self-updating from interaction with user correction
    Given a knowledge base with a model and tokenizer
    When I self-update from interaction with query "What is the capital of France?" generated response "London" and user correction "Paris"
    Then the knowledge base should contain "Query: What is the capital of France? Correct Answer: Paris"

  Scenario: Self-updating from interaction without user correction
    Given a knowledge base with a model and tokenizer
    When I self-update from interaction with query "What is the capital of France?" generated response "Paris" and no user correction
    Then the knowledge base should contain "Query: What is the capital of France? Response: Paris"

  Scenario: Getting knowledge base statistics
    Given a knowledge base with a model and tokenizer
    When I add the document "Document 1"
    And I add the document "Document 2"
    And I retrieve documents for the query "Document 1"
    Then the knowledge base stats should show 2 total documents
    And the knowledge base stats should show 1 total access
