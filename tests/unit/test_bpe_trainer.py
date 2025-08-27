import unittest
import os
import json
import shutil
from collections import defaultdict
import numpy as np
import sys

# Add project root to path to allow importing quanta_tissu
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer, get_pairs

class TestBPETokenizer(unittest.TestCase):

    def setUp(self):
        self.test_dir = "./test_tokenizer_output"
        os.makedirs(self.test_dir, exist_ok=True)
        self.save_prefix = os.path.join(self.test_dir, "test_tokenizer")

    def tearDown(self):
        if os.path.exists(self.test_dir):
            # Manually remove directory and its contents
            for root, dirs, files in os.walk(self.test_dir, topdown=False):
                for name in files:
                    os.remove(os.path.join(root, name))
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
            os.rmdir(self.test_dir)

    def test_get_pairs(self):
        ids = [1, 2, 3, 1, 2]
        expected_pairs = {(1, 2), (2, 3), (3, 1)}
        self.assertEqual(get_pairs(ids), expected_pairs)

        ids_single = [1]
        self.assertEqual(get_pairs(ids_single), set())

        ids_empty = []
        self.assertEqual(get_pairs(ids_empty), set())

    def test_tokenizer_init(self):
        tokenizer = BPETokenizer()
        self.assertIsInstance(tokenizer.merges, dict)
        self.assertIsInstance(tokenizer.vocab, dict)
        self.assertIsInstance(tokenizer.reverse_vocab, dict)
        self.assertEqual(len(tokenizer.vocab), 0) # Should be empty before training/loading

    def test_tokenizer_train_basic(self):
        tokenizer = BPETokenizer()
        text = "aaabdaaabac"
        vocab_size = 258 # 256 initial bytes + 2 merges
        tokenizer.train(text, vocab_size)

        # Check vocab size
        self.assertEqual(len(tokenizer.vocab), vocab_size)

        # Check if merges were learned
        self.assertGreater(len(tokenizer.merges), 0)

        # Check encoding/decoding of a simple text
        encoded = tokenizer.encode("aaab")
        decoded = tokenizer.decode(encoded)
        self.assertEqual(decoded, "aaab")

    def test_tokenizer_train_no_merges(self):
        tokenizer = BPETokenizer()
        text = "abcde"
        vocab_size = 256 # Only initial bytes, no merges
        tokenizer.train(text, vocab_size)

        self.assertEqual(len(tokenizer.vocab), vocab_size)
        self.assertEqual(len(tokenizer.merges), 0)

        encoded = tokenizer.encode("abc")
        decoded = tokenizer.decode(encoded)
        self.assertEqual(decoded, "abc")

    def test_tokenizer_train_full_corpus(self):
        tokenizer = BPETokenizer()
        # Use a slightly larger corpus to ensure merges happen
        text = "apple banana apple orange banana apple"
        vocab_size = 260 # 256 initial + 4 merges
        tokenizer.train(text, vocab_size)

        self.assertGreater(len(tokenizer.merges), 0)
        self.assertGreater(len(tokenizer.vocab), 256)

        encoded = tokenizer.encode("apple banana")
        decoded = tokenizer.decode(encoded)
        self.assertEqual(decoded, "apple banana")

    def test_tokenizer_save_load(self):
        tokenizer = BPETokenizer()
        text = "test save and load functionality"
        vocab_size = 260
        tokenizer.train(text, vocab_size)
        tokenizer.save(self.save_prefix)

        # Check if files are created
        self.assertTrue(os.path.exists(f"{self.save_prefix}_vocab.json"))
        self.assertTrue(os.path.exists(f"{self.save_prefix}_merges.txt"))

        loaded_tokenizer = BPETokenizer()
        loaded_tokenizer.load(self.save_prefix)

        # Verify loaded tokenizer is identical
        self.assertEqual(tokenizer.vocab, loaded_tokenizer.vocab)
        self.assertEqual(tokenizer.merges, loaded_tokenizer.merges)

        # Test encoding/decoding with loaded tokenizer
        encoded = loaded_tokenizer.encode(text)
        decoded = loaded_tokenizer.decode(encoded)
        self.assertEqual(decoded, text)

    def test_tokenizer_encode_decode_roundtrip(self):
        tokenizer = BPETokenizer()
        text = "Hello, world! This is a test string with some special characters: éàçüö"
        vocab_size = 500 # Ensure enough merges for diverse characters
        tokenizer.train(text, vocab_size)

        encoded = tokenizer.encode(text)
        decoded = tokenizer.decode(encoded)
        self.assertEqual(decoded, text)

    def test_tokenizer_encode_empty_string(self):
        tokenizer = BPETokenizer()
        text = ""
        vocab_size = 256
        tokenizer.train("some text", vocab_size)
        encoded = tokenizer.encode(text)
        self.assertEqual(encoded, [])

    def test_tokenizer_decode_empty_list(self):
        tokenizer = BPETokenizer()
        ids = []
        decoded = tokenizer.decode(ids)
        self.assertEqual(decoded, "")

    def test_tokenizer_decode_invalid_id(self):
        tokenizer = BPETokenizer()
        text = "abc"
        vocab_size = 258
        tokenizer.train(text, vocab_size)
        
        # Pass an ID that is not in the vocabulary
        invalid_ids = [tokenizer.encode("a")[0], 99999, tokenizer.encode("c")[0]]
        decoded = tokenizer.decode(invalid_ids)
        # Expect replacement character for invalid ID
        self.assertIn("?", decoded) 
        self.assertNotEqual(decoded, "ac") # Should not be just 'ac'

if __name__ == '__main__':
    unittest.main()