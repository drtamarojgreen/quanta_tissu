import unittest
import os
import sys
from unittest.mock import patch, MagicMock

# Add the project root to sys.path to import the CLI
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
if PROJECT_ROOT not in sys.path:
    sys.path.append(PROJECT_ROOT)

from web_platform.dev.cli import main

class TestDockerConfig(unittest.TestCase):
    def test_dockerfile_exists(self):
        dockerfile_path = os.path.join(PROJECT_ROOT, 'web_platform/dev/Dockerfile')
        self.assertTrue(os.path.exists(dockerfile_path), "Dockerfile should exist")

    def test_dockerfile_content(self):
        dockerfile_path = os.path.join(PROJECT_ROOT, 'web_platform/dev/Dockerfile')
        with open(dockerfile_path, 'r') as f:
            content = f.read()
            self.assertIn("FROM python:3.12-alpine", content)
            self.assertIn("apk add --no-cache", content)
            self.assertIn("ENV PYTHONPATH=/app", content)

    @patch('web_platform.dev.cli.requests.get')
    def test_cli_db_status(self, mock_get):
        # Mocking the API response
        mock_resp = MagicMock()
        mock_resp.status_code = 200
        mock_resp.json.return_value = {"databases": ["main_db"]}
        mock_get.return_value = mock_resp

        # Test running the CLI with 'db status'
        with patch('sys.argv', ['cli.py', 'db', 'status']):
            with patch('sys.stdout', new=MagicMock()) as mock_stdout:
                main()
                # Check if requests.get was called with the correct URL
                self.assertTrue(mock_get.called)
                args, kwargs = mock_get.call_args
                self.assertIn('/api/db/databases', args[0])

    @patch('web_platform.dev.cli.requests.post')
    def test_cli_model_generate(self, mock_post):
        mock_resp = MagicMock()
        mock_resp.status_code = 200
        mock_resp.json.return_value = {"generated_text": "hello"}
        mock_post.return_value = mock_resp

        with patch('sys.argv', ['cli.py', 'model', 'hello']):
            with patch('sys.stdout', new=MagicMock()) as mock_stdout:
                main()
                self.assertTrue(mock_post.called)
                args, kwargs = mock_post.call_args
                self.assertIn('/api/model/generate', args[0])
                self.assertEqual(kwargs['json']['prompt'], 'hello')

if __name__ == '__main__':
    unittest.main()
