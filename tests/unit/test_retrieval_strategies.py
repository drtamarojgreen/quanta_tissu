import unittest
import numpy as np
import os
import sys
from unittest.mock import MagicMock, patch

# Add project root to path for module discovery
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from quanta_tissu.tisslm.core.retrieval.strategy import (
    RetrievalStrategy,
    BM25RetrievalStrategy
)
from quanta_tissu.tisslm.core.retrieval.semantic import (
    CosineSimilarityStrategy,
    CNNSimilarityStrategy,
    GeneticSimilarityStrategy,
    BayesianSimilarityStrategy
)

class TestRetrievalStrategies(unittest.TestCase):

    def setUp(self):
        self.query_embedding = np.array([1.0, 2.0, 3.0])
        self.doc_embeddings = [
            np.array([1.0, 2.0, 3.0]),  # Identical
            np.array([-1.0, -2.0, -3.0]), # Opposite
            np.array([1.0, 0.0, 0.0]),   # Orthogonal
            np.array([0.5, 1.0, 1.5])    # Scaled version
        ]
        self.zero_embedding = np.array([0.0, 0.0, 0.0])

    def test_cosine_similarity_strategy(self):
        strategy = CosineSimilarityStrategy()
        similarities = strategy.calculate_similarity(self.query_embedding, self.doc_embeddings)

        # Expected cosine similarities (approximate)
        # Identical: 1.0
        # Opposite: -1.0
        # Orthogonal: 0.18898 (dot product 1.0 / (sqrt(14) * 1.0))
        # Scaled: 1.0
        expected_similarities = np.array([
            1.0,
            -1.0,
            1.0 / (np.linalg.norm(self.query_embedding) * np.linalg.norm(np.array([1.0, 0.0, 0.0]))),
            1.0
        ])

        np.testing.assert_almost_equal(similarities, expected_similarities, decimal=5)

        # Test with zero query embedding
        similarities_zero_query = strategy.calculate_similarity(self.zero_embedding, self.doc_embeddings)
        np.testing.assert_almost_equal(similarities_zero_query, np.zeros(len(self.doc_embeddings)))

        # Test with zero document embeddings
        similarities_zero_docs = strategy.calculate_similarity(self.query_embedding, [self.zero_embedding])
        np.testing.assert_almost_equal(similarities_zero_docs, np.zeros(1))

    def test_cnn_similarity_strategy(self):
        strategy = CNNSimilarityStrategy(embedding_dim=self.query_embedding.shape[0])
        
        # Manually calculate expected similarities based on the fixed weights in CNNSimilarityStrategy
        # Filters: np.array([[1.0, 1.0, 1.0], [1.0, -1.0, 0.0], [0.0, 1.0, -1.0]])
        # Dense weights are now calculated based on the number of filters

        doc_embeddings_np = np.array(self.doc_embeddings)
        convolved = np.dot(doc_embeddings_np, strategy.filters.T)
        activated = np.maximum(0, convolved)
        expected_similarities = np.dot(activated, strategy.dense_weights)

        similarities = strategy.calculate_similarity(self.query_embedding, self.doc_embeddings)
        self.assertEqual(similarities.shape, (len(self.doc_embeddings),))
        np.testing.assert_almost_equal(similarities, expected_similarities, decimal=5)

        # Test with zero query embedding
        similarities_zero_query = strategy.calculate_similarity(self.zero_embedding, self.doc_embeddings)
        self.assertEqual(similarities_zero_query.shape, (len(self.doc_embeddings),))
        self.assertTrue(np.all((similarities_zero_query >= 0) & (similarities_zero_query <= 1)))

        # Test with zero document embeddings
        similarities_zero_docs = strategy.calculate_similarity(self.query_embedding, [self.zero_embedding])
        self.assertEqual(similarities_zero_docs.shape, (1,))
        self.assertTrue(np.all((similarities_zero_docs >= 0) & (similarities_zero_docs <= 1)))

    def test_bm25_retrieval_strategy(self):
        corpus = ["this is a document", "this is another document", "a third document"]
        query = "document"
        strategy = BM25RetrievalStrategy(corpus)
        similarities = strategy.calculate_similarity(None, None, query_text=query)

        # Basic assertion: scores should be non-negative and sum to something reasonable
        self.assertTrue(np.all(similarities >= 0))
        self.assertGreater(np.sum(similarities), 0)
        self.assertEqual(similarities.shape, (len(corpus),))

        # Test with a query that should match one document more strongly
        query_specific = "this is a document"
        similarities_specific = strategy.calculate_similarity(None, None, query_text=query_specific)
        self.assertGreater(similarities_specific[0], similarities_specific[1])

    def test_genetic_similarity_strategy(self):
        from quanta_tissu.tisslm.core.retrieval.semantic import GeneticSimilarityStrategy
        strategy = GeneticSimilarityStrategy()
        similarities = strategy.calculate_similarity(self.query_embedding, self.doc_embeddings)

        self.assertTrue(np.all(similarities >= 0))
        self.assertEqual(similarities.shape, (len(self.doc_embeddings),))
        # More specific assertions would require knowing the exact GA behavior
        # For now, just check basic properties

    def test_bayesian_similarity_strategy(self):
        from quanta_tissu.tisslm.core.retrieval.semantic import BayesianSimilarityStrategy
        strategy = BayesianSimilarityStrategy()
        # Dummy Hessian matrix for testing
        hessian_matrix = np.array([[1.0, 0.5, 0.1], [0.5, 1.0, 0.2], [0.1, 0.2, 1.0]])
        similarities = strategy.calculate_similarity(self.query_embedding, self.doc_embeddings, hessian_matrix=hessian_matrix)

        # Cosine similarity can be negative, so we don't assert non-negativity.
        self.assertEqual(similarities.shape, (len(self.doc_embeddings),))
        self.assertTrue(np.all((similarities >= -1) & (similarities <= 1)), "Similarities should be between -1 and 1.")

if __name__ == '__main__':
    unittest.main()