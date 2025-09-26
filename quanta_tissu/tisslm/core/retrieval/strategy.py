import numpy as np
from abc import ABC, abstractmethod

class RetrievalStrategy(ABC):
    """
    Abstract base class for all retrieval similarity strategies.
    """
    @abstractmethod
    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        """
        Calculates the similarity between a query embedding and a set of document embeddings.

        Args:
            query_embedding (np.ndarray): The embedding of the query.
            doc_embeddings (list[np.ndarray]): A list of embeddings for the documents.
            **kwargs: Additional parameters that might be needed by specific strategies.

        Returns:
            np.ndarray: An array of similarity scores.
        """
        pass

class CosineSimilarityStrategy(RetrievalStrategy):
    """
    Calculates similarity using cosine similarity.
    """
    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        query_norm = np.linalg.norm(query_embedding)
        doc_norms = np.linalg.norm(doc_embeddings, axis=1)
        return np.dot(doc_embeddings, query_embedding) / (doc_norms * query_norm)

class BM25RetrievalStrategy(RetrievalStrategy):
    """
    Implements the BM25 retrieval strategy.
    This is a keyword-based search and does not use embeddings.
    """
    def __init__(self, corpus: list[str], k1=1.5, b=0.75):
        self.k1 = k1
        self.b = b
        self.corpus = corpus
        self.doc_len = [len(doc.split()) for doc in corpus]
        self.avgdl = sum(self.doc_len) / len(self.corpus)
        self.doc_freqs = []
        self.idf = {}
        self._initialize()

    def _initialize(self):
        """
        Initializes the BM25 index.
        """
        nd = {}  # document frequencies
        for doc in self.corpus:
            for word in set(doc.split()):
                nd[word] = nd.get(word, 0) + 1

        for word, freq in nd.items():
            self.idf[word] = np.log((len(self.corpus) - freq + 0.5) / (freq + 0.5) + 1)

        for doc in self.corpus:
            freqs = {}
            for word in doc.split():
                freqs[word] = freqs.get(word, 0) + 1
            self.doc_freqs.append(freqs)

    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        """
        Calculates BM25 scores. The query_embedding is expected to be a string for BM25.
        """
        query = kwargs.get('query_text')
        if not query:
            raise ValueError("BM25RetrievalStrategy requires 'query_text' in kwargs")

        query_terms = query.split()
        scores = np.zeros(len(self.corpus))

        for i in range(len(self.corpus)):
            score = 0
            for term in query_terms:
                if term in self.doc_freqs[i]:
                    tf = self.doc_freqs[i][term]
                    score += (self.idf.get(term, 0) * (tf * (self.k1 + 1)) /
                              (tf + self.k1 * (1 - self.b + self.b * self.doc_len[i] / self.avgdl)))
            scores[i] = score

        return scores
