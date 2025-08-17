import requests
import json

BASE_URL = "http://localhost:8080"

def register_steps(runner):

    @runner.step(r'^Then the query result should be empty$')
    def query_result_should_be_empty(context):
        assert isinstance(context['query_result'], list), f"Query result is not a list: {context['query_result']}"
        assert len(context['query_result']) == 0, f"Query result is not empty: {context['query_result']}"
