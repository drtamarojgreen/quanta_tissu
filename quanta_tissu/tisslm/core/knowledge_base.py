import numpy as np
import json
import logging
from datetime import datetime

from .db.client import TissDBClient
from .embedding.embedder import Embedder
from .retrieval.strategy import RetrievalStrategy
from .model_error_handler import ModelProcessingError, InputValidationError
from .system_error_handler import DatabaseConnectionError # Import DatabaseConnectionError

logger = logging.getLogger(__name__)

class KnowledgeBase:
    """
    A facade for the knowledge storage and retrieval system.
    It coordinates an embedder, a database client, and retrieval strategies.
    """
    def __init__(self, embedder: Embedder, db_client: TissDBClient):
        """
        Initializes the KnowledgeBase.

        Args:
            embedder: An instance of the Embedder class.
            db_client: An instance of the TissDBClient class.
        """
        self.embedder = embedder
        self.db_client = db_client
        self.connected = False
        try:
            # The collections the KB requires
            collections = ["knowledge", "knowledge_feedback", "feedback"]
            self.connected = self.db_client.ensure_db_setup(collections)
        except Exception as e:
            logger.warning(f"KnowledgeBase could not connect to DB. Operating in disconnected mode. Error: {e}")

    def add_document(self, text: str, metadata: dict = None):
        """
        Embeds a document and adds it to the knowledge base.
        """
        if not self.connected:
            logger.warning("Cannot add document: KnowledgeBase is not connected.")
            return

        embedding = self.embedder.embed(text)
        doc = {
            'text': text,
            'embedding': json.dumps(embedding.tolist()),
            'source': 'user_input',
            'timestamp': datetime.utcnow().isoformat()
        }
        if metadata:
            doc.update(metadata)

        self.db_client.add_document('knowledge', doc)
        logger.info(f"Added document to knowledge base.")

    def retrieve(self, query_text: str, strategy: RetrievalStrategy, k: int = 1, **kwargs) -> list:
        """
        Retrieves the top-k most relevant documents for a query using a given strategy.
        """
        documents = []
        doc_embeddings = []

        # Check if use_db is explicitly set to True in kwargs and if connected
        use_db_for_retrieval = kwargs.get('use_db', False) and self.connected

        if use_db_for_retrieval:
            try:
                # Attempt to get all documents from the database
                all_docs_data = self.db_client.get_all_documents('knowledge')
                if all_docs_data:
                    documents = [doc['text'] for doc in all_docs_data]
                    doc_embeddings = [np.array(json.loads(doc['embedding'])) for doc in all_docs_data]
                else:
                    logger.warning("No documents retrieved from TissDB. Falling back to dummy data if provided.")
            except DatabaseConnectionError as e:
                logger.warning(f"TissDB retrieval failed: {e}. Falling back to dummy data if provided.")
                # Continue to use dummy data if DB retrieval fails
            except Exception as e:
                logger.error(f"Unexpected error during TissDB retrieval: {e}. Falling back to dummy data if provided.", exc_info=True)
        else:
            logger.info("TissDB not used for retrieval (use_db=False or not connected). Using dummy data if provided.")

        # If no documents from DB or DB not used, try to use backward_pass_data (dummy data)
        if not documents and 'backward_pass_data' in kwargs and 'receptor_field' in kwargs['backward_pass_data']:
            receptor_field = kwargs['backward_pass_data']['receptor_field']
            if 'documents' in receptor_field and 'embeddings' in receptor_field:
                documents = receptor_field['documents']
                doc_embeddings = [np.array(emb) for emb in receptor_field['embeddings']]
                logger.info(f"Using {len(documents)} dummy documents for retrieval analysis.")
            else:
                logger.warning("backward_pass_data provided but missing 'documents' or 'embeddings' in 'receptor_field'.")
        
        if not documents:
            logger.warning("No documents available for retrieval analysis (neither from DB nor dummy data). Returning empty list.")
            return []

        try:
            # 2. Embed the query
            query_embedding = self.embedder.embed(query_text)

            # 3. Use the strategy to calculate similarities
            similarities = strategy.calculate_similarity(query_embedding, doc_embeddings, **kwargs)

            if not isinstance(similarities, np.ndarray) or similarities.ndim == 0:
                logger.error(f"Similarity calculation returned a non-array or scalar: {similarities}")
                return []

            # 4. Rank and return top-k documents
            if k >= len(similarities):
                top_k_indices = np.argsort(similarities)[::-1]
            else:
                top_k_indices = np.argpartition(similarities, -k)[-k:]
                top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]

            return [documents[i] for i in top_k_indices]
        except (ValueError, IndexError, TypeError, json.JSONDecodeError) as e:
            logger.error(f"Error during retrieval: {e}", exc_info=True)
            raise ModelProcessingError(f"Failed to retrieve or rank documents: {e}") from e

    def self_update_from_interaction(self, query: str, generated_response: str, user_correction: str = None):
        """
        Adds a new document to the KB based on a user interaction.
        """
        if user_correction:
            document_text = f"Query: {query}\nCorrect Answer: {user_correction}"
            metadata = {'source': 'user_correction'}
        else:
            document_text = f"Query: {query}\nResponse: {generated_response}"
            metadata = {'source': 'generated_response'}
        self.add_document(document_text, metadata=metadata)

    def add_feedback(self, query: str, retrieved_docs: list, feedback_score: int, feedback_text: str):
        """
        Adds user feedback to the database.
        """
        if not self.connected:
            logger.warning("Cannot add feedback: KnowledgeBase is not connected.")
            return
            
        feedback_data = {
            'query': query,
            'retrieved_docs': retrieved_docs,
            'score': feedback_score,
            'text': feedback_text,
            'timestamp': datetime.utcnow().isoformat()
        }
        self.db_client.add_feedback(feedback_data)
        logger.info("Feedback added to knowledge base.")

    def get_knowledge_stats(self) -> dict:
        """
        Gets statistics from the database.
        """
        if not self.connected:
            logger.warning("Cannot get stats: KnowledgeBase is not connected.")
            return {}
        return self.db_client.get_stats()