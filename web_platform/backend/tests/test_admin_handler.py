import unittest
import json
from unittest.mock import MagicMock
import os
import sys

# Add project root to path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from web_platform.backend.handlers.admin_handler import handle_admin

class TestAdminHandler(unittest.TestCase):
    def setUp(self):
        self.handler = MagicMock()
        self.handler.wfile = MagicMock()
        self.handler.command = 'GET'

    def test_handle_get_config(self):
        result = handle_admin(self.handler, '/api/config', {}, 'GET')
        self.assertTrue(result)
        self.handler.send_response.assert_called_with(200)

    def test_handle_unknown_path(self):
        result = handle_admin(self.handler, '/api/unknown', {}, 'GET')
        self.assertFalse(result)

if __name__ == '__main__':
    unittest.main()
