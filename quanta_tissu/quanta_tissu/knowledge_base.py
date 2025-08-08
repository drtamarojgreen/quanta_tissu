import numpy as np

class KnowledgeBase:
    """
    A vector-based knowledge storage system for efficient similarity search and retrieval.
    Supports self-updating mechanisms and feedback-based learning.
    """
    
    def __init__(self, model_embeddings, tokenizer):
        self.model_embeddings = model_embeddings
        self.tokenizer = tokenizer
        self.documents = []
        self.doc_embeddings = []
        self.doc_metadata = []  # Store metadata like timestamps, relevance scores, etc.
        self.feedback_history = []  # Store user feedback for self-improvement

    def _embed_text(self, text):
        """Generates an embedding for a text by averaging its token embeddings."""
        token_ids = self.tokenizer(text)
        if token_ids.size == 0:
            # Return a zero vector if the text is empty or has no known tokens
            return np.zeros(self.model_embeddings.shape[1])

        # Get embeddings for each token and average them
        embeddings = self.model_embeddings[token_ids]
        return np.mean(embeddings, axis=0)

    def add_document(self, text, metadata=None):
        """
        Adds a document to the knowledge base with optional metadata.
        
        Args:
            text: The document text to add
            metadata: Optional dictionary containing metadata (timestamp, source, etc.)
        """
        embedding = self._embed_text(text)
        self.documents.append(text)
        self.doc_embeddings.append(embedding)
        
        # Add metadata with default values
        doc_metadata = {
            'timestamp': np.datetime64('now'),
            'relevance_score': 1.0,
            'access_count': 0,
            'source': 'user_input'
        }
        if metadata:
            doc_metadata.update(metadata)
        self.doc_metadata.append(doc_metadata)
        
        print(f"Added to KB: '{text}'")

    def retrieve(self, query_text, k=1):
        """
        Retrieves the top k most relevant documents for a given query text.
        Relevance is determined by cosine similarity.
        """
        if not self.documents:
            return []

        query_embedding = self._embed_text(query_text)

        # Calculate cosine similarity between the query and all document embeddings
        query_norm = np.linalg.norm(query_embedding)
        doc_norms = np.linalg.norm(self.doc_embeddings, axis=1)

        # Avoid division by zero
        if query_norm == 0 or np.all(doc_norms == 0):
            return []

        similarities = np.dot(self.doc_embeddings, query_embedding) / (doc_norms * query_norm)

        # Get the indices of the top k documents
        # Using argpartition for efficiency if k is small compared to the number of docs
        if k >= len(similarities):
            top_k_indices = np.argsort(similarities)[::-1]
        else:
            top_k_indices = np.argpartition(similarities, -k)[-k:]
            top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]

        # Update access counts for retrieved documents
        for i in top_k_indices:
            self.doc_metadata[i]['access_count'] += 1
            
        return [self.documents[i] for i in top_k_indices]
    
    def add_feedback(self, query, retrieved_docs, feedback_score, feedback_text=None):
        """
        Add user feedback to improve future retrievals.
        
        Args:
            query: The original query
            retrieved_docs: List of documents that were retrieved
            feedback_score: Numeric score (e.g., 1-5) indicating relevance
            feedback_text: Optional text feedback
        """
        feedback_entry = {
            'timestamp': np.datetime64('now'),
            'query': query,
            'retrieved_docs': retrieved_docs,
            'score': feedback_score,
            'text': feedback_text
        }
        self.feedback_history.append(feedback_entry)
        
        # Update relevance scores based on feedback
        self._update_relevance_scores(query, retrieved_docs, feedback_score)
    
    def _update_relevance_scores(self, query, retrieved_docs, feedback_score):
        """
        Update document relevance scores based on user feedback.
        This implements a simple self-updating mechanism.
        """
        # Find indices of retrieved documents
        for doc_text in retrieved_docs:
            try:
                doc_index = self.documents.index(doc_text)
                # Adjust relevance score based on feedback
                current_score = self.doc_metadata[doc_index]['relevance_score']
                # Simple exponential moving average update
                alpha = 0.1  # Learning rate
                new_score = (1 - alpha) * current_score + alpha * (feedback_score / 5.0)
                self.doc_metadata[doc_index]['relevance_score'] = new_score
            except ValueError:
                # Document not found in knowledge base
                continue
    
    def self_update_from_interaction(self, query, generated_response, user_correction=None):
        """
        Self-updating mechanism that learns from model interactions.
        
        Args:
            query: The user's query
            generated_response: The model's response
            user_correction: Optional correction provided by user
        """
        if user_correction:
            # Add the corrected information as a new document
            corrected_doc = f"Query: {query} Correct Answer: {user_correction}"
            self.add_document(corrected_doc, metadata={
                'source': 'self_correction',
                'original_response': generated_response
            })
            print(f"Self-updated KB with correction: '{user_correction}'")
        else:
            # If no correction, assume the response was acceptable
            # and potentially add it as knowledge if it's informative
            if len(generated_response.split()) > 2:  # Simple heuristic
                knowledge_doc = f"Query: {query} Response: {generated_response}"
                self.add_document(knowledge_doc, metadata={
                    'source': 'self_generated',
                    'confidence': 0.7  # Lower confidence for self-generated content
                })
    
    def get_knowledge_stats(self):
        """
        Return statistics about the knowledge base for monitoring and debugging.
        """
        if not self.documents:
            return {"total_docs": 0}
            
        stats = {
            "total_docs": len(self.documents),
            "avg_relevance_score": np.mean([meta['relevance_score'] for meta in self.doc_metadata]),
            "total_accesses": sum(meta['access_count'] for meta in self.doc_metadata),
            "feedback_entries": len(self.feedback_history),
            "sources": {}
        }
        
        # Count documents by source
        for meta in self.doc_metadata:
            source = meta.get('source', 'unknown')
            stats['sources'][source] = stats['sources'].get(source, 0) + 1
            
        return stats
