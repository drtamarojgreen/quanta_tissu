import json
import regex as re
from collections import defaultdict

# Regex for splitting text into words and punctuation, similar to GPT-2.
BPE_SPLIT_PATTERN = r"""'(?:[sdmt]|ll|ve|re)| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+"""

def get_pairs(ids):
    """Helper function to find consecutive pairs of IDs in a list."""
    pairs = set()
    for i in range(len(ids) - 1):
        pairs.add((ids[i], ids[i+1]))
    return pairs

class BPETokenizer:
    """
    A from-scratch implementation of the Byte-Pair Encoding (BPE) tokenizer.
    """

    def __init__(self):
        self.merges = {}
        self.vocab = {}
        self.reverse_vocab = {}

    def train(self, text, vocab_size, verbose=False):
        """
        Trains the BPE tokenizer on a given text corpus.

        Args:
            text (str): The training text corpus.
            vocab_size (int): The desired final vocabulary size.
            verbose (bool): Whether to print progress during training.
        """
        if vocab_size < 256:
            raise ValueError("Vocabulary size must be at least 256 to cover all bytes.")

        # 1. Pre-tokenize the text into a list of words using regex
        pre_tokenizer = re.compile(BPE_SPLIT_PATTERN)
        words = pre_tokenizer.findall(text)
        
        # 2. Initialize vocabulary with all individual bytes (0-255)
        initial_vocab_size = 256
        self.vocab = {i: bytes([i]) for i in range(initial_vocab_size)}
        
        # Split words into lists of their UTF-8 byte representations
        word_byte_sequences = [list(word.encode("utf-8")) for word in words]

        # 3. Iteratively merge the most frequent pair of tokens
        num_merges = vocab_size - initial_vocab_size
        for i in range(num_merges):
            # Calculate pair frequencies across all word sequences
            pair_counts = defaultdict(int)
            for sequence in word_byte_sequences:
                for pair in get_pairs(sequence):
                    pair_counts[pair] += 1
            
            if not pair_counts:
                break # No more pairs to merge

            # Find the most frequent pair
            best_pair = max(pair_counts, key=pair_counts.get)
            
            # Create a new token ID for the merged pair
            new_token_id = initial_vocab_size + i
            
            # Update merges and vocabulary
            self.merges[best_pair] = new_token_id
            self.vocab[new_token_id] = self.vocab[best_pair[0]] + self.vocab[best_pair[1]]

            # Merge the best pair in all word sequences for the next iteration
            new_word_byte_sequences = []
            for sequence in word_byte_sequences:
                new_sequence = []
                j = 0
                while j < len(sequence):
                    if j < len(sequence) - 1 and (sequence[j], sequence[j+1]) == best_pair:
                        new_sequence.append(new_token_id)
                        j += 2
                    else:
                        new_sequence.append(sequence[j])
                        j += 1
                new_word_byte_sequences.append(new_sequence)
            word_byte_sequences = new_word_byte_sequences

            if verbose:
                print(f"Merge {i+1}/{num_merges}: {best_pair} -> {new_token_id} ({self.vocab[new_token_id].decode('utf-8', 'replace')})")

        # Create the reverse vocabulary for decoding
        self.reverse_vocab = {v: k for k, v in self.vocab.items()}

    def _encode_chunk(self, text_bytes):
        """Encodes a chunk of text bytes using the learned merges."""
        ids = list(text_bytes)
        while len(ids) >= 2:
            # Find the next best pair to merge based on the learned order
            pairs = get_pairs(ids)
            best_pair = min(pairs, key=lambda p: self.merges.get(p, float('inf')))
            
            if best_pair not in self.merges:
                break # No more merges are possible in this chunk
            
            # Merge the pair
            new_id = self.merges[best_pair]
            new_ids = []
            i = 0
            while i < len(ids):
                if i < len(ids) - 1 and (ids[i], ids[i+1]) == best_pair:
                    new_ids.append(new_id)
                    i += 2
                else:
                    new_ids.append(ids[i])
                    i += 1
            ids = new_ids
        return ids

    def encode(self, text):
        """Encodes a string into a list of token IDs."""
        pre_tokenizer = re.compile(BPE_SPLIT_PATTERN)
        words = pre_tokenizer.findall(text)
        
        all_ids = []
        for word in words:
            text_bytes = word.encode("utf-8")
            all_ids.extend(self._encode_chunk(text_bytes))
        return all_ids

    def decode(self, ids):
        """Decodes a list of token IDs back into a string."""
        tokens = [self.vocab.get(i, b'?') for i in ids]
        text_bytes = b"".join(tokens)
        return text_bytes.decode("utf-8", errors="replace")

    def save(self, prefix):
        """
        Saves the tokenizer's vocabulary and merges to files.

        Args:
            prefix (str): The prefix for the filenames.
        """
        vocab_file = f"{prefix}_vocab.json"
        merges_file = f"{prefix}_merges.txt"

        # Save vocabulary
        # The vocabulary contains bytes, which are not directly JSON serializable.
        # We need to convert them to a serializable format, e.g., a list of integers.
        serializable_vocab = {k: list(v) for k, v in self.vocab.items()}
        with open(vocab_file, "w") as f:
            json.dump(serializable_vocab, f)

        # Save merges
        with open(merges_file, "w") as f:
            for pair, new_id in self.merges.items():
                f.write(f"{pair[0]} {pair[1]} {new_id}\n")

    def load(self, prefix):
        """
        Loads the tokenizer's vocabulary and merges from files.

        Args:
            prefix (str): The prefix for the filenames.
        """
        vocab_file = f"{prefix}_vocab.json"
        merges_file = f"{prefix}_merges.txt"

        # Load vocabulary
        with open(vocab_file, "r") as f:
            serializable_vocab = json.load(f)
            self.vocab = {int(k): bytes(v) for k, v in serializable_vocab.items()}

        # Load merges
        self.merges = {}
        with open(merges_file, "r") as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) == 3:
                    p1, p2, new_id = map(int, parts)
                    self.merges[(p1, p2)] = new_id

        # Rebuild reverse vocabulary
        self.reverse_vocab = {v: k for k, v in self.vocab.items()}