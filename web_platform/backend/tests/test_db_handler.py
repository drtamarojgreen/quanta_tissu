import unittest
import json
from unittest.mock import MagicMock, patch
import os
import sys

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from web_platform.backend.handlers.db_handler import handle_db

class TestDbHandler(unittest.TestCase):
    def setUp(self):
        self.handler = MagicMock()
        self.handler.wfile = MagicMock()

    @patch('urllib.request.urlopen')
    def test_handle_db_databases(self, mock_urlopen):
        # Mock TissDB response
        mock_response = MagicMock()
        mock_response.status = 200
        mock_response.getheaders.return_value = [('Content-Type', 'application/json')]
        mock_response.read.return_value = json.dumps(['main_db']).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

        result = handle_db(self.handler, '/api/db/databases', {}, 'GET')
        self.assertTrue(result)
        self.handler.send_response.assert_called_with(200)

    def _mock_query_response(self, mock_urlopen, body):
        mock_response = MagicMock()
        mock_response.status = 200
        mock_response.getheaders.return_value = [('Content-Type', 'application/json')]
        mock_response.read.return_value = json.dumps(body).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

    @patch('urllib.request.urlopen')
    def test_query_without_collection_runs_against_database(self, mock_urlopen):
        rows = [{'_id': '0', 's': '<http://example.org/alice>'}]
        self._mock_query_response(mock_urlopen, rows)
        query = 'PREFIX ex: <http://example.org/> SELECT ?s WHERE { ?s ex:knows ?o }'

        result = handle_db(self.handler, '/api/db/query', {'db_name': 'rdfdb', 'query': query}, 'POST')

        self.assertTrue(result)
        request = mock_urlopen.call_args[0][0]
        self.assertTrue(request.full_url.endswith('/rdfdb/_query'))
        self.assertEqual(request.get_method(), 'POST')
        self.assertEqual(json.loads(request.data.decode('utf-8')), {'query': query})
        self.handler.send_response.assert_called_with(200)
        self.handler.wfile.write.assert_called_with(json.dumps(rows).encode('utf-8'))

    @patch('urllib.request.urlopen')
    def test_query_without_collection_defaults_to_main_db(self, mock_urlopen):
        self._mock_query_response(mock_urlopen, [])
        handle_db(self.handler, '/api/db/query', {'query': 'SELECT * FROM knowledge'}, 'POST')
        self.assertTrue(mock_urlopen.call_args[0][0].full_url.endswith('/main_db/_query'))

    @patch('urllib.request.urlopen')
    def test_query_with_collection_keeps_collection_route(self, mock_urlopen):
        self._mock_query_response(mock_urlopen, [])
        handle_db(self.handler, '/api/db/query', {'collection': 'knowledge', 'query': 'SELECT * FROM knowledge'}, 'POST')
        self.assertTrue(mock_urlopen.call_args[0][0].full_url.endswith('/main_db/knowledge/_query'))

    @patch('urllib.request.urlopen')
    def test_ntriples_upload_is_forwarded_to_database(self, mock_urlopen):
        self._mock_query_response(mock_urlopen, {'loaded': 2})
        ntriples = ('<http://example.org/a> <http://example.org/p> "x" .\n'
                    '<http://example.org/a> <http://example.org/q> <http://example.org/b> .\n')

        result = handle_db(self.handler, '/api/db/ntriples', {'db_name': 'rdfdb', 'data': ntriples}, 'POST')

        self.assertTrue(result)
        request = mock_urlopen.call_args[0][0]
        self.assertTrue(request.full_url.endswith('/rdfdb/_ntriples'))
        self.assertEqual(request.get_method(), 'POST')
        self.assertEqual(request.data.decode('utf-8'), ntriples)
        self.handler.send_response.assert_called_with(200)
        self.handler.wfile.write.assert_called_with(json.dumps({'loaded': 2}).encode('utf-8'))

    @patch('urllib.request.urlopen')
    def test_ntriples_upload_defaults_to_main_db(self, mock_urlopen):
        self._mock_query_response(mock_urlopen, {'loaded': 0})
        handle_db(self.handler, '/api/db/ntriples', {'data': ''}, 'POST')
        self.assertTrue(mock_urlopen.call_args[0][0].full_url.endswith('/main_db/_ntriples'))

    @patch('urllib.request.urlopen')
    def test_ntriples_upload_without_data_is_rejected(self, mock_urlopen):
        result = handle_db(self.handler, '/api/db/ntriples', {'db_name': 'rdfdb'}, 'POST')
        self.assertTrue(result)
        mock_urlopen.assert_not_called()
        self.handler.send_response.assert_called_with(400)

if __name__ == '__main__':
    unittest.main()
