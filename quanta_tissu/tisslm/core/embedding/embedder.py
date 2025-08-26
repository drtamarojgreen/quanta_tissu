import numpy as np
import logging

from ..model_error_handler import ModelProcessingError

logger = logging.getLogger(__name__)

class Embedder:
    """
    A class responsible for converting text into vector embeddings.
    """
    def __init__(self, tokenizer, model_embeddings_value):
        """
        Initializes the Embedder.

        Args:
            tokenizer: An object with a `tokenize` method that returns a list of token IDs.
            model_embeddings_value (np.ndarray): The numpy array of the model's embedding weights.
        """
        self.tokenizer = tokenizer
        self.model_embeddings = model_embeddings_value
        self.embedding_dim = model_embeddings_value.shape[1]

    def embed(self, text: str) -> np.ndarray:
        """
        Converts a string of text into a single vector embedding.
        The current strategy is to average the embeddings of the tokens.
        """
        try:
            token_ids = self.tokenizer.tokenize(text)
            if not token_ids:
                logger.warning("Input text for embedding is empty or tokenized to empty.")
                return np.zeros(self.embedding_dim)

            # Ensure token_ids is a numpy array for consistent processing
            token_ids_np = np.array(token_ids)

            # Get the embeddings for the tokens
            embeddings = self.model_embeddings[token_ids_np]

            # Average the embeddings to get a single vector for the text
            return np.mean(embeddings, axis=0)

        except IndexError as e:
            logger.error(f"Token ID out of bounds for model embeddings: {e}", exc_info=True)
            raise ModelProcessingError("Error embedding text: Invalid token ID found.") from e
        except Exception as e:
            logger.error(f"An unexpected error occurred during text embedding: {e}", exc_info=True)
            raise ModelProcessingError(f"An unexpected error occurred during text embedding: {e}") from e
