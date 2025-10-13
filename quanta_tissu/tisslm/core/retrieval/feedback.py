import numpy as np
from ..knowledge_base import KnowledgeBase

class FeedbackBasedRetriever:
    def __init__(self, knowledge_base: KnowledgeBase):
        self.knowledge_base = knowledge_base

    def search(self, query_text: str, **kwargs):
        # Get initial ranking from the knowledge base
        retrieved_docs, initial_scores = self.knowledge_base.retrieve(query_text, **kwargs)

        # Get feedback from the database
        feedback_data = self.knowledge_base.db_client.get_all_documents('feedback')

        # Create a dictionary to store the feedback scores for each document
        feedback_scores = {}
        for feedback in feedback_data:
            if feedback['query'] == query_text:
                for doc in feedback['retrieved_docs']:
                    if doc not in feedback_scores:
                        feedback_scores[doc] = []
                    feedback_scores[doc].append(feedback['score'])

        # Calculate the average feedback score for each document
        avg_feedback_scores = {doc: np.mean(scores) for doc, scores in feedback_scores.items()}

        # Re-rank the retrieved documents based on the feedback score
        # For simplicity, we will just add the feedback score to the similarity score.
        
        reranked_scores = []
        for doc, score in zip(retrieved_docs, initial_scores):
            reranked_scores.append(score + avg_feedback_scores.get(doc, 0))
            
        reranked_indices = np.argsort(reranked_scores)[::-1]
        
        reranked_docs = [retrieved_docs[i] for i in reranked_indices]

        return reranked_docs
