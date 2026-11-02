import requests

from tests.features.steps.test_database_steps import BASE_URL, get_headers


def register_steps(runner):
    @runner.step(r'I import this N-Triples document')
    def import_ntriples(context):
        response = requests.post(
            f"{BASE_URL}/{context['db_name']}/_ntriples",
            data=context['text'],
            headers=get_headers(context),
        )
        context['rdf_import_response'] = response

    @runner.step(r'the RDF import should report (\d+) new triples')
    def rdf_import_should_report(context, count):
        response = context['rdf_import_response']
        assert response.status_code == 200, f"Import failed: {response.status_code}: {response.text}"
        assert response.json().get('loaded') == int(count), response.text

    @runner.step(r'I execute this SPARQL query')
    def execute_sparql_query(context):
        response = requests.post(
            f"{BASE_URL}/{context['db_name']}/_query",
            json={'query': context['text']},
            headers=get_headers(context),
        )
        context['sparql_response'] = response
        context['sparql_results'] = response.json() if response.ok else []

    @runner.step(r'the SPARQL results should contain the binding "([^"]+)" = \'(.*)\'')
    def sparql_result_should_contain_binding(context, key, value):
        response = context['sparql_response']
        assert response.status_code == 200, f"Query failed: {response.status_code}: {response.text}"
        assert any(row.get(key) == value for row in context['sparql_results']), (
            f"Expected {key}={value!r} in {context['sparql_results']}"
        )

    @runner.step(r'the SPARQL results should omit the binding "([^"]+)" for "([^"]+)"')
    def sparql_result_should_omit_optional_binding(context, key, value):
        assert any(row.get('name') == f'"{value}"' and key not in row
                   for row in context['sparql_results']), context['sparql_results']

    @runner.step(r'the SPARQL results should be empty')
    def sparql_results_should_be_empty(context):
        response = context['sparql_response']
        assert response.status_code == 200, f"Query failed: {response.status_code}: {response.text}"
        assert context['sparql_results'] == [], context['sparql_results']

    @runner.step(r'the RDF import should fail with status (\d+)')
    def rdf_import_should_fail(context, status):
        response = context['rdf_import_response']
        assert response.status_code == int(status), f"Expected {status}, got {response.status_code}: {response.text}"

    @runner.step(r'the SPARQL query should fail with status (\d+)')
    def sparql_query_should_fail(context, status):
        response = context['sparql_response']
        assert response.status_code == int(status), f"Expected {status}, got {response.status_code}: {response.text}"
