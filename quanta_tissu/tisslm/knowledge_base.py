import numpy as np
import json
from datetime import datetime

from .tissdb_client import TissDBClient

class KnowledgeBase:
    """
    A knowledge storage system that uses TissDB for persistence and retrieval,
    powered by a pure Python TissDB client.
    """
    def __init__(self, model_embeddings, tokenizer, db_host='127.0.0.1', db_port=8080):
        """
        Initializes the KnowledgeBase and connects to TissDB.

        Args:
            model_embeddings: The model's embedding layer.
            tokenizer: The tokenizer for the model.
            db_host (str): The TissDB server host.
            db_port (int): The TissDB server port.
        """
        self.client = TissDBClient(host=db_host, port=db_port)
        self.model_embeddings = model_embeddings
        self.tokenizer = tokenizer
        self._setup_database()

    def _setup_database(self):
        """
        Ensures the 'knowledge' table exists in the database.
        This is an idempotent operation.
        """
        query = """
        CREATE TABLE IF NOT EXISTS knowledge (
            id INTEGER PRIMARY KEY,
            text TEXT,
            embedding TEXT,
            source TEXT,
            timestamp TEXT,
            relevance_score REAL,
            access_count INTEGER
        );
        """
        try:
            with self.client as c:
                c.query(query)
            print("Database schema setup complete.")
        except Exception as e:
            print(f"Database setup failed: {e}")
            # This might be critical, so we could re-raise or handle it
            raise

    def _embed_text(self, text):
        """Generates an embedding for a text by averaging its token embeddings."""
        token_ids = self.tokenizer.tokenize(text)
        if token_ids.size == 0:
            return np.zeros(self.model_embeddings.value.shape[1])
        embeddings = self.model_embeddings.value[token_ids]
        return np.mean(embeddings, axis=0)

    def add_document(self, text, metadata=None):
        """
        Adds a document to the knowledge base in TissDB.
        """
        embedding = self._embed_text(text)
        embedding_json = json.dumps(embedding.tolist())

        doc_metadata = {
            'source': 'user_input',
            'timestamp': datetime.utcnow().isoformat(),
            'relevance_score': 1.0,
            'access_count': 0
        }
        if metadata:
            doc_metadata.update(metadata)

        query = """
        INSERT INTO knowledge (text, embedding, source, timestamp, relevance_score, access_count)
        VALUES ($text, $embedding, $source, $timestamp, $relevance_score, $access_count);
        """
        params = {
            "text": text,
            "embedding": embedding_json,
            "source": doc_metadata['source'],
            "timestamp": doc_metadata['timestamp'],
            "relevance_score": doc_metadata['relevance_score'],
            "access_count": doc_metadata['access_count']
        }
        
        with self.client as c:
            c.query(query, params)
        print(f"Added to KB: '{text}'")

    def retrieve(self, query_text, k=1):
        """
        Retrieves the top k most relevant documents for a given query text from TissDB.
        """
        query_embedding = self._embed_text(query_text)

        with self.client as c:
            select_query = "SELECT id, text, embedding, access_count FROM knowledge;"
            result_str = c.query(select_query)

            if not result_str or result_str == "[]":
                return []

            try:
                all_docs_data = json.loads(result_str)
            except json.JSONDecodeError:
                print(f"Warning: Could not decode JSON from DB: {result_str}")
                return []

            if not all_docs_data:
                return []

            doc_embeddings = [np.array(json.loads(doc['embedding'])) for doc in all_docs_data]
            documents = [doc['text'] for doc in all_docs_data]

            query_norm = np.linalg.norm(query_embedding)
            doc_norms = np.linalg.norm(doc_embeddings, axis=1)

            if query_norm == 0 or np.all(doc_norms == 0):
                return []

            similarities = np.dot(doc_embeddings, query_embedding) / (doc_norms * query_norm)

            if k >= len(similarities):
                top_k_indices = np.argsort(similarities)[::-1]
            else:
                top_k_indices = np.argpartition(similarities, -k)[-k:]
                top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]
            
            # Update access counts for the retrieved documents
            for i in top_k_indices:
                doc_id = all_docs_data[i]['id']
                new_access_count = all_docs_data[i].get('access_count', 0) + 1
                update_query = "UPDATE knowledge SET access_count = $count WHERE id = $id;"
                c.query(update_query, {"count": new_access_count, "id": doc_id})

            return [documents[i] for i in top_k_indices]

    def self_update_from_interaction(self, query, generated_response, user_correction=None):
        """
        Self-updating mechanism that learns from model interactions.
        """
        if user_correction:
            corrected_doc = f"Query: {query} Correct Answer: {user_correction}"
            self.add_document(corrected_doc, metadata={'source': 'self_correction'})
            print(f"Self-updated KB with correction: '{user_correction}'")
        else:
            knowledge_doc = f"Query: {query} Response: {generated_response}"
            self.add_document(knowledge_doc, metadata={'source': 'self_generated'})

    def get_knowledge_stats(self):
        """
        Return statistics about the knowledge base from TissDB.
        """
        with self.client as c:
            count_result = c.query("SELECT COUNT(*) FROM knowledge;")
            total_docs = json.loads(count_result)[0]['COUNT(*)']
            
            if total_docs == 0:
                return {"total_docs": 0}

            avg_rel_result = c.query("SELECT AVG(relevance_score) FROM knowledge;")
            avg_relevance_score = json.loads(avg_rel_result)[0]['AVG(relevance_score)']
            
            total_access_result = c.query("SELECT SUM(access_count) FROM knowledge;")
            total_accesses = json.loads(total_access_result)[0]['SUM(access_count)']

            stats = {
                "total_docs": total_docs,
                "avg_relevance_score": avg_relevance_score,
                "total_accesses": total_accesses
            }
            return stats

    # The add_feedback method is complex to implement without proper transaction support
    # in the client, as it involves a read-then-write. For this refactoring,
    # we will simplify it or assume the logic is sufficient for a demo.
    def add_feedback(self, query, retrieved_docs, feedback_score):
        """
        Updates relevance scores based on feedback.
        NOTE: This is a simplified, non-transactional implementation.
        """
        with self.client as c:
            for doc_text in retrieved_docs:
                # This is not robust, a proper implementation would use IDs.
                update_query = "UPDATE knowledge SET relevance_score = relevance_score * 0.9 + $new_score * 0.1 WHERE text = $text;"
                c.query(update_query, {"new_score": feedback_score / 5.0, "text": doc_text})
