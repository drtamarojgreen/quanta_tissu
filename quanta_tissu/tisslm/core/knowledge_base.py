import numpy as np
import json
import requests
from datetime import datetime
from .system_error_handler import DatabaseConnectionError, SystemError
from .model_error_handler import ModelError

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

            # Create the feedback collection
            collection_name = "feedback"
            response = requests.put(f"{self.base_url}/{self.db_name}/{collection_name}")
            if response.status_code not in [201, 200, 409] and "already exists" not in response.text:
                 response.raise_for_status()

            print("Database and collection setup complete.")
        except requests.exceptions.RequestException as e:
            raise DatabaseConnectionError(f"Database setup failed: {e}") from e

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
            raise DatabaseConnectionError(f"Failed to add document to KB: {e}") from e

    def retrieve(self, query_text, k=1, use_db=False, method='cosine', cnn_model=None, ga_params=None, bayes_params=None, backward_pass_data=None):
        """
        Retrieves the top k most relevant documents for a given query text.

        Args:
            query_text (str): The text to search for.
            k (int): The number of documents to retrieve.
            use_db (bool): Whether to use the TissDB knowledge base.
            method (str): The retrieval method to use. One of 'cosine', 'cnn', 'genetic', 'bayes'.
            cnn_model: A trained CNN model for the 'cnn' method.
            ga_params: Parameters for the genetic algorithm for the 'genetic' method.
            bayes_params: Parameters for the forward Bayes estimation method.
            backward_pass_data (dict): Data from the backward pass, including 'receptor_field' and 'hessian_matrix'.
        """
        if not use_db and backward_pass_data is None:
            return []

        query_embedding = self._embed_text(query_text)

        if use_db:
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

            except requests.exceptions.RequestException as e:
                raise DatabaseConnectionError(f"Failed to retrieve from KB: {e}") from e
            except (json.JSONDecodeError, KeyError) as e:
                raise SystemError(f"Failed to parse response from KB: {e}") from e
        
        elif backward_pass_data:
            if 'receptor_field' not in backward_pass_data:
                raise ModelError("backward_pass_data must contain 'receptor_field'.")
            receptor_field = backward_pass_data['receptor_field']
            if 'documents' not in receptor_field or 'embeddings' not in receptor_field:
                 raise ModelError("receptor_field must contain 'documents' and 'embeddings'.")
            documents = receptor_field['documents']
            doc_embeddings = receptor_field['embeddings']

        else:
            return []


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
                raise ModelError("backward_pass_data must contain 'hessian_matrix' for bayes method.")
            hessian_matrix = backward_pass_data['hessian_matrix']
            if bayes_params is None:
                bayes_params = self._get_default_bayes_params()
            similarities = self._bayesian_similarity(query_embedding, doc_embeddings, hessian_matrix, bayes_params)
        
        else:
            raise ModelError(f"Unknown retrieval method: {method}")


        if k >= len(similarities):
            top_k_indices = np.argsort(similarities)[::-1]
        else:
            top_k_indices = np.argpartition(similarities, -k)[-k:]
            top_k_indices = top_k_indices[np.argsort(similarities[top_k_indices])[::-1]]
        
        return [documents[i] for i in top_k_indices]

    def _cosine_similarity(self, query_embedding, doc_embeddings):
        query_norm = np.linalg.norm(query_embedding)
        doc_norms = np.linalg.norm(doc_embeddings, axis=1)

        # Avoid division by zero
        if query_norm == 0 or np.all(doc_norms == 0):
            return np.zeros(len(doc_embeddings))

        return np.dot(doc_embeddings, query_embedding) / (doc_norms * query_norm)

    def _cnn_similarity(self, query_embedding, doc_embeddings, model_params):
        # This is a simplified placeholder for a CNN-based similarity.
        # A real implementation would require a proper model definition and training.
        print(f"Warning: CNN similarity is a placeholder. Using model: {model_params}")
        # Simulate a simple CNN: conv -> pool -> dense
        # We'll use random weights for the simulation.
        n_docs, embedding_dim = np.array(doc_embeddings).shape
        filters = np.random.rand(model_params['filters'], embedding_dim)
        dense_weights = np.random.rand(model_params['filters'])

        similarities = []
        for doc_embedding in doc_embeddings:
            # Simple convolution-like operation
            convolved = np.dot(doc_embedding, filters.T)
            # ReLU activation
            activated = np.maximum(0, convolved)
            # Max pooling
            pooled = np.max(activated)
            # Dense layer
            similarity = np.dot(pooled, dense_weights)
            similarities.append(similarity)
        return np.array(similarities)

    def _genetic_similarity(self, query_embedding, doc_embeddings, ga_params):
        print(f"Warning: Genetic similarity is a placeholder. Using params: {ga_params}")
        # This is a simplified placeholder for a genetic algorithm.
        population_size = ga_params['population_size']
        generations = ga_params['generations']
        mutation_rate = ga_params['mutation_rate']

        # Initialize population with random document indices
        population = np.random.randint(0, len(doc_embeddings), size=(population_size, 1))

        for _ in range(generations):
            # Calculate fitness (cosine similarity)
            fitness = self._cosine_similarity(query_embedding, np.array(doc_embeddings)[population.flatten()])

            # Selection (tournament selection)
            new_population = []
            for _ in range(population_size):
                i, j = np.random.randint(0, population_size, 2)
                if fitness[i] > fitness[j]:
                    new_population.append(population[i])
                else:
                    new_population.append(population[j])
            population = np.array(new_population)

            # Crossover (not implemented for simplicity)

            # Mutation
            for i in range(population_size):
                if np.random.rand() < mutation_rate:
                    population[i] = np.random.randint(0, len(doc_embeddings))

        # Return the fitness of the final population as similarities
        final_fitness = self._cosine_similarity(query_embedding, np.array(doc_embeddings)[population.flatten()])
        similarities = np.zeros(len(doc_embeddings))
        for i, p in enumerate(population):
            similarities[p[0]] = final_fitness[i]
        return similarities

    def _bayesian_similarity(self, query_embedding, doc_embeddings, hessian_matrix, params):
        print(f"Warning: Bayesian similarity is a placeholder. Using params: {params}")
        # This is a simplified placeholder for Bayesian similarity.
        # A real implementation would use the Hessian to model uncertainty.
        eigenvalues = np.array(hessian_matrix['eigenvalues'])
        # Simulate uncertainty by adding noise based on eigenvalues
        uncertainty = 1.0 / (eigenvalues + 1e-6)
        noise = np.random.normal(0, np.mean(uncertainty), size=query_embedding.shape)
        noisy_query = query_embedding + noise

        return self._cosine_similarity(noisy_query, doc_embeddings)

    def _get_default_cnn_model(self):
        print("Warning: Using default CNN model. This is a placeholder.")
        return {"layers": 5, "filters": 128, "activation": "relu"}

    def _get_default_ga_params(self):
        print("Warning: Using default GA parameters. This is a placeholder.")
        return {"population_size": 100, "generations": 50, "mutation_rate": 0.01}

    def _get_default_bayes_params(self):
        print("Warning: Using default Bayes parameters. This is a placeholder.")
        return {"alpha": 1.0, "beta": 1.0}

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
            raise DatabaseConnectionError(f"Failed to get KB stats: {e}") from e

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
            raise DatabaseConnectionError(f"Failed to add feedback to KB: {e}") from e
