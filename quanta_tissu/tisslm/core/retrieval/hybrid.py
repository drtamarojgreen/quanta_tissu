import numpy as np
from .strategy import RetrievalStrategy
from .semantic import BM25, CosineSimilarityStrategy

class HybridRetrievalStrategy(RetrievalStrategy):
    """
    Combines keyword-based and semantic search results.
    """
    def __init__(self, keyword_strategy, semantic_strategy, keyword_weight=0.5, semantic_weight=0.5):
        self.keyword_strategy = keyword_strategy
        self.semantic_strategy = semantic_strategy
        self.keyword_weight = keyword_weight
        self.semantic_weight = semantic_weight

    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        # Get scores from semantic strategy
        semantic_scores = self.semantic_strategy.calculate_similarity(query_embedding, doc_embeddings, **kwargs)

        # Get scores from keyword strategy
        query_text = kwargs.get('query_text')
        if not query_text:
            raise ValueError("HybridRetrievalStrategy requires 'query_text' in kwargs.")
        
        # We need to have the corpus for the keyword search.
        # We can get it from the keyword_strategy object.
        keyword_scores = self.keyword_strategy.search(query_text)

        # Normalize scores
        semantic_scores_normalized = (semantic_scores - np.min(semantic_scores)) / (np.max(semantic_scores) - np.min(semantic_scores))
        
        keyword_scores_dict = dict(keyword_scores)
        keyword_scores_array = np.array([keyword_scores_dict.get(i, 0) for i in range(len(doc_embeddings))])
        keyword_scores_normalized = (keyword_scores_array - np.min(keyword_scores_array)) / (np.max(keyword_scores_array) - np.min(keyword_scores_array))

        # Combine scores
        combined_scores = (self.keyword_weight * keyword_scores_normalized) + (self.semantic_weight * semantic_scores_normalized)

        return combined_scores
