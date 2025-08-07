import numpy as np

class KnowledgeBase:
    def __init__(self, model_embeddings, tokenizer):
        self.model_embeddings = model_embeddings
        self.tokenizer = tokenizer
        self.documents = []
        self.doc_embeddings = []

    def _embed_text(self, text):
        """Generates an embedding for a text by averaging its token embeddings."""
        token_ids = self.tokenizer(text)
        if not token_ids:
            # Return a zero vector if the text is empty or has no known tokens
            return np.zeros(self.model_embeddings.shape[1])

        # Get embeddings for each token and average them
        embeddings = self.model_embeddings[token_ids]
        return np.mean(embeddings, axis=0)

    def add_document(self, text):
        """Adds a document to the knowledge base."""
        embedding = self._embed_text(text)
        self.documents.append(text)
        self.doc_embeddings.append(embedding)
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

        return [self.documents[i] for i in top_k_indices]
