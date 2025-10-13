import numpy as np
from .strategy import RetrievalStrategy
import math
from collections import defaultdict

class CosineSimilarityStrategy(RetrievalStrategy):
    """
    Calculates similarity using cosine similarity.
    """
    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        doc_embeddings_np = np.array(doc_embeddings)
        query_norm = np.linalg.norm(query_embedding)
        doc_norms = np.linalg.norm(doc_embeddings_np, axis=1)

        similarities = np.zeros(len(doc_embeddings_np))

        # Handle cases where query_norm is zero
        if query_norm == 0:
            return similarities

        # Calculate similarities for non-zero document embeddings
        non_zero_doc_indices = doc_norms != 0
        if np.any(non_zero_doc_indices):
            similarities[non_zero_doc_indices] = np.dot(doc_embeddings_np[non_zero_doc_indices], query_embedding) / (doc_norms[non_zero_doc_indices] * query_norm)

        return similarities

class CNNSimilarityStrategy(RetrievalStrategy):
    """
    A simplified CNN-based similarity model.
    Filters are derived from simple mathematical patterns to perform feature extraction.
    """
    def __init__(self, embedding_dim):
        # Generate random filters based on the embedding dimension
        # For a simplified CNN, we can assume a fixed number of filters (e.g., 3 or 5)
        # and each filter's length matches the embedding_dim.
        num_filters = 5  # Example: using 5 random filters
        self.filters = np.random.rand(num_filters, embedding_dim) * 2 - 1 # Random values between -1 and 1

        # Dense weights are now calculated based on the number of filters
        self.dense_weights = np.full(num_filters, 1.0 / num_filters) # Equal contribution from each filter

    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        doc_embeddings_np = np.array(doc_embeddings)

        # Extract features from query embedding
        query_features = np.dot(query_embedding, self.filters.T)
        query_activated = np.maximum(0, query_features)
        query_score = np.dot(query_activated, self.dense_weights)

        # Extract features from document embeddings
        convolved_docs = np.dot(doc_embeddings_np, self.filters.T)
        activated_docs = np.maximum(0, convolved_docs)
        doc_scores = np.dot(activated_docs, self.dense_weights)

        # Calculate similarity as the difference between query score and doc scores
        # A smaller difference means higher similarity
        similarities = 1.0 - np.abs(query_score - doc_scores) / (np.max(doc_scores) - np.min(doc_scores) + 1e-6) # Normalize difference
        similarities = np.clip(similarities, 0, 1) # Ensure similarities are between 0 and 1
        
        return similarities


class GeneticSimilarityStrategy(RetrievalStrategy):
    """
    A simplified genetic algorithm-based similarity search.
    """
    def get_default_ga_params(self):
        return {"population_size": 100, "generations": 50, "mutation_rate": 0.01, "crossover_rate": 0.7}

    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        np.random.seed(42) # For deterministic testing
        ga_params = kwargs.get('ga_params', self.get_default_ga_params())
        doc_embeddings_np = np.array(doc_embeddings)

        population_size = ga_params['population_size']
        generations = ga_params['generations']
        mutation_rate = ga_params['mutation_rate']
        crossover_rate = ga_params['crossover_rate']

        # Initialize population with random indices
        population = np.random.randint(0, len(doc_embeddings_np), size=(population_size, 1))

        for gen in range(generations):
            # Calculate fitness of the current population
            pop_embeddings = doc_embeddings_np[population.flatten()]
            fitness = CosineSimilarityStrategy().calculate_similarity(query_embedding, pop_embeddings)

            # Selection (tournament selection)
            selected_indices = []
            for _ in range(population_size):
                idx1, idx2 = np.random.randint(0, population_size, 2)
                if fitness[idx1] > fitness[idx2]:
                    selected_indices.append(idx1)
                else:
                    selected_indices.append(idx2)
            selected_population = population[selected_indices]

            # Crossover
            next_population = []
            for i in range(0, population_size, 2):
                parent1 = selected_population[i]
                parent2 = selected_population[i+1] if i+1 < population_size else selected_population[0] # Handle odd population size

                if np.random.rand() < crossover_rate:
                    # Simple one-point crossover (for single-gene individuals, this is just picking one parent)
                    child1 = parent1 if np.random.rand() < 0.5 else parent2
                    child2 = parent2 if np.random.rand() < 0.5 else parent1
                else:
                    child1, child2 = parent1, parent2
                next_population.extend([child1, child2])
            population = np.array(next_population[:population_size]) # Trim if odd population size

            # Mutation
            for i in range(population_size):
                if np.random.rand() < mutation_rate:
                    population[i] = np.random.randint(0, len(doc_embeddings_np)) # Mutate to a random document index

        # Calculate final similarities based on the evolved population
        final_pop_embeddings = doc_embeddings_np[population.flatten()]
        final_fitness = CosineSimilarityStrategy().calculate_similarity(query_embedding, final_pop_embeddings)

        # Map fitness back to the original document indices
        similarities = np.zeros(len(doc_embeddings_np))
        for i, p_index in enumerate(population.flatten()):
            similarities[p_index] = max(similarities[p_index], final_fitness[i]) # Keep the best fitness for each doc
        return similarities

