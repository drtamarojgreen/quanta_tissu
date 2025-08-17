Feature: TissDB and TissLM Integration

  This feature tests the integration points between TissDB and TissLM,
  specifically focusing on the Retrieval-Augmented Generation (RAG) pipeline
  where TissLM retrieves contextual information from TissDB.

  Scenario: TissLM KnowledgeBase retrieves context from TissDB for a user prompt
    Given a running TissDB instance
    And a TissDB collection named "knowledge_articles" is available for TissLM
    And the "knowledge_articles" collection contains a document with ID "lsm_doc" and content {"title": "LSM Tree", "body": "A Log-Structured Merge-Tree is a data structure designed for efficient writes."}
    And the "knowledge_articles" collection contains a document with ID "b_tree_doc" and content {"title": "B-Tree", "body": "A B-Tree is a self-balancing tree data structure that maintains sorted data."}

    When the TissLM receives a user prompt "Tell me about LSM Trees"
    And the TissLM KnowledgeBase formulates a TissQL query "SELECT body FROM knowledge_articles WHERE title = 'LSM Tree'"
    And the KnowledgeBase executes the query against the "knowledge_articles" collection
    Then the query result for the KnowledgeBase should contain "A Log-Structured Merge-Tree is a data structure designed for efficient writes."
    And the query result for the KnowledgeBase should not contain "A B-Tree is a self-balancing tree data structure that maintains sorted data."

  Scenario: TissLM generates an augmented prompt using retrieved context
    Given a user prompt "What is a Log-Structured Merge-Tree?"
    And a retrieved context from TissDB: "A Log-Structured Merge-Tree (LSM-Tree) is a data structure with performance characteristics that make it attractive for write-heavy workloads."

    When the TissLM augments the prompt with the retrieved context
    Then the final prompt sent to the language model should be:
      """
      context: {A Log-Structured Merge-Tree (LSM-Tree) is a data structure with performance characteristics that make it attractive for write-heavy workloads.} question: {What is a Log-Structured Merge-Tree?}
      """

  Scenario: Sinew C++ client interaction pattern with TissDB
    # This scenario simulates the interaction of a C++ application using the Sinew client.
    # The test is performed at the API level, verifying that the TissDB server
    # responds correctly to requests that a Sinew client would make.
    Given a running TissDB instance
    And a collection named "cpp_app_data" exists

    When a simulated Sinew client creates a document with ID "sinew_doc" and content {"source": "C++ App", "value": 42} in "cpp_app_data"
    Then the document with ID "sinew_doc" in "cpp_app_data" should have content {"source": "C++ App", "value": 42}
    When a simulated Sinew client deletes the document with ID "sinew_doc" from "cpp_app_data"
    Then the document with ID "sinew_doc" in "cpp_app_data" should not exist
    And I delete the collection "cpp_app_data"
