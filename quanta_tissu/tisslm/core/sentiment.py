
from .tokenizer import Tokenizer

# A simple, built-in lexicon for sentiment analysis.
# In a real-world scenario, this would be much larger.
POSITIVE_WORDS = [
    'love', 'happy', 'good', 'great', 'excellent', 'amazing', 'wonderful', 'joy',
    'success', 'beautiful', 'friendly', 'kind', 'generous', 'awesome'
]

NEGATIVE_WORDS = [
    'hate', 'sad', 'bad', 'terrible', 'horrible', 'awful', 'pain', 'fear',
    'failure', 'ugly', 'mean', 'cruel', 'selfish', 'boring'
]

class SentimentAnalyzer:
    """
    A simple, lexicon-based sentiment analyzer to bias text generation.
    """
    def __init__(self, tokenizer: Tokenizer):
        self.tokenizer = tokenizer
        self.positive_token_ids = self._get_token_ids_for_words(POSITIVE_WORDS)
        self.negative_token_ids = self._get_token_ids_for_words(NEGATIVE_WORDS)

    def _get_token_ids_for_words(self, words: list[str]) -> list[int]:
        """Converts a list of words to their corresponding token IDs."""
        token_ids = set()
        for word in words:
            # A word might be tokenized into multiple IDs. We add all of them.
            # We also try with a leading space, as tokenizers often treat words differently
            # depending on their position.
            ids = self.tokenizer.tokenize(word)
            for i in ids:
                token_ids.add(i)
            ids_with_space = self.tokenizer.tokenize(' ' + word)
            for i in ids_with_space:
                token_ids.add(i)
        return list(token_ids)

    def get_sentiment_bias(self, sentiment: str, strength: float = 0.5) -> dict[int, float]:
        """
        Creates a bias dictionary to be used in the generation process.

        Args:
            sentiment (str): The desired sentiment, either 'positive' or 'negative'.
            strength (float): The strength of the bias to apply.

        Returns:
            dict[int, float]: A dictionary mapping token IDs to a bias value.
        """
        bias_dict = {}
        if sentiment == 'positive':
            for token_id in self.positive_token_ids:
                bias_dict[token_id] = strength
        elif sentiment == 'negative':
            for token_id in self.negative_token_ids:
                bias_dict[token_id] = strength # Positive bias to increase probability
        else:
            return {}
        
        return bias_dict