class BayesianSimilarityStrategy(RetrievalStrategy):
    """
    A simplified Bayesian-based similarity approach.
    This implementation uses the Hessian eigenvalues to model uncertainty
    and samples a noisy query from a simplified posterior distribution.
    """
    def calculate_similarity(self, query_embedding, doc_embeddings, **kwargs):
        # For deterministic testing
        np.random.seed(42)

        hessian_matrix = kwargs.get('hessian_matrix')
        if hessian_matrix is None:
            raise ValueError("BayesianSimilarityStrategy requires a 'hessian_matrix' in kwargs.")

        # FIX: Handle both NumPy array and dictionary for Hessian data
        if isinstance(hessian_matrix, np.ndarray):
            # If it's a matrix, compute the eigenvalues.
            # Assuming the Hessian is symmetric, which it should be.
            eigenvalues = np.linalg.eigvalsh(hessian_matrix)
        elif isinstance(hessian_matrix, dict):
            # For backward compatibility or other use cases, allow passing eigenvalues directly.
            if 'eigenvalues' not in hessian_matrix:
                raise ValueError("Hessian dictionary must contain 'eigenvalues' key.")
            eigenvalues = np.array(hessian_matrix['eigenvalues'])
        else:
            raise TypeError(f"Unsupported type for 'hessian_matrix': {type(hessian_matrix)}")
        
        # Simplified Bayesian update:
        # Assume query_embedding is the prior mean.
        # Use eigenvalues to define a simplified precision (inverse variance) for the likelihood.
        # A larger eigenvalue means less uncertainty in that dimension.
        
        # Ensure eigenvalues match query_embedding dimension, or use a default if not provided
        if len(eigenvalues) != len(query_embedding):
            # If eigenvalues don't match, use a scalar uncertainty for all dimensions
            mean_uncertainty = 1.0 / (np.mean(eigenvalues) + 1e-6) if len(eigenvalues) > 0 else 1.0
            posterior_variance = np.full(query_embedding.shape, mean_uncertainty)
        else:
            posterior_variance = 1.0 / (eigenvalues + 1e-6) # Simplified: inverse of eigenvalues as variance

        # Sample a noisy query from a Gaussian centered at the original query_embedding
        # with variance derived from the Hessian eigenvalues.
        # This simulates drawing from a posterior distribution where the Hessian informs uncertainty.
        # Ensure posterior_variance is non-negative before taking square root
        posterior_variance = np.maximum(posterior_variance, 1e-9) # Clamp to a small positive value
        noisy_query = np.random.normal(query_embedding, np.sqrt(posterior_variance), size=query_embedding.shape)

        return CosineSimilarityStrategy().calculate_similarity(noisy_query, doc_embeddings)

class BM25:
    """
    BM25 is a keyword-based search algorithm, not a semantic one.
    It is placed here as per user request.
    """
    def __init__(self, corpus, tokenizer, k1=1.5, b=0.75):
        self.k1 = k1
        self.b = b
        self.tokenizer = tokenizer
        self.corpus = corpus
        self.doc_len = [len(doc) for doc in corpus]
        self.avgdl = np.mean(self.doc_len)
        self.doc_freq = defaultdict(int)
        self.term_freq = []
        self.doc_count = len(corpus)

        for doc in corpus:
            frequencies = defaultdict(int)
            for term in doc:
                frequencies[term] += 1
            self.term_freq.append(frequencies)

            for term in frequencies:
                self.doc_freq[term] += 1

    def _calculate_idf(self, term):
        return math.log(1 + (self.doc_count - self.doc_freq[term] + 0.5) / (self.doc_freq[term] + 0.5))

    def search(self, query):
        query_terms = self.tokenizer(query)
        scores = np.zeros(self.doc_count)

        for i in range(self.doc_count):
            for term in query_terms:
                if term in self.term_freq[i]:
                    idf = self._calculate_idf(term)
                    tf = self.term_freq[i][term]
                    numerator = tf * (self.k1 + 1)
                    denominator = tf + self.k1 * (1 - self.b + self.b * self.doc_len[i] / self.avgdl)
                    scores[i] += idf * (numerator / denominator)
        
        # Return documents sorted by score
        return sorted(zip(range(self.doc_count), scores), key=lambda x: x[1], reverse=True)