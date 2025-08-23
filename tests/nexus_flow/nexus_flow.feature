Feature: Nexus Flow Graph Visualization
  As a user, I want to visualize and interact with graphs
  so that I can understand complex data relationships.

  Scenario: Visualizing an existing graph from TissDB
    Given the Nexus Flow application is running
    When the TissDB workflow is initiated
    Then the application should load a graph from TissDB
    And the graph should be rendered on the screen

  Scenario: Generating a new graph from a user prompt
    Given the Nexus Flow application is running
    When the Generation workflow is initiated
    And the user provides a textual prompt describing a graph
    Then the application should generate a new graph based on the prompt
    And the new graph should be displayed with an animation

  Scenario: Interacting with a displayed graph
    Given a graph is displayed on the screen
    When the user views the graph
    Then the user should see nodes and edges clearly drawn
    And node labels should be visible next to the nodes
    When the user presses the spacebar
    Then the application should proceed to the next step or exit
