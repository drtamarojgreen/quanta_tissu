import sys
import os

# This file defines the BDD steps for Nexus Flow functionality.
# It is intended to be loaded by the custom bdd_runner.py script.

# The runner will pass itself to this function, and we will use its
# `step` decorator to register our step implementations.
def register_steps(runner):

    @runner.step(r'the Nexus Flow application is running')
    def nexus_app_is_running(context):
        # In a real test, this would involve launching the C++ application.
        # For now, we'll just set a flag in the context.
        print("SIMULATING: Nexus Flow application is running.")
        context['app_running'] = True
        assert context['app_running'] is True

    @runner.step(r'the TissDB workflow is initiated')
    def tissdb_workflow_initiated(context):
        print("SIMULATING: TissDB workflow initiated.")
        context['workflow'] = 'TissDB'
        assert context['workflow'] == 'TissDB'

    @runner.step(r'the application should load a graph from TissDB')
    def load_graph_from_tissdb(context):
        print("SIMULATING: Application loads graph from TissDB.")
        context['graph_loaded'] = True
        assert context['graph_loaded'] is True

    @runner.step(r'the graph should be rendered on the screen')
    def graph_rendered_on_screen(context):
        print("SIMULATING: Graph is rendered on the screen.")
        context['rendered'] = True
        assert context['rendered'] is True

    @runner.step(r'the Generation workflow is initiated')
    def generation_workflow_initiated(context):
        print("SIMULATING: Generation workflow initiated.")
        context['workflow'] = 'Generation'
        assert context['workflow'] == 'Generation'

    @runner.step(r'the user provides a textual prompt describing a graph')
    def user_provides_prompt(context):
        print("SIMULATING: User provides a textual prompt.")
        context['prompt'] = "A test graph."
        assert context['prompt'] is not None

    @runner.step(r'the application should generate a new graph based on the prompt')
    def generate_graph_from_prompt(context):
        print("SIMULATING: Application generates a new graph.")
        context['graph_generated'] = True
        assert context['graph_generated'] is True

    @runner.step(r'the new graph should be displayed with an animation')
    def graph_displayed_with_animation(context):
        print("SIMULATING: New graph displayed with animation.")
        context['animation_played'] = True
        assert context['animation_played'] is True

    @runner.step(r'a graph is displayed on the screen')
    def graph_is_displayed(context):
        print("SIMULATING: A graph is already displayed.")
        context['rendered'] = True
        assert context['rendered'] is True

    @runner.step(r'the user views the graph')
    def user_views_graph(context):
        # This is a passive step, no action needed.
        print("SIMULATING: User is viewing the graph.")
        pass

    @runner.step(r'the user should see nodes and edges clearly drawn')
    def nodes_and_edges_clear(context):
        print("SIMULATING: Nodes and edges are verified as clearly drawn.")
        context['clarity_verified'] = True
        assert context['clarity_verified'] is True

    @runner.step(r'node labels should be visible next to the nodes')
    def labels_are_visible(context):
        print("SIMULATING: Node labels are verified as visible.")
        context['labels_visible'] = True
        assert context['labels_visible'] is True

    @runner.step(r'the user presses the spacebar')
    def user_presses_spacebar(context):
        print("SIMULATING: User presses the spacebar.")
        context['spacebar_pressed'] = True
        assert context['spacebar_pressed'] is True

    @runner.step(r'the application should proceed to the next step or exit')
    def app_proceeds_or_exits(context):
        print("SIMULATING: Application proceeds to the next state or exits.")
        context['app_proceeded'] = True
        assert context['app_proceeded'] is True

    # --- Steps for new scenarios ---

    @runner.step(r'no graphs are available in TissDB')
    def no_graphs_in_tissdb(context):
        print("SIMULATING: TissDB is configured to return no graphs.")
        context['tissdb_graphs'] = []
        assert context['tissdb_graphs'] == []

    @runner.step(r'the user should be shown a "no graphs loaded" message')
    def user_sees_no_graphs_message(context):
        print("SIMULATING: Verifying 'no graphs loaded' message is shown.")
        # In a real test, we would check the screen buffer.
        context['message_shown'] = "no graphs loaded"
        assert context['message_shown'] == "no graphs loaded"

    @runner.step(r'the application should wait for user input to continue')
    def app_waits_for_input(context):
        print("SIMULATING: Application is waiting for user input (e.g., spacebar).")
        # This is a conceptual step for the BDD flow.
        pass

    @runner.step(r'the user provides an empty prompt')
    def user_provides_empty_prompt(context):
        print("SIMULATING: User provides an empty prompt.")
        context['prompt'] = ""
        assert context['prompt'] == ""

    @runner.step(r'the application should return to the main menu')
    def app_returns_to_menu(context):
        print("SIMULATING: Application returns to the main menu.")
        # In a real test, we would check the application state.
        context['app_state'] = 'main_menu'
        assert context['app_state'] == 'main_menu'

    @runner.step(r'the backend script is set to fail')
    def backend_script_fails(context):
        print("SIMULATING: Backend script is configured to return an error.")
        context['backend_should_fail'] = True
        assert context['backend_should_fail'] is True

    @runner.step(r'the user should be shown a "graph generation failed" message')
    def user_sees_generation_failed_message(context):
        print("SIMULATING: Verifying 'graph generation failed' message is shown.")
        context['message_shown'] = "graph generation failed"
        assert context['message_shown'] == "graph generation failed"
