import numpy as np
import json
import requests
from datetime import datetime

class KnowledgeBase:
    """
    A knowledge storage and retrieval system that uses the TissDB HTTP API.
    """
    def __init__(self, model_embeddings, tokenizer, db_host='127.0.0.1', db_port=8080, db_name='testdb'):
        """
        Initializes the KnowledgeBase.

        Args:
            model_embeddings: The model's embedding layer.
            tokenizer: The tokenizer for the model.
            db_host (str): The TissDB server host.
            db_port (int): The TissDB server port.
            db_name (str): The name of the database to use.
        """
        self.base_url = f"http://{db_host}:{db_port}"
        self.db_name = db_name
        self.model_embeddings = model_embeddings
        self.tokenizer = tokenizer
        self._setup_database()

    def _setup_database(self):
        """
        Ensures the database and the 'knowledge' collection exist.
        This is an idempotent operation.
        """
        try:
            # Create the database
            response = requests.put(f"{self.base_url}/{self.db_name}")
            # The server may incorrectly return 500 when the DB already exists.
            if response.status_code not in [201, 200, 409] and "already exists" not in response.text:
                 response.raise_for_status()

            # Create the collections
            for collection_name in ["knowledge", "knowledge_feedback"]:
                response = requests.put(f"{self.base_url}/{self.db_name}/{collection_name}")
                if response.status_code not in [201, 200, 409] and "already exists" not in response.text:
                    response.raise_for_status()

            print("Database and collection setup complete.")
        except requests.exceptions.RequestException as e:
            print(f"Database setup failed: {e}")
            raise

    def _embed_text(self, text):
        """Generates an embedding for a text by averaging its token embeddings."""
        token_ids = self.tokenizer.tokenize(text)
        if token_ids.size == 0:
            return np.zeros(self.model_embeddings.shape[1])
        embeddings = self.model_embeddings.value[token_ids]
        return np.mean(embeddings, axis=0)

    def add_document(self, text, metadata=None):
        """
        Adds a document to the knowledge base in TissDB.
        """
        embedding = self._embed_text(text)

        doc = {
            'text': text,
            'embedding': json.dumps(embedding.tolist()),
            'source': 'user_input',
            'timestamp': datetime.utcnow().isoformat(),
            'relevance_score': 1.0,
            'access_count': 0
        }
        if metadata:
            doc.update(metadata)

        try:
            # The API generates the ID, so we use POST
            response = requests.post(f"{self.base_url}/{self.db_name}/knowledge", json=doc)
            response.raise_for_status()
            print(f"Added to KB: '{text}'")
        except requests.exceptions.RequestException as e:
            print(f"Failed to add document to KB: {e}")


    def retrieve(self, query_text, k=1):
        """
        Retrieves the top k most relevant documents for a given query text from TissDB.
        """
        query_embedding = self._embed_text(query_text)

        try:
            # We have to retrieve all documents and do the similarity search client-side
            # as the DB doesn't support vector search yet.
            query = {"query": "SELECT id, text, embedding FROM knowledge"}
            response = requests.post(f"{self.base_url}/{self.db_name}/knowledge/_query", json=query)
            response.raise_for_status()
            all_docs_data = response.json()

            if not all_docs_data:
                return []

            doc_embeddings = [np.array(json.loads(doc['embedding'])) for doc in all_docs_data]
            documents = [doc['text'] for doc in all_docs_data]

            query_norm = np.linalg.norm(query_embedding)
            doc_norms = np.linalg.norm(doc_embeddings, axis=1)

            # Avoid division by zero
            if query_norm == 0 or np.all(doc_norms == 0):
                return []

            similarities = np.dot(doc_embeddings, query_embedding) / (doc_norms * query_norm)

            if k >= len(similarities):
                top_k_indices = np.argsort(similarities)[::-1]
            else:
                top_k_indices = np.argpartition(similarities, -k)[-k:]
                top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]
            
            return [documents[i] for i in top_k_indices]

        except requests.exceptions.RequestException as e:
            print(f"Failed to retrieve from KB: {e}")
            return []
        except (json.JSONDecodeError, KeyError) as e:
            print(f"Failed to parse response from KB: {e}")
            return []

    # Other methods like self_update, get_knowledge_stats, add_feedback would
    # also need to be refactored to use the HTTP API. For now, we focus on
    # the core functionality needed for the BDD tests.
    def self_update_from_interaction(self, query, generated_response, user_correction=None):
        """
        Updates the knowledge base from a user interaction.
        If a user correction is provided, it's treated as the ground truth.
        """
        if user_correction:
            document_text = f"Query: {query} Correct Answer: {user_correction}"
            metadata = {'source': 'user_correction'}
        else:
            document_text = f"Query: {query} Response: {generated_response}"
            metadata = {'source': 'generated_response'}

        self.add_document(document_text, metadata=metadata)

    def get_knowledge_stats(self):
        """
        Retrieves statistics about the knowledge base from TissDB.
        """
        try:
            response = requests.get(f"{self.base_url}/{self.db_name}/_stats")
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Failed to get KB stats: {e}")
            return {}

    def add_feedback(self, query, retrieved_docs, feedback_score, feedback_text):
        """
        Adds feedback for a set of retrieved documents to TissDB.
        """
        feedback_data = {
            'query': query,
            'retrieved_docs': retrieved_docs,
            'score': feedback_score,
            'text': feedback_text,
            'timestamp': datetime.utcnow().isoformat()
        }
        try:
            response = requests.post(f"{self.base_url}/{self.db_name}/_feedback", json=feedback_data)
            response.raise_for_status()
        except requests.exceptions.RequestException as e:
            print(f"Failed to add feedback to KB: {e}")
