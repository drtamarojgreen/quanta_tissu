import unittest
import os
import sys
from unittest.mock import patch, MagicMock, mock_open
import logging

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

# Mock system_config before importing train_bpe to avoid FileNotFoundError
with patch('quanta_tissu.tisslm.core.train_bpe.system_config') as mock_system_config:
    mock_system_config._project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
    mock_system_config.bpe_tokenizer_prefix = os.path.join(mock_system_config._project_root, "models", "test_trained_tokenizer")
    from quanta_tissu.tisslm.core.train_bpe import main

class TestTrainBPEScript(unittest.TestCase):

    def setUp(self):
        # Clear any existing handlers from root logger to prevent interference
        logging.basicConfig(level=logging.CRITICAL) # Suppress logging during tests

    @patch('argparse.ArgumentParser')
    @patch('quanta_tissu.tisslm.core.train_bpe.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isdir')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isfile')
    @patch('quanta_tissu.tisslm.core.train_bpe.glob.glob')
    @patch('builtins.open', new_callable=mock_open)
    @patch('quanta_tissu.tisslm.core.train_bpe.os.makedirs')
    @patch('builtins.print') # Mock print to capture output
    def test_train_bpe_with_directory_corpus(self, mock_print, mock_makedirs, mock_open_file, mock_glob, mock_isfile, mock_isdir, MockBPETokenizer, MockArgumentParser):
        # Setup mocks
        mock_isdir.return_value = True
        mock_isfile.return_value = False
        mock_glob.return_value = ["corpus/file1.txt", "corpus/file2.txt"]
        mock_open_file.side_effect = [mock_open(read_data="content of file1").return_value, mock_open(read_data="content of file2").return_value]

        mock_args = MagicMock()
        mock_args.corpus_path = "corpus_dir"
        mock_args.vocab_size = 500
        mock_args.save_prefix = "temp_output/test_prefix"
        mock_args.verbose = False
        MockArgumentParser.return_value.parse_args = MagicMock(return_value=mock_args)

        # Run the main function
        main()

        # Assertions
        mock_isdir.assert_called_once_with("corpus_dir")
        mock_glob.assert_called_once_with(os.path.join("corpus_dir", "*.txt"))
        self.assertEqual(mock_open_file.call_count, 2)
        MockBPETokenizer.assert_called_once_with()
        tokenizer_instance = MockBPETokenizer.return_value
        tokenizer_instance.train.assert_called_once_with("content of file1\ncontent of file2\n", 500, verbose=False)
        mock_makedirs.assert_called_once_with("temp_output", exist_ok=True)
        tokenizer_instance.save.assert_called_once_with("temp_output/test_prefix")

    @patch('argparse.ArgumentParser')
    @patch('quanta_tissu.tisslm.core.train_bpe.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isdir')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isfile')
    @patch('builtins.open', new_callable=mock_open)
    @patch('quanta_tissu.tisslm.core.train_bpe.os.makedirs') # Added patch
    @patch('builtins.print')
    def test_train_bpe_with_file_corpus(self, mock_print, mock_makedirs, mock_open_file, mock_isfile, mock_isdir, MockBPETokenizer, MockArgumentParser):
        mock_isdir.return_value = False
        mock_isfile.return_value = True
        mock_open_file.return_value = mock_open(read_data="single file content").return_value

        mock_args = MagicMock()
        mock_args.corpus_path = "single_file.txt"
        mock_args.vocab_size = 300
        mock_args.save_prefix = "temp_output/file_prefix"
        mock_args.verbose = True
        MockArgumentParser.return_value.parse_args = MagicMock(return_value=mock_args)

        main()

        mock_isfile.assert_called_once_with("single_file.txt")
        mock_open_file.assert_called_once_with("single_file.txt", "r", encoding="utf-8", errors="replace")
        tokenizer_instance = MockBPETokenizer.return_value
        tokenizer_instance.train.assert_called_once_with("single file content", 300, verbose=True)
        tokenizer_instance.save.assert_called_once_with("temp_output/file_prefix")

    @patch('argparse.ArgumentParser')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isdir')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isfile')
    @patch('builtins.print')
    def test_train_bpe_empty_corpus_dir(self, mock_print, mock_isfile, mock_isdir, MockArgumentParser):
        mock_isdir.return_value = True
        mock_isfile.return_value = False
        with patch('quanta_tissu.tisslm.core.train_bpe.glob.glob', return_value=[]): # No txt files found
            mock_args = MagicMock()
            mock_args.corpus_path = "empty_dir"
            mock_args.vocab_size = 500
            mock_args.save_prefix = "temp_output/test_prefix"
            mock_args.verbose = False
            MockArgumentParser.return_value.parse_args = MagicMock(return_value=mock_args)

            main()
            mock_print.assert_any_call("Error: No .txt files found in the corpus directory: empty_dir")

    @patch('argparse.ArgumentParser')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isdir')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isfile')
    @patch('builtins.open', new_callable=mock_open)
    @patch('builtins.print')
    def test_train_bpe_empty_corpus_file(self, mock_print, mock_open_file, mock_isfile, mock_isdir, MockArgumentParser):
        mock_isdir.return_value = False
        mock_isfile.return_value = True
        mock_open_file.return_value = mock_open(read_data="").return_value # Empty file

        mock_args = MagicMock()
        mock_args.corpus_path = "empty_file.txt"
        mock_args.vocab_size = 500
        mock_args.save_prefix = "temp_output/test_prefix"
        mock_args.verbose = False
        MockArgumentParser.return_value.parse_args = MagicMock(return_value=mock_args)

        main()
        mock_print.assert_any_call("Error: Corpus is empty after loading.")

    @patch('argparse.ArgumentParser')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isdir')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isfile')
    @patch('builtins.print')
    def test_train_bpe_invalid_corpus_path(self, mock_print, mock_isfile, mock_isdir, MockArgumentParser):
        mock_isdir.return_value = False
        mock_isfile.return_value = False

        mock_args = MagicMock()
        mock_args.corpus_path = "non_existent_path"
        mock_args.vocab_size = 500
        mock_args.save_prefix = "temp_output/test_prefix"
        mock_args.verbose = False
        MockArgumentParser.return_value.parse_args = MagicMock(return_value=mock_args)

        main()
        mock_print.assert_any_call("Error: Corpus path is neither a file nor a directory: non_existent_path")

    @patch('argparse.ArgumentParser')
    @patch('quanta_tissu.tisslm.core.train_bpe.BPETokenizer')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isdir')
    @patch('quanta_tissu.tisslm.core.train_bpe.os.path.isfile')
    @patch('quanta_tissu.tisslm.core.train_bpe.glob.glob')
    @patch('builtins.open', new_callable=mock_open)
    def test_train_bpe_save_load_verification(self, mock_open_file, mock_glob, mock_isfile, mock_isdir, MockBPETokenizer, MockArgumentParser):
        # Setup mocks for successful training and saving
        mock_isdir.return_value = True
        mock_isfile.return_value = False
        mock_glob.return_value = ["corpus/file1.txt"]
        mock_open_file.side_effect = [mock_open(read_data="sample text").return_value, # For corpus read
                                      mock_open().return_value, # For vocab save
                                      mock_open().return_value] # For merges save

        mock_args = MagicMock()
        mock_args.corpus_path = "corpus_dir"
        mock_args.vocab_size = 257
        mock_args.save_prefix = "temp_output/test_prefix"
        mock_args.verbose = False
        MockArgumentParser.return_value.parse_args = MagicMock(return_value=mock_args)

        # Mock BPETokenizer methods for save verification
        mock_tokenizer_instance = MockBPETokenizer.return_value
        mock_tokenizer_instance.vocab = {256: b'ab'}
        mock_tokenizer_instance.merges = {(97, 98): 256}
        mock_tokenizer_instance.encode.return_value = [256]
        mock_tokenizer_instance.decode.return_value = "sample text"

        # Only one mock instance needed as main() only creates one tokenizer
        MockBPETokenizer.return_value = mock_tokenizer_instance

        main()

        # Assert save was called correctly
        mock_tokenizer_instance.save.assert_called_once_with(mock_args.save_prefix)

        # Removed assertions related to loading as main() does not perform a load operation.
        # The BPETokenizer.load method is tested in test_bpe_trainer.py.

if __name__ == '__main__':
    import logging
    unittest.main()
