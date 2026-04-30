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

if __name__ == '__main__':
    unittest.main()
