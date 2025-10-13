from abc import ABC, abstractmethod
from typing import List

class ChunkingStrategy(ABC):
    """Abstract base class for document chunking strategies."""
    @abstractmethod
    def chunk(self, text: str) -> List[str]:
        """
        Splits a text into a list of smaller chunks.

        Args:
            text (str): The input text to be chunked.

        Returns:
            List[str]: A list of text chunks.
        """
        pass

class FixedSizeChunking(ChunkingStrategy):
    """
    Chunks text into fixed-size segments with optional overlap.
    """
    def __init__(self, chunk_size: int, chunk_overlap: int):
        if chunk_overlap >= chunk_size:
            raise ValueError("chunk_overlap must be smaller than chunk_size.")
        self.chunk_size = chunk_size
        self.chunk_overlap = chunk_overlap

    def chunk(self, text: str) -> List[str]:
        """
        Splits the text into chunks of `chunk_size` with `chunk_overlap`.
        """
        if not text:
            return []
        
        words = text.split()
        chunks = []
        start = 0
        while start < len(words):
            end = start + self.chunk_size
            chunk_words = words[start:end]
            chunks.append(" ".join(chunk_words))
            if end >= len(words):
                break
            start += self.chunk_size - self.chunk_overlap
        return chunks

class SentenceChunking(ChunkingStrategy):
    """
    Chunks text based on sentence boundaries.
    (Note: This is a simple implementation and may not handle all edge cases.)
    """
    def chunk(self, text: str) -> List[str]:
        """
        Splits the text into sentences.
        """
        if not text:
            return []
        
        # A simple regex could be used here, but for simplicity without adding 're',
        # we'll split on common sentence-ending punctuation.
        text = text.replace('. ', '.\n').replace('? ', '?\n').replace('! ', '!\n')
        return [line.strip() for line in text.splitlines() if line.strip()]
