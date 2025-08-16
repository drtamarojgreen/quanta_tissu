import requests
import json

BASE_URL = "http://localhost:8080"

def register_steps(runner):
    @runner.step(r'^Given a database$')
    def context(context):
        # The database is started by the runner
        pass

    @runner.step(r'^When I put the key "(.*)" with the value "(.*)"$')
    def put_key_value(context, key, value):
        data = {"key": key, "value": value}
        response = requests.post(f"{BASE_URL}/documents", json=data)
        assert response.status_code == 201

    @runner.step(r'^And I get the value for the key "(.*)"$')
    def get_value(context, key):
        response = requests.get(f"{BASE_URL}/documents/{key}")
        if response.status_code == 200:
            context['value'] = response.json()['value']
        else:
            context['value'] = None

    @runner.step(r'^Then the value should be "(.*)"$')
    def check_value(context, expected_value):
        assert context['value'] == expected_value
        return "Test passed!"

    @runner.step(r'^When I delete the key "(.*)"$')
    def delete_key(context, key):
        response = requests.delete(f"{BASE_URL}/documents/{key}")
        assert response.status_code == 204

    @runner.step(r'^Then the value should be None$')
    def check_value_is_none(context):
        assert context['value'] is None
        return "Test passed!"