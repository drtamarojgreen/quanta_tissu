import numpy as np
import json
import requests
import logging
from datetime import datetime
from .model_error_handler import TissModelError, InputValidationError, ModelProcessingError, ConfigurationError
from .system_error_handler import DatabaseConnectionError, handle_errors

logger = logging.getLogger(__name__)

class KnowledgeBase:
    """
    A knowledge storage and retrieval system that uses the TissDB HTTP API.
    """
    def __init__(self, model_embeddings, tokenizer, db_host='127.0.0.1', db_port=8080, db_name='testdb'):
        """
        Initializes the KnowledgeBase.
        """
        self.base_url = f"http://{db_host}:{db_port}"
        self.db_name = db_name
        self.model_embeddings = model_embeddings
        self.tokenizer = tokenizer
        self.connected = False
        self._setup_database()

    def _setup_database(self):
        """
        Ensures the database and the 'knowledge' collection exist.
        """
        try:
            response = requests.put(f"{self.base_url}/{self.db_name}")
            if response.status_code not in [201, 200, 409] and "already exists" not in response.text:
                 response.raise_for_status()

            for collection_name in ["knowledge", "knowledge_feedback"]:
                response = requests.put(f"{self.base_url}/{self.db_name}/{collection_name}")
                if response.status_code not in [201, 200, 409] and "already exists" not in response.text:
                    response.raise_for_status()

            collection_name = "feedback"
            response = requests.put(f"{self.base_url}/{self.db_name}/{collection_name}")
            if response.status_code not in [201, 200, 409] and "already exists" not in response.text:
                 response.raise_for_status()
            
            self.connected = True
            logger.info("Database and collection setup complete.")

        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Database setup failed: {e}") from e
            logger.warning(f"Database setup failed: {e}. KnowledgeBase will operate in disconnected mode.")

    def _embed_text(self, text):
        try:
            token_ids = self.tokenizer.tokenize(text)
            if not isinstance(token_ids, np.ndarray):
                # Ensure token_ids is a numpy array for consistent processing
                token_ids = np.array(token_ids)
            if token_ids.size == 0:
                logger.warning("Input text for embedding is empty or tokenized to empty.")
                return np.zeros(self.model_embeddings.shape[1])

            embeddings = self.model_embeddings.value[token_ids]
            return np.mean(embeddings, axis=0)
        except IndexError as e:
            logger.error(f"Token ID out of bounds for model embeddings: {e}", exc_info=True)
            raise ModelProcessingError(f"Error embedding text: Invalid token ID found.") from e
        except Exception as e:
            logger.error(f"An unexpected error occurred during text embedding: {e}", exc_info=True)
            raise ModelProcessingError(f"An unexpected error occurred during text embedding: {e}") from e

    @handle_errors
    def add_document(self, text, metadata=None):
        if not self.connected:
            logger.warning("Cannot add document: KnowledgeBase is not connected to the database.")
            return

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
            response = requests.post(f"{self.base_url}/{self.db_name}/knowledge", json=doc)
            response.raise_for_status()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to add document to KB: {e}") from e

    @handle_errors
    def retrieve(self, query_text, k=1, use_db=False, method='cosine', cnn_model=None, ga_params=None, bayes_params=None, backward_pass_data=None):
        if not use_db and backward_pass_data is None:
            return []

        query_embedding = self._embed_text(query_text)

        if use_db:
            if not self.connected:
                logger.warning("Cannot retrieve from DB: KnowledgeBase is not connected.")
                return []
            try:
                query = {"query": "SELECT id, text, embedding FROM knowledge"}
                response = requests.post(f"{self.base_url}/{self.db_name}/knowledge/_query", json=query)
                response.raise_for_status()
                all_docs_data = response.json()

                if not all_docs_data:
                    return []

                doc_embeddings = [np.array(json.loads(doc['embedding'])) for doc in all_docs_data]
                documents = [doc['text'] for doc in all_docs_data]

            except requests.exceptions.RequestException as e:
                raise DatabaseConnectionError(f"Failed to retrieve from KB: {e}") from e
            except (json.JSONDecodeError, KeyError) as e:
                raise ModelProcessingError(f"Failed to parse response from KB: {e}") from e
        
        elif backward_pass_data:
            if 'receptor_field' not in backward_pass_data:
                raise InputValidationError("backward_pass_data must contain 'receptor_field'.")
            receptor_field = backward_pass_data['receptor_field']
            if 'documents' not in receptor_field or 'embeddings' not in receptor_field:
                 raise InputValidationError("receptor_field must contain 'documents' and 'embeddings'.")
            documents = receptor_field['documents']
            doc_embeddings = receptor_field['embeddings']

        else:
            return []

        try:
            if method == 'cosine':
                similarities = self._cosine_similarity(query_embedding, doc_embeddings)
            elif method == 'cnn':
                if cnn_model is None:
                    cnn_model = self._get_default_cnn_model()
                similarities = self._cnn_similarity(query_embedding, doc_embeddings, cnn_model)
            elif method == 'genetic':
                if ga_params is None:
                    ga_params = self._get_default_ga_params()
                similarities = self._genetic_similarity(query_embedding, doc_embeddings, ga_params)
            elif method == 'bayes':
                if 'hessian_matrix' not in backward_pass_data:
                    raise InputValidationError("backward_pass_data must contain 'hessian_matrix' for bayes method.")
                hessian_matrix = backward_pass_data['hessian_matrix']
                if bayes_params is None:
                    bayes_params = self._get_default_bayes_params()
                similarities = self._bayesian_similarity(query_embedding, doc_embeddings, hessian_matrix, bayes_params)
            else:
                raise ConfigurationError(f"Unknown retrieval method: {method}")

            if not isinstance(similarities, np.ndarray):
                similarities = np.array(similarities)

            if k >= len(similarities):
                top_k_indices = np.argsort(similarities)[::-1]
            else:
                top_k_indices = np.argpartition(similarities, -k)[-k:]
                top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]

            return [documents[i] for i in top_k_indices]
        except (ValueError, IndexError, TypeError) as e:
            # Catch common numpy/data-related errors during similarity calculation
            logger.error(f"Error during similarity calculation or indexing in retrieve: {e}", exc_info=True)
            raise ModelProcessingError(f"Failed to calculate similarities or rank documents: {e}") from e

    def _cosine_similarity(self, query_embedding, doc_embeddings):
        query_norm = np.linalg.norm(query_embedding)
        doc_norms = np.linalg.norm(doc_embeddings, axis=1)

        if query_norm == 0 or np.all(doc_norms == 0):
            return np.zeros(len(doc_embeddings))

        return np.dot(doc_embeddings, query_embedding) / (doc_norms * query_norm)

    def _cnn_similarity(self, query_embedding, doc_embeddings, model_params):
        n_docs, embedding_dim = np.array(doc_embeddings).shape
        filters = np.random.rand(model_params['filters'], embedding_dim)
        dense_weights = np.random.rand(model_params['filters'])

        similarities = []
        for doc_embedding in doc_embeddings:
            convolved = np.dot(doc_embedding, filters.T)
            activated = np.maximum(0, convolved)
            pooled = np.max(activated)
            similarity = np.dot(pooled, dense_weights)
            similarities.append(similarity)
        return np.array(similarities)

    def _genetic_similarity(self, query_embedding, doc_embeddings, ga_params):
        population_size = ga_params['population_size']
        generations = ga_params['generations']
        mutation_rate = ga_params['mutation_rate']

        population = np.random.randint(0, len(doc_embeddings), size=(population_size, 1))

        for _ in range(generations):
            fitness = self._cosine_similarity(query_embedding, np.array(doc_embeddings)[population.flatten()])

            new_population = []
            for _ in range(population_size):
                i, j = np.random.randint(0, population_size, 2)
                if fitness[i] > fitness[j]:
                    new_population.append(population[i])
                else:
                    new_population.append(population[j])
            population = np.array(new_population)

            for i in range(population_size):
                if np.random.rand() < mutation_rate:
                    population[i] = np.random.randint(0, len(doc_embeddings))

        final_fitness = self._cosine_similarity(query_embedding, np.array(doc_embeddings)[population.flatten()])
        similarities = np.zeros(len(doc_embeddings))
        for i, p in enumerate(population):
            similarities[p[0]] = final_fitness[i]
        return similarities

    def _bayesian_similarity(self, query_embedding, doc_embeddings, hessian_matrix, params):
        eigenvalues = np.array(hessian_matrix['eigenvalues'])
        uncertainty = 1.0 / (eigenvalues + 1e-6)
        noise = np.random.normal(0, np.mean(uncertainty), size=query_embedding.shape)
        noisy_query = query_embedding + noise

        return self._cosine_similarity(noisy_query, doc_embeddings)

    def _get_default_cnn_model(self):
        return {"layers": 5, "filters": 128, "activation": "relu"}

    def _get_default_ga_params(self):
        return {"population_size": 100, "generations": 50, "mutation_rate": 0.01}

    def _get_default_bayes_params(self):
        return {"alpha": 1.0, "beta": 1.0}

    @handle_errors
    def self_update_from_interaction(self, query, generated_response, user_correction=None):
        if not self.connected:
            logger.warning("Cannot self-update: KnowledgeBase is not connected.")
            return

        if user_correction:
            document_text = f"Query: {query} Correct Answer: {user_correction}"
            metadata = {'source': 'user_correction'}
        else:
            document_text = f"Query: {query} Response: {generated_response}"
            metadata = {'source': 'generated_response'}

        self.add_document(document_text, metadata=metadata)

    @handle_errors
    def get_knowledge_stats(self):
        if not self.connected:
            logger.warning("Cannot get stats: KnowledgeBase is not connected.")
            return {}
        try:
            response = requests.get(f"{self.base_url}/{self.db_name}/_stats")
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to get KB stats: {e}") from e

    @handle_errors
    def add_feedback(self, query, retrieved_docs, feedback_score, feedback_text):
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
        try:
            response = requests.post(f"{self.base_url}/{self.db_name}/_feedback", json=feedback_data)
            response.raise_for_status()
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Failed to add feedback to KB: {e}") from e
