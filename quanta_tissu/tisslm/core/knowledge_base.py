import numpy as np
import json
import logging
from datetime import datetime

from .db.client import TissDBClient
from .embedding.embedder import Embedder
from .retrieval.strategy import RetrievalStrategy
from .retrieval.semantic import CosineSimilarityStrategy
from .retrieval.chunking import ChunkingStrategy
from .model_error_handler import ModelProcessingError, InputValidationError
from .system_error_handler import DatabaseConnectionError # Import DatabaseConnectionError

logger = logging.getLogger(__name__)

class KnowledgeBase:
    """
    A facade for the knowledge storage and retrieval system.
    It coordinates an embedder, a database client, and retrieval strategies.
    """
    def __init__(self, embedder: Embedder, db_client: TissDBClient = None):
        """
        Initializes the KnowledgeBase.

        Args:
            embedder: An instance of the Embedder class.
            db_client: Optional. An instance of the TissDBClient class. If None,
                       the KnowledgeBase operates in an in-memory only mode.
        """
        self.embedder = embedder
        self.db_client = db_client
        self.connected = False
        self._local_document_cache = [] # Initialize local cache
        self.query_cache = {}
        
        if self.db_client:
            try:
                # The collections the KB requires
                collections = ["knowledge", "knowledge_feedback", "feedback"]
                self.connected = self.db_client.ensure_db_setup(collections)
            except Exception as e:
                logger.warning(f"KnowledgeBase could not connect to DB. Operating in disconnected mode. Error: {e}")
        else:
            logger.info("KnowledgeBase initialized in in-memory only mode (no database client provided).")

    def add_document(self, text: str, metadata: dict = None, chunking_strategy: ChunkingStrategy = None):
        """
        Embeds a document and adds it to the knowledge base.
        If a chunking_strategy is provided, the document is split into chunks,
        and each chunk is added as a separate document.
        """
        if chunking_strategy:
            chunks = chunking_strategy.chunk(text)
            logger.info(f"Splitting document into {len(chunks)} chunks using {chunking_strategy.__class__.__name__}.")
        else:
            chunks = [text]

        for i, chunk_text in enumerate(chunks):
            embedding = self.embedder.embed(chunk_text)
            
            chunk_metadata = metadata.copy() if metadata else {}
            chunk_metadata['source'] = chunk_metadata.get('source', 'user_input')
            if len(chunks) > 1:
                chunk_metadata['chunk_index'] = i
                chunk_metadata['total_chunks'] = len(chunks)

            doc = {
                'text': chunk_text.replace('\n', '\\n'),
                'embedding': json.dumps(embedding.tolist()),
                'timestamp': datetime.utcnow().isoformat(),
                **chunk_metadata
            }

            # Add to local cache
            self._local_document_cache.append({'text': chunk_text, 'embedding': embedding})

            if not self.connected:
                logger.warning("Cannot add document to DB: KnowledgeBase is not connected. Document added to local cache only.")
                continue # Continue to next chunk if not connected

            self.db_client.add_document('knowledge', doc)
            logger.info(f"Added document chunk {i+1}/{len(chunks)} to knowledge base.")

    def retrieve(self, query_text: str, strategy: RetrievalStrategy = CosineSimilarityStrategy(), k: int = 1, time_decay_factor: float = None, **kwargs) -> tuple[list, np.ndarray]:
        """
        Retrieves the top-k most relevant documents for a query using a given strategy.
        Applies time-based re-ranking if time_decay_factor is provided.
        Returns a tuple of (documents, scores).
        """
        query_embedding = self.embedder.embed(query_text)
        
        # Semantic Caching Logic
        cache_threshold = kwargs.get('cache_threshold', 0.98)
        for cached_query_embedding, cached_result in self.query_cache.values():
            similarity = np.dot(query_embedding, cached_query_embedding) / (np.linalg.norm(query_embedding) * np.linalg.norm(cached_query_embedding))
            if similarity > cache_threshold:
                logger.info(f"Returning semantically cached result for query: {query_text} (similarity: {similarity:.4f})")
                return cached_result

        documents = []
        doc_embeddings = []
        document_timestamps = [] # To store timestamps for re-ranking

        # Check if use_db is explicitly set to True in kwargs and if connected
        use_db_for_retrieval = kwargs.get('use_db', False) and self.connected

        if use_db_for_retrieval:
            try:
                all_docs_data = self.db_client.get_all_documents('knowledge')
                if all_docs_data:
                    documents = [doc['text'] for doc in all_docs_data]
                    doc_embeddings = [np.array(json.loads(doc['embedding'])) for doc in all_docs_data]
                    document_timestamps = [datetime.fromisoformat(doc['timestamp']) for doc in all_docs_data]
                    logger.info(f"Retrieved {len(documents)} documents from TissDB.")
                else:
                    logger.warning("No documents retrieved from TissDB. Using local cache for retrieval.")
            except DatabaseConnectionError as e:
                logger.warning(f"TissDB retrieval failed: {e}. Using local cache for retrieval.")
            except Exception as e:
                logger.error(f"Unexpected error during TissDB retrieval: {e}. Using local cache for retrieval.", exc_info=True)
        
        # Fallback to local cache if DB retrieval failed or not used
        if not documents and self._local_document_cache:
            documents = [d['text'] for d in self._local_document_cache]
            doc_embeddings = [d['embedding'] for d in self._local_document_cache]
            # Assuming local cache also stores metadata with timestamp
            # This part needs refinement if _local_document_cache only stores text and embedding
            # For now, using current time as a placeholder for local cache documents if no timestamp is available
            document_timestamps = [datetime.utcnow()] * len(self._local_document_cache)
            logger.info(f"Using {len(documents)} documents from local cache for retrieval.")

        if not documents:
            logger.warning("No documents available for retrieval analysis (neither from DB nor local cache). Returning empty list.")
            return [], np.array([])

        try:
            similarities = strategy.calculate_similarity(query_embedding, doc_embeddings, **kwargs)

            if not isinstance(similarities, np.ndarray) or similarities.ndim == 0:
                logger.error(f"Similarity calculation returned a non-array or scalar: {similarities}")
                return [], np.array([])

            # Apply time-based re-ranking
            if time_decay_factor is not None and document_timestamps:
                current_time = datetime.utcnow()
                decayed_scores = np.copy(similarities)
                for i, timestamp in enumerate(document_timestamps):
                    time_diff_seconds = (current_time - timestamp).total_seconds()
                    # Apply exponential decay: score = original_score * exp(-time_decay_factor * time_diff_seconds)
                    # Or a simpler linear boost for recency: score = original_score + (time_decay_factor / (time_diff_seconds + 1))
                    # Let's use a simple linear boost for now, assuming time_decay_factor is a small positive number
                    decayed_scores[i] += time_decay_factor / (time_diff_seconds / (3600 * 24) + 1) # Boost for recency, scaled by days

                similarities = decayed_scores
                logger.info(f"Applied time-based re-ranking with factor: {time_decay_factor}")

            if k >= len(similarities):
                top_k_indices = np.argsort(similarities)[::-1]
            else:
                top_k_indices = np.argpartition(similarities, -k)[-k:]
                top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]

            result_docs = [documents[i] for i in top_k_indices]
            result_scores = similarities[top_k_indices]
            
            # Store the embedding along with the result in the cache
            self.query_cache[query_text] = (query_embedding, (result_docs, result_scores))
            return result_docs, result_scores
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
        Computes and returns statistics about the knowledge base.
        If connected to the database, it will attempt to retrieve stats from there.
        Otherwise, it computes stats from the local document cache.
        """
        stats = {}
        documents_to_analyze = []

        if self.connected:
            try:
                # Attempt to get all documents from the database for stats calculation
                all_docs_data = self.db_client.get_all_documents('knowledge')
                if all_docs_data:
                    documents_to_analyze = [doc['text'] for doc in all_docs_data]
                    logger.info(f"Analyzing {len(documents_to_analyze)} documents from TissDB for stats.")
                else:
                    logger.warning("No documents retrieved from TissDB for stats. Using local cache.")
            except DatabaseConnectionError as e:
                logger.warning(f"TissDB stats retrieval failed: {e}. Using local cache for stats.")
            except Exception as e:
                logger.error(f"Unexpected error during TissDB stats retrieval: {e}. Using local cache for stats.", exc_info=True)
        
        if not documents_to_analyze and self._local_document_cache:
            documents_to_analyze = [d['text'] for d in self._local_document_cache]
            logger.info(f"Analyzing {len(documents_to_analyze)} documents from local cache for stats.")

        if not documents_to_analyze:
            return {
                'total_documents': 0,
                'total_tokens': 0,
                'average_document_length': 0,
                'unique_sources': []
            }

        total_documents = len(documents_to_analyze)
        total_tokens = 0
        sources = set()

        for doc_text in documents_to_analyze:
            # Simple tokenization by splitting on whitespace
            tokens = doc_text.split()
            total_tokens += len(tokens)
            # Note: To get 'source' metadata, we'd need to store it with the text in _local_document_cache
            # For now, we'll assume 'source' is part of the metadata if available, or default.
            # This part needs refinement if metadata is not directly accessible here.
            # For now, we'll just use a placeholder for sources.
            sources.add('unknown' if 'source' not in doc_text else 'known') # Placeholder

        average_document_length = total_tokens / total_documents if total_documents > 0 else 0

        stats = {
            'total_documents': total_documents,
            'total_tokens': total_tokens,
            'average_document_length': average_document_length,
            'unique_sources': list(sources)
        }
        return stats