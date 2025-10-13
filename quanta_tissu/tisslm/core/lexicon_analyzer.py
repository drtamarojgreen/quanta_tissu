
import logging
import re
from collections import Counter

logger = logging.getLogger(__name__)

class LexiconAnalyzer:
    """
    A basic lexicon analyzer for identifying words and word combinations from scratch.
    """
    def __init__(self):
        pass

    def identify_multi_word_expressions(self, text: str, mwe_list: list[str]) -> list[str]:
        """
        Identifies predefined multi-word expressions in the text.
        """
        found_mwe = []
        text_lower = text.lower()
        for mwe in mwe_list:
            if mwe.lower() in text_lower:
                found_mwe.append(mwe)
        return found_mwe

    def identify_collocations(self, text: str, n_gram: int = 2, top_n: int = 10) -> list[tuple[str, int]]:
        """
        Identifies frequent adjacent word n-grams (collocations).
        """
        words = re.findall(r'\b\w+\b', text.lower())
        if len(words) < n_gram: return []

        ngrams = []
        for i in range(len(words) - n_gram + 1):
            ngrams.append(tuple(words[i:i+n_gram]))
        
        return Counter(ngrams).most_common(top_n)

    def analyze_text(self, text: str, mwe_list: list[str] = None, n_gram: int = 2, top_n_collocations: int = 10):
        """
        Performs a basic lexical analysis of the text.
        """
        logger.debug(f"Analyzing text: '{text[:50]}...'")
        
        analysis = {}
        if mwe_list:
            analysis["found_mwe"] = self.identify_multi_word_expressions(text, mwe_list)
        
        analysis["collocations"] = self.identify_collocations(text, n_gram, top_n_collocations)

        logger.debug(f"Lexical Analysis: {analysis}")
        return analysis

# Example Usage (for testing/demonstration)
def main():
    analyzer = LexiconAnalyzer()
    text = "The quick brown fox jumps over the lazy dog. The dog barks loudly. Quick brown fox is fast."
    
    mwe_examples = ["quick brown fox", "jumps over", "lazy dog"]

    print("--- Lexicon Analyzer Examples ---")
    analyzer.analyze_text(text, mwe_examples, n_gram=2, top_n_collocations=5)
    analyzer.analyze_text(text, mwe_examples, n_gram=3, top_n_collocations=3)

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, stream=sys.stdout, format='%(levelname)s: %(message)s')
    main()
