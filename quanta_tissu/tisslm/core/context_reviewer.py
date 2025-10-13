
import logging
import re
from collections import Counter

logger = logging.getLogger(__name__)

class ContextReviewer:
    """
    A basic context reviewer for analyzing sentences and paragraphs from scratch.
    """
    def __init__(self):
        pass

    def _get_sentences(self, text: str) -> list[str]:
        """
        Helper to split text into sentences.
        """
        return [s.strip() for s in re.split(r'(?<=[.?!])\s+', text) if s.strip()]

    def _get_words(self, text: str) -> list[str]:
        """
        Helper to get words from text.
        """
        return re.findall(r'\b\w+\b', text.lower())

    def check_sentence_transitions(self, text: str, min_overlap_words: int = 1) -> list[dict]:
        """
        Checks for basic word overlap between consecutive sentences as a heuristic for smooth transitions.
        """
        sentences = self._get_sentences(text)
        transition_issues = []

        for i in range(len(sentences) - 1):
            s1_words = set(self._get_words(sentences[i]))
            s2_words = set(self._get_words(sentences[i+1]))
            
            common_words = s1_words.intersection(s2_words)
            if len(common_words) < min_overlap_words:
                transition_issues.append({
                    "sentences": [sentences[i], sentences[i+1]],
                    "overlap": len(common_words),
                    "issue": f"Low word overlap ({len(common_words)}) between sentences."
                })
        return transition_issues

    def check_paragraph_unity(self, text: str, max_unique_keywords_ratio: float = 0.3) -> list[dict]:
        """
        Checks for basic paragraph unity by analyzing unique keywords.
        """
        paragraphs = [p.strip() for p in text.split('\n\n') if p.strip()]
        unity_issues = []

        for i, para in enumerate(paragraphs):
            words = self._get_words(para)
            if not words: continue

            word_counts = Counter(words)
            total_words = sum(word_counts.values())
            unique_words = len(word_counts)

            if total_words > 0 and (unique_words / total_words) > max_unique_keywords_ratio:
                unity_issues.append({
                    "paragraph_index": i,
                    "paragraph_text": para[:100] + "..." if len(para) > 100 else para,
                    "unique_word_ratio": f"{unique_words / total_words:.2f}",
                    "issue": "High ratio of unique words, may lack unity."
                })
        return unity_issues

    def analyze_text(self, text: str):
        """
        Performs a basic context review of the text.
        """
        logger.debug(f"Reviewing text: '{text[:50]}...'\n")
        
        analysis = {
            "sentence_transitions": self.check_sentence_transitions(text),
            "paragraph_unity": self.check_paragraph_unity(text)
        }

        logger.debug(f"Context Review: {analysis}")
        return analysis

# Example Usage (for testing/demonstration)
def main():
    reviewer = ContextReviewer()
    text = (
        "The cat sat on the mat. The mat was old. The dog barked loudly. "
        "It was a sunny day.\n\n" # New paragraph
        "Birds sang in the trees. The trees were green. The sky was blue. "
        "A car drove by. The car was red."
    )

    print("--- Context Reviewer Examples ---")
    reviewer.analyze_text(text)

if __name__ == "__main__":
    import sys
    logging.basicConfig(level=logging.DEBUG, stream=sys.stdout, format='%(levelname)s: %(message)s')
    main()
