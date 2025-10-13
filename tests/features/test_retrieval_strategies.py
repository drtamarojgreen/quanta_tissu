import unittest
import numpy as np
from unittest.mock import MagicMock, patch

# Adjust path to import modules from the project root
import os
import sys
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.retrieval.strategy import CosineSimilarityStrategy, CNNSimilarityStrategy, GeneticSimilarityStrategy, BayesianSimilarityStrategy

class TestRetrievalStrategies(unittest.TestCase):

    def test_cosine_similarity_strategy(self):
        strategy = CosineSimilarityStrategy()
        query_embedding = np.array([1.0, 1.0])
        doc_embeddings = [
            np.array([1.0, 1.0]), # Identical
            np.array([1.0, 0.0]), # Orthogonal
            np.array([-1.0, -1.0]), # Opposite
            np.array([0.0, 0.0]) # Zero vector
        ]

        similarities = strategy.calculate_similarity(query_embedding, doc_embeddings)
        expected_similarities = np.array([1.0, 0.70710678, -1.0, 0.0])
        np.testing.assert_allclose(similarities, expected_similarities, atol=1e-7)

        # Test with zero query embedding
        query_embedding_zero = np.array([0.0, 0.0])
        similarities_zero_query = strategy.calculate_similarity(query_embedding_zero, doc_embeddings)
        np.testing.assert_allclose(similarities_zero_query, np.zeros(len(doc_embeddings)))

    def test_cnn_similarity_strategy(self):
        embedding_dim = 10
        strategy = CNNSimilarityStrategy(embedding_dim)
        query_embedding = np.random.rand(embedding_dim)
        doc_embeddings = [np.random.rand(embedding_dim) for _ in range(5)]

        similarities = strategy.calculate_similarity(query_embedding, doc_embeddings)
        self.assertEqual(similarities.shape, (len(doc_embeddings),))
        self.assertTrue(np.all(similarities >= 0)) # Due to ReLU

    def test_genetic_similarity_strategy(self):
        embedding_dim = 5
        strategy = GeneticSimilarityStrategy()
        query_embedding = np.random.rand(embedding_dim)
        doc_embeddings = [np.random.rand(embedding_dim) for _ in range(10)]

        # Test with default GA params
        similarities = strategy.calculate_similarity(query_embedding, doc_embeddings)
        self.assertEqual(similarities.shape, (len(doc_embeddings),))
        self.assertTrue(np.all(similarities >= -1) and np.all(similarities <= 1)) # Cosine similarity range

        # Test with custom GA params
        custom_ga_params = {"population_size": 50, "generations": 10, "mutation_rate": 0.05, "crossover_rate": 0.8}
        similarities_custom = strategy.calculate_similarity(query_embedding, doc_embeddings, ga_params=custom_ga_params)
        self.assertEqual(similarities_custom.shape, (len(doc_embeddings),))

    def test_bayesian_similarity_strategy(self):
        embedding_dim = 5
        strategy = BayesianSimilarityStrategy()
        query_embedding = np.random.rand(embedding_dim)
        doc_embeddings = [np.random.rand(embedding_dim) for _ in range(10)]

        # Test with numpy array for hessian_matrix
        hessian_matrix_np = np.random.rand(embedding_dim, embedding_dim)
        hessian_matrix_np = hessian_matrix_np + hessian_matrix_np.T # Make symmetric
        similarities_np = strategy.calculate_similarity(query_embedding, doc_embeddings, hessian_matrix=hessian_matrix_np)
        self.assertEqual(similarities_np.shape, (len(doc_embeddings),))

        # Test with dictionary for hessian_matrix (providing eigenvalues directly)
        eigenvalues = np.random.rand(embedding_dim)
        hessian_matrix_dict = {'eigenvalues': eigenvalues}
        similarities_dict = strategy.calculate_similarity(query_embedding, doc_embeddings, hessian_matrix=hessian_matrix_dict)
        self.assertEqual(similarities_dict.shape, (len(doc_embeddings),))

        # Test error when hessian_matrix is missing
        with self.assertRaises(ValueError):
            strategy.calculate_similarity(query_embedding, doc_embeddings)

        # Test error when hessian_matrix dict is missing eigenvalues
        with self.assertRaises(ValueError):
            strategy.calculate_similarity(query_embedding, doc_embeddings, hessian_matrix={})

if __name__ == '__main__':
    unittest.main()
