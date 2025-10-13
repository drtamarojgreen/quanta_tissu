import os
import json
from .tokenizer import Tokenizer

class SentimentAnalyzer:
    """
    A lexicon-based sentiment analyzer that loads its lexicon from a file.
    """
    def __init__(self, tokenizer: Tokenizer, sentiment_lexicon_path: str):
        self.tokenizer = tokenizer
        self.sentiment_lexicon_path = sentiment_lexicon_path
        self.sentiment_scores = {}
        self.positive_token_ids = []
        self.negative_token_ids = []
        self._load_lexicon()

    def _load_lexicon(self):
        """Loads the sentiment lexicon from the specified file."""
        if not os.path.exists(self.sentiment_lexicon_path):
            print(f"Warning: Sentiment lexicon file not found at {self.sentiment_lexicon_path}. Sentiment analysis will be disabled.")
            return
        
        try:
            with open(self.sentiment_lexicon_path, 'r', encoding='utf-8') as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith('#'): # Skip empty lines and comments
                        continue
                    parts = line.split(',')
                    if len(parts) == 2:
                        word = parts[0].strip().lower()
                        try:
                            score = float(parts[1].strip())
                            self.sentiment_scores[word] = score
                        except ValueError:
                            print(f"Warning: Invalid score for word '{word}' in lexicon. Skipping.")
                    else:
                        print(f"Warning: Invalid line format in lexicon: '{line}'. Skipping.")
            
            # Populate positive and negative token IDs based on loaded scores
            for word, score in self.sentiment_scores.items():
                if score > 0:
                    self.positive_token_ids.extend(self._get_token_ids_for_word(word))
                elif score < 0:
                    self.negative_token_ids.extend(self._get_token_ids_for_word(word))
            
            # Remove duplicates from token ID lists
            self.positive_token_ids = list(set(self.positive_token_ids))
            self.negative_token_ids = list(set(self.negative_token_ids))

            print(f"Loaded {len(self.sentiment_scores)} words into sentiment lexicon. Positive token IDs: {len(self.positive_token_ids)}, Negative token IDs: {len(self.negative_token_ids)}.")

        except IOError as e:
            print(f"Error reading sentiment lexicon file {self.sentiment_lexicon_path}: {e}")

    def _get_token_ids_for_word(self, word: str) -> list[int]:
        """Converts a single word to its corresponding token IDs, including leading space variant."""
        token_ids = []
        ids = self.tokenizer.tokenize(word)
        token_ids.extend(ids)
        ids_with_space = self.tokenizer.tokenize(' ' + word)
        token_ids.extend(ids_with_space)
        return list(set(token_ids)) # Return unique token IDs

    def get_sentiment_bias(self, sentiment: str, strength: float = 0.5) -> dict[int, float]:
        """
        Creates a bias dictionary to be used in the generation process.

        Args:
            sentiment (str): The desired sentiment, either 'positive' or 'negative'.
            strength (float): The base strength of the bias to apply.

        Returns:
            dict[int, float]: A dictionary mapping token IDs to a bias value.
        """
        bias_dict = {}
        target_token_ids = []

        if sentiment == 'positive':
            target_token_ids = self.positive_token_ids
        elif sentiment == 'negative':
            target_token_ids = self.negative_token_ids
        else:
            return {}
        
        for token_id in target_token_ids:
            # Apply bias based on the provided strength
            bias_dict[token_id] = strength
        
        return bias_dict