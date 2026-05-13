import json
import subprocess
import os

def register_steps(runner):
    @runner.step(r'the platform is initialized')
    def step_platform_initialized(context):
        # We assume the CLI API is available via node
        context['api_cmd'] = ['node', 'web_platform/frontend/js/api/platform_api.js']
        subprocess.check_output(context['api_cmd'] + ['clear_state'])
        pass

    @runner.step(r'I switch to the "([^"]*)" tab')
    def step_switch_tab(context, tab_name):
        res = subprocess.check_output(context['api_cmd'] + ['switch_tab', json.dumps(tab_name)])
        data = json.loads(res)
        assert data['active_tab'] == tab_name

    @runner.step(r'I generate a response with prompt "([^"]*)"')
    def step_generate_response(context, prompt):
        # We need to make sure the backend is running for this to actually work
        # but the CLI API mocks the response if it fails to reach backend or we can mock it here
        # For BDD, let's assume backend is running as per our plan.
        res = subprocess.check_output(context['api_cmd'] + ['generate', json.dumps(prompt)])
        data = json.loads(res)
        assert 'response' in data

    @runner.step(r'I switch back to the "playground" tab')
    def step_switch_back(context):
        res = subprocess.check_output(context['api_cmd'] + ['switch_tab', '"playground"'])
        data = json.loads(res)
        assert data['active_tab'] == 'playground'

    @runner.step(r'the playground should still show "([^"]*)" as the last prompt')
    def step_verify_persistence(context, expected_prompt):
        # We need to pass JSON arguments carefully to node from python
        # Using double quotes inside single quotes for the shell
        res = subprocess.check_output(context['api_cmd'] + ['get_module_state', '"ModelModule"'])
        mstate = json.loads(res)
        # In Node, lastPrompt might be an empty string if generate was mocked or failed
        # Let's verify what we got
        if mstate.get('lastPrompt') != expected_prompt:
             print(f"DEBUG: expected {expected_prompt}, got {mstate.get('lastPrompt')}")
        assert mstate['lastPrompt'] == expected_prompt
