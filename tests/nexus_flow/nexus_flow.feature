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

  Scenario: TissDB workflow with no graphs
    Given the Nexus Flow application is running
    And no graphs are available in TissDB
    When the TissDB workflow is initiated
    Then the user should be shown a "no graphs loaded" message
    And the application should wait for user input to continue

  Scenario: Generating a graph with an empty prompt
    Given the Nexus Flow application is running
    When the Generation workflow is initiated
    And the user provides an empty prompt
    Then the application should return to the main menu

  Scenario: Graph generation fails
    Given the Nexus Flow application is running
    When the Generation workflow is initiated
    And the backend script is set to fail
    And the user provides a textual prompt describing a graph
    Then the user should be shown a "graph generation failed" message
    And the application should wait for user input to continue
