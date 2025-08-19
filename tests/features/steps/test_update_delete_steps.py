import requests
import json
import re

BASE_URL = "http://localhost:8080"
DB_NAME = "testdb" # Use a consistent test database

def register_steps(runner):
    @runner.step(r'^Then the document with ID "(.*)" in "(.*)" should exist$')
    def document_should_exist(context, doc_id, collection_name):
        response = requests.get(f"{BASE_URL}/{DB_NAME}/{collection_name}/{doc_id}")
        assert response.status_code == 200, f"Expected document '{doc_id}' to exist, but it does not (status code: {response.status_code})."
