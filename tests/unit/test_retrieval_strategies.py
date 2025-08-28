import unittest
import numpy as np
import os
import sys
from unittest.mock import MagicMock, patch

# Add project root to path for module discovery
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from quanta_tissu.tisslm.core.retrieval.strategy import (
    CosineSimilarityStrategy,
    CNNSimilarityStrategy,
    GeneticSimilarityStrategy,
    BayesianSimilarityStrategy,
    RetrievalStrategy
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