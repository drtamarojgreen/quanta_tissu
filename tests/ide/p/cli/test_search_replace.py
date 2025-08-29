import unittest
import os
import sys
from unittest.mock import patch, mock_open

# Add the parent directory of `quanta_tissu` to the Python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../../')))

from quanta_tissu.ide.p.cli import search_replace

class TestSearchReplace(unittest.TestCase):

    @patch('sys.stdout')
    @patch('quanta_tissu.ide.p.cli.search_replace.search_and_replace_in_file')
    @patch('quanta_tissu.ide.p.cli.search_replace.find_files')
    def test_exclude_argument_filters_files(self, mock_find_files, mock_search_and_replace, mock_stdout):
        """
        Tests that the --exclude argument correctly filters out files from the search list.
        """
        # Arrange
        # Mock find_files to return a specific list of files
        mock_find_files.return_value = ['file1.py', 'file2.txt', 'data.log', 'src/component.py', 'src/component.log']

        # Mock search_and_replace to avoid actual file operations
        mock_search_and_replace.return_value = (True, "content", "modified_content")

        # Simulate command line arguments including the new --exclude flag
        test_args = [
            'search_replace.py',
            '--pattern', 'test',
            '--files', '**/*',
            '--exclude', '*.log' # Exclude all log files
        ]

        # Act
        with patch.object(sys, 'argv', test_args):
            try:
                search_replace.main()
            except SystemExit:
                # Argparse calls sys.exit on error, catch it for the test
                pass

        # Assert
        # Get the list of files that were actually passed to the search function
        searched_files = [call.args[0] for call in mock_search_and_replace.call_args_list]

        # Check that the non-excluded files were searched
        self.assertIn('file1.py', searched_files)
        self.assertIn('file2.txt', searched_files)
        self.assertIn('src/component.py', searched_files)

        # Check that the excluded files were NOT searched
        self.assertNotIn('data.log', searched_files)
        self.assertNotIn('src/component.log', searched_files)


if __name__ == '__main__':
    unittest.main()
