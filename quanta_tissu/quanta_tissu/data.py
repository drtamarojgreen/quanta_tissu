import numpy as np
import os
from .tokenizer import Tokenizer

def load_corpus(corpus_path: str):
    """
    Loads text from all .txt files in the specified corpus path and tokenizes it.
    """
    full_text = ""
    for filename in os.listdir(corpus_path):
        if filename.endswith(".txt"):
            file_path = os.path.join(corpus_path, filename)
            with open(file_path, "r", encoding="utf-8", errors="replace") as f:
                full_text += f.read() + "\n" # Add newline to separate content from different files

    tokenizer = Tokenizer()
    token_ids = tokenizer.tokenize(full_text)
    return token_ids

class Dataset:
    def __init__(self, token_ids, batch_size, seq_len):
        self.token_ids = token_ids
        self.batch_size = batch_size
        self.seq_len = seq_len

        # Calculate how many full batches we can make
        self.num_tokens = len(self.token_ids)
        self.num_batches = (self.num_tokens - 1) // (self.batch_size * self.seq_len)

        if self.num_batches == 0:
            raise ValueError("Not enough text to create a single batch. Try a smaller batch_size or seq_len.")

        # Trim the data to fit full batches
        end_idx = self.num_batches * self.batch_size * self.seq_len
        self.data = self.token_ids[:end_idx + 1]

    def __len__(self):
        return self.num_batches

    def __iter__(self):
        self.current_batch = 0
        return self

    def __next__(self):
        if self.current_batch >= self.num_batches:
            raise StopIteration

        # Calculate start and end indices for the chunk of data for this batch
        start_idx = self.current_batch * self.batch_size * self.seq_len
        end_idx = start_idx + self.batch_size * self.seq_len

        # Create x and y
        # x is the input sequence chunk
        x_chunk = self.data[start_idx : end_idx]
        # y is the target sequence chunk, shifted by one
        y_chunk = self.data[start_idx + 1 : end_idx + 1]

        # Reshape into batches
        x = np.array(x_chunk).reshape((self.batch_size, self.seq_len))
        y = np.array(y_chunk).reshape((self.batch_size, self.seq_len))

        self.current_batch += 1
        return x, y
