from abc import ABC, abstractmethod
from typing import List, Dict
import numpy as np
from collections import defaultdict

class KeywordExtractor(ABC):
    """Abstract base class for keyword extraction strategies."""
    @abstractmethod
    def extract_keywords(self, text: str, top_n: int = 5) -> List[str]:
        """
        Extracts top_n keywords from a given text.

        Args:
            text (str): The input text.
            top_n (int): The number of top keywords to extract.

        Returns:
            List[str]: A list of extracted keywords.
        """
        pass

class TFIDFKeywordExtractor(KeywordExtractor):
    """
    Extracts keywords using a simple TF-IDF approach.
    Requires a corpus to build IDF values.
    """
    def __init__(self, corpus: List[str] = None, stop_words: List[str] = None):
        self.corpus = corpus if corpus is not None else []
        self.stop_words = set(stop_words) if stop_words is not None else set()
        self.idf = {}
        self._build_idf()

    def _tokenize(self, text: str) -> List[str]:
        """Simple tokenizer: lowercase, split by non-alphanumeric."""
        return [word for word in text.lower().split() if word.isalpha() and word not in self.stop_words]

    def _build_idf(self):
        """Builds Inverse Document Frequency (IDF) from the corpus."""
        doc_count = len(self.corpus)
        if doc_count == 0:
            return

        df = defaultdict(int) # Document frequency
        for doc in self.corpus:
            for word in set(self._tokenize(doc)):
                df[word] += 1
        
        for word, count in df.items():
            self.idf[word] = np.log(doc_count / (count + 1)) # Add 1 to avoid division by zero

    def extract_keywords(self, text: str, top_n: int = 5) -> List[str]:
        """
        Extracts keywords from text using TF-IDF scores.
        """
        if not text:
            return []

        tokens = self._tokenize(text)
        if not tokens:
            return []

        # Calculate Term Frequency (TF) for the current document
        tf = defaultdict(int)
        for token in tokens:
            tf[token] += 1
        
        # Calculate TF-IDF score for each token
        tfidf_scores = {}
        for token in set(tokens):
            tfidf_scores[token] = tf[token] * self.idf.get(token, 0) # Use 0 if word not in corpus

        # Sort by TF-IDF score and return top_n keywords
        sorted_keywords = sorted(tfidf_scores.items(), key=lambda item: item[1], reverse=True)
        return [word for word, score in sorted_keywords[:top_n]]
