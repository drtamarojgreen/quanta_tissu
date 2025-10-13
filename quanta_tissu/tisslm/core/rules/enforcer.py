import logging
import re
import math

logger = logging.getLogger(__name__)

# --- Lexicons (Simple, from scratch) ---
# These are very basic and can be expanded.
COMMON_PRONOUNS = {
    "i", "you", "he", "she", "it", "we", "they",
    "me", "him", "her", "us", "them",
    "my", "your", "his", "her", "its", "our", "their",
    "mine", "yours", "hers", "ours", "theirs"
}

COMMON_VERB_SUFFIXES = {"ing", "ed", "s", "es", "d", "en", "n"}
COMMON_ADJECTIVE_SUFFIXES = {"able", "ible", "al", "ful", "ic", "ish", "ive", "less", "ous", "y"}
COMMON_ARTICLES = {"a", "an", "the"}
COMMON_QUESTION_WORDS = {"who", "what", "where", "when", "why", "how", "which", "whom", "whose"}

# Common homophones/confused words for misuse detection
CONFUSED_WORDS = {
    "their": ["there", "they're"],
    "there": ["their", "they're"],
    "they're": ["their", "there"],
    "its": ["it's"],
    "it's": ["its"],
    "your": ["you're"],
    "you're": ["your"],
    "to": ["too", "two"],
    "too": ["to", "two"],
    "two": ["to", "too"],
    "then": ["than"],
    "than": ["then"],
    "affect": ["effect"],
    "effect": ["affect"],
}

# New lexicons for advanced grammar rules
NEGATION_WORDS = {"not", "no", "never", "n't"}
COMMON_CONJUNCTIONS = {"and", "but", "or", "nor", "for", "yet", "so", "although", "because", "since", "unless", "while"}
COMMON_TRANSITIVE_VERBS = {"eat", "make", "take", "give", "see", "find", "tell", "ask", "call", "need"} # Very limited list
COMMON_BE_VERBS = {"is", "am", "are", "was", "were", "be", "been", "being"}

# --- Rule Engine Design ---

class Rule:
    """Base class for a rule that applies a transformation to a text string.
    Strictness (0.0 to 1.0) influences how aggressively the rule is applied.
    """
    def __init__(self, strictness: float = 0.5):
        self.strictness = max(0.0, min(1.0, strictness)) # Clamp between 0 and 1

    def apply(self, text: str) -> str:
        """Applies the rule to the text and returns the transformed text."""
        raise NotImplementedError

# --- Concrete Rule Implementations ---

class RepeatedWordRule(Rule):
    """Removes simple, consecutive repeated words based on strictness.
    Higher strictness removes more repetitions.
    """
    def apply(self, text: str) -> str:
        # At strictness 0.0, remove 4+ repetitions. At 0.5, 3+. At 1.0, 2+.
        min_repetitions = math.ceil(4 - self.strictness * 2)

        pattern_str = r'(\b\w+\b)((?:\s+\1){' + str(int(min_repetitions) - 1) + r',})'
        pattern = re.compile(pattern_str, re.IGNORECASE)

        return pattern.sub(r'\1', text)

class CapitalizationRule(Rule):
    """Ensures the first word of every sentence is capitalized based on strictness.
    Higher strictness means more aggressive capitalization.
    """
    def apply(self, text: str) -> str:
        # Capitalize the very first letter of the entire text if it's a letter
        if text and text[0].islower() and self.strictness > 0:
            text = text[0].upper() + text[1:]

        # Find sentence starts (after .?!) and capitalize the next letter
        # Only apply if strictness is above a threshold
        if self.strictness > 0.2:
            def capitalize_match(match):
                return match.group(1) + match.group(2).upper()
            pattern = re.compile(r'([.?!]\s*)([a-z])')
            text = pattern.sub(capitalize_match, text)
            
        return text

class SentenceTerminationRule(Rule):
    """Ensures that sentences end with appropriate punctuation based on strictness.
    Higher strictness means always adding a period if missing.
    """
    def apply(self, text: str) -> str:
        if not text.strip():
            return text

        last_char = text.strip()[-1]
        # Determine minimum word count based on strictness
        # strictness 1.0 -> min_words 1
        # strictness 0.5 -> min_words 3
        # strictness 0.0 -> min_words 5
        min_words = math.ceil(5 - self.strictness * 4)

        if last_char.isalnum():
            if len(text.split()) >= min_words:
                return text + '.'
        return text

class PunctuationSpacingRule(Rule):
    """Ensures correct spacing around punctuation based on strictness.
    e.g., 'word .' -> 'word.'
    """
    def apply(self, text: str) -> str:
        if self.strictness >= 0.5: # Changed from > 0.5 to >= 0.5
            # Remove space before punctuation
            text = re.sub(r'\s+([.,!?;:])', r'\1', text)
            # Ensure one space after punctuation
            text = re.sub(r'([.,!?;:])(\S)', r'\1 \2', text)
        return text

class SentenceLengthRule(Rule):
    """Flags or truncates sentences that are too long or too short based on strictness.
    For now, it will just add a warning/note.
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text

        sentences = re.split(r'([.?!])\s*', text)
        processed_sentences = []
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            punctuation = sentences[i+1] if i+1 < len(sentences) else '.'
            
            words = re.findall(r'\b\w+\b', sentence_text)
            num_words = len(words)

            min_len = 5 + (1 - self.strictness) * 5 # 5 at strict 1.0, 10 at strict 0.0
            max_len = 25 + self.strictness * 25 # 50 at strict 1.0, 25 at strict 0.0

            if num_words < min_len and self.strictness > 0.5:
                logger.warning(f"[SentenceLengthRule] Sentence too short (len={num_words}): '{sentence_text}...'")
            elif num_words > max_len and self.strictness > 0.5:
                logger.warning(f"[SentenceLengthRule] Sentence too long (len={num_words}): '{sentence_text}...'")
            
            processed_sentences.append(sentence_text + punctuation)
        
        return " ".join(processed_sentences).strip()

class PronounRule(Rule):
    """Identifies common pronouns and can flag unusual usage based on strictness.
    (Very basic, does not check grammatical correctness).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for word in words:
            if word in COMMON_PRONOUNS and self.strictness > 0.8:
                # Example: could flag or try to correct, for now just logs
                logger.debug(f"[PronounRule] Found pronoun: '{word}'")
        return text

class VerbFormRule(Rule):
    """Identifies words that look like verbs based on common suffixes.
    (Very basic, does not check grammatical correctness).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for word in words:
            if any(word.endswith(suffix) for suffix in COMMON_VERB_SUFFIXES) and self.strictness > 0.8:
                logger.debug(f"[VerbFormRule] Found verb-like word: '{word}'")
        return text

class AdjectiveRule(Rule):
    """Identifies words that look like adjectives based on common suffixes.
    (Very basic, does not check grammatical correctness).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for word in words:
            if any(word.endswith(suffix) for suffix in COMMON_ADJECTIVE_SUFFIXES) and self.strictness > 0.8:
                logger.debug(f"[AdjectiveRule] Found adjective-like word: '{word}'")
        return text

class NounRule(Rule):
    """Identifies words that look like nouns based on common suffixes or capitalization.
    (Very basic, does not check grammatical correctness).
    """
    COMMON_NOUN_SUFFIXES = {"tion", "sion", "ment", "ness", "ity", "er", "or", "ist", "ism"}
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text)
        for word in words:
            is_noun_like = False
            if word[0].isupper() and len(word) > 1: # Simple check for proper nouns
                is_noun_like = True
            elif any(word.lower().endswith(suffix) for suffix in self.COMMON_NOUN_SUFFIXES):
                is_noun_like = True
            
            if is_noun_like and self.strictness > 0.8:
                logger.debug(f"[NounRule] Found noun-like word: '{word}'")
        return text

class ArticleRule(Rule):
    """Identifies common articles (a, an, the).
    (Very basic, does not check grammatical correctness).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for word in words:
            if word in COMMON_ARTICLES and self.strictness > 0.8:
                logger.debug(f"[ArticleRule] Found article: '{word}'")
        return text

class QuestionRule(Rule):
    """Identifies sentences that are questions.
    (Very basic, based on punctuation or starting with question words).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            punctuation = sentences[i+1] if i+1 < len(sentences) else ''
            
            is_question = False
            if punctuation == '?':
                is_question = True
            else:
                first_word = re.match(r'\b\w+\b', sentence_text.lower())
                if first_word and first_word.group(0) in COMMON_QUESTION_WORDS:
                    is_question = True
            
            if is_question and self.strictness > 0.8:
                logger.debug(f"[QuestionRule] Found question: '{sentence_text}...'")
        return text

class QuestionWordRule(Rule):
    """Identifies common question words.
    (Very basic, does not check grammatical correctness).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for word in words:
            if word in COMMON_QUESTION_WORDS and self.strictness > 0.8:
                logger.debug(f"[QuestionWordRule] Found question word: '{word}'")
        return text

class PunctuationConsistencyRule(Rule):
    """Ensures consistent use of punctuation (e.g., no multiple punctuation marks, correct spacing).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        # 1. Remove multiple punctuation marks first to avoid spacing issues.
        if self.strictness >= 0.5:
            text = re.sub(r'([.,!?;:])([.,!?;:])+', r'\1', text)
        # 2. Remove space before punctuation.
        text = re.sub(r'\s+([.,!?;:])', r'\1', text)
        # 3. Ensure one space after punctuation.
        text = re.sub(r'([.,!?;:])(\S)', r'\1 \2', text)
        return text

class CommonWordMisuseRule(Rule):
    """Flags common misuses of homophones or easily confused words.
    (Very basic, does not attempt correction).
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for i, word in enumerate(words):
            if word in CONFUSED_WORDS and self.strictness > 0.5:
                # This rule is just for flagging, not correcting.
                logger.debug(f"[CommonWordMisuseRule] Potentially misused word: '{word}'")
        return text

class SentenceStartVarietyRule(Rule):
    """Flags if too many consecutive sentences start with the same word.
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'[.?!]\s*', text)
        sentences = [s.strip() for s in sentences if s.strip()]
        
        if len(sentences) < 3: return text # Need at least 3 sentences to check variety

        start_words = []
        for sentence in sentences:
            match = re.match(r'\b\w+\b', sentence.lower())
            if match: start_words.append(match.group(0))

        if len(start_words) < 3: return text

        # Check for 3 consecutive sentences starting with the same word
        for i in range(len(start_words) - 2):
            if start_words[i] == start_words[i+1] == start_words[i+2] and self.strictness > 0.7:
                logger.warning(f"[SentenceStartVarietyRule] Lack of variety: 3+ sentences start with '{start_words[i]}'")
                break # Flag once per text
        return text

class BasicSubjectVerbAgreementRule(Rule):
    """Very basic check for subject-verb agreement for common pronouns.
    Flags simple mismatches like 'he go' vs 'he goes'.
    """
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            words = re.findall(r'\b\w+\b', sentence_text.lower())
            
            # Simple check: pronoun followed by verb form
            for j, word in enumerate(words):
                if word in ["he", "she", "it"] and j + 1 < len(words):
                    next_word = words[j+1]
                    # If singular subject, verb should end in 's' (present tense)
                    if not next_word.endswith('s') and self.strictness > 0.7:
                        logger.warning(f"[BasicSubjectVerbAgreementRule] Possible agreement error: '{word} {next_word}'")
                elif word in ["i", "we", "you", "they"] and j + 1 < len(words):
                    next_word = words[j+1]
                    # If plural subject, verb should NOT end in 's' (present tense)
                    if next_word.endswith('s') and self.strictness > 0.7:
                        logger.warning(f"[BasicSubjectVerbAgreementRule] Possible agreement error: '{word} {next_word}'")
        return text

class DeterminerQuantifierRule(Rule):
    """Identifies common determiners, quantifiers, and cardinal numbers.
    (Very basic, does not check grammatical correctness).
    """
    COMMON_DETERMINERS_QUANTIFIERS = {
        "a", "an", "the", "this", "that", "these", "those",
        "my", "your", "his", "her", "its", "our", "their",
        "each", "every", "either", "neither",
        "some", "any", "many", "much", "few", "little", "all", "both", "half", "several", "enough"
    }
    COMMON_CARDINAL_NUMBERS = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"}

    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        words = re.findall(r'\b\w+\b', text.lower())
        for word in words:
            if word in self.COMMON_DETERMINERS_QUANTIFIERS and self.strictness > 0.8:
                logger.debug(f"[DeterminerQuantifierRule] Found determiner/quantifier: '{word}'")
            elif word.isdigit() and self.strictness > 0.8: # Check for digit numbers
                logger.debug(f"[DeterminerQuantifierRule] Found cardinal number (digit): '{word}'")
            elif word in self.COMMON_CARDINAL_NUMBERS and self.strictness > 0.8:
                logger.debug(f"[DeterminerQuantifierRule] Found cardinal number (word): '{word}'")
        return text

class NounOfAddressRule(Rule):
    """Flags potential nouns of address (e.g., 'John,' at the start of a sentence).
    (Very basic, does not check grammatical correctness)."""
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            # Check for capitalized word followed by comma at sentence start
            match = re.match(r'^([A-Z][a-z]*),\s*', sentence_text)
            if match and self.strictness > 0.8:
                logger.debug(f"[NounOfAddressRule] Found potential noun of address: '{match.group(1)}'")
        return text

class ConjunctionUsageRule(Rule):
    """Flags common issues with conjunctions.
    (Very basic, does not check grammatical correctness)."""
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            words = re.findall(r'\b\w+\b', sentence_text.lower())
            
            if not words: continue

            # Flag if sentence starts with a conjunction (if strictness is high)
            if words[0] in COMMON_CONJUNCTIONS and self.strictness > 0.7:
                logger.warning(f"[ConjunctionUsageRule] Sentence starts with conjunction: '{sentence_text}...'")
            
            # Flag consecutive conjunctions
            for j in range(len(words) - 1):
                if words[j] in COMMON_CONJUNCTIONS and words[j+1] in COMMON_CONJUNCTIONS and self.strictness > 0.9:
                    logger.warning(f"[ConjunctionUsageRule] Consecutive conjunctions: '{words[j]} {words[j+1]}'")
        return text

class DoubleNegativeRule(Rule):
    """Identifies and flags sentences with double negatives.
    (Very basic, does not check grammatical correctness)."""
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            words = re.findall(r'\b\w+\b', sentence_text.lower())
            
            negation_count = sum(1 for word in words if word in NEGATION_WORDS)
            if negation_count >= 2 and self.strictness > 0.7:
                logger.warning(f"[DoubleNegativeRule] Double negative detected: '{sentence_text}...'")
        return text

class ProperNounCapitalizationRule(Rule):
    """Flags proper nouns that are not capitalized.
    (Very basic, assumes proper nouns are capitalized unless at sentence start)."""
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            words = re.findall(r'\b\w+\b', sentence_text)
            
            for j, word in enumerate(words):
                if j == 0: continue # Skip first word of sentence (handled by CapitalizationRule)
                if len(word) > 1 and word[0].isupper() and not word[1:].islower() and self.strictness > 0.8: # Heuristic for proper noun
                    # This is a very weak heuristic, just for demonstration
                    logger.warning(f"[ProperNounCapitalizationRule] Possible uncapitalized proper noun: '{word}'")
        return text

class BasicVerbArgumentRule(Rule):
    """Very basic check for common transitive verbs lacking an apparent object.
    (Highly simplified, prone to false positives/negatives)."""
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            words = re.findall(r'\b\w+\b', sentence_text.lower())
            
            for j, word in enumerate(words):
                if word in COMMON_TRANSITIVE_VERBS and j + 1 == len(words) and self.strictness > 0.8:
                    logger.warning(f"[BasicVerbArgumentRule] Transitive verb '{word}' at end of sentence without object.")
        return text

class WordOrderRule(Rule):
    """Very basic check for simple subject-verb order deviations.
    Flags if a common verb appears before a common subject pronoun.
    (Highly simplified, prone to false positives/negatives)."""
    def apply(self, text: str) -> str:
        if self.strictness == 0: return text
        sentences = re.split(r'([.?!])\s*', text)
        for i in range(0, len(sentences), 2):
            sentence_text = sentences[i].strip()
            if not sentence_text: continue
            
            words = re.findall(r'\b\w+\b', sentence_text.lower())
            
            for j, word in enumerate(words):
                if word in COMMON_BE_VERBS and j + 1 < len(words) and words[j+1] in COMMON_PRONOUNS and self.strictness > 0.8:
                    logger.warning(f"[WordOrderRule] Possible inverted subject-verb order: '{word} {words[j+1]}'")
        return text

# --- The Rule Enforcer ---

class RuleEnforcer:
    """
    A system to apply a suite of rules to clean up generated text.
    Rules are applied sequentially.
    """
    def __init__(self, strictness: float = 0.5, rules: list[Rule] = None):
        self.strictness = max(0.0, min(1.0, strictness)) # Clamp between 0 and 1
        
        if rules is None:
            # Default suite of rules. Order matters!
            self.rules = [
                RepeatedWordRule(strictness=self.strictness),
                PunctuationSpacingRule(strictness=self.strictness),
                CapitalizationRule(strictness=self.strictness),
                SentenceTerminationRule(strictness=self.strictness),
                SentenceLengthRule(strictness=self.strictness),
                PronounRule(strictness=self.strictness),
                VerbFormRule(strictness=self.strictness),
                AdjectiveRule(strictness=self.strictness),
                NounRule(strictness=self.strictness),
                ArticleRule(strictness=self.strictness),
                QuestionRule(strictness=self.strictness),
                QuestionWordRule(strictness=self.strictness),
                PunctuationConsistencyRule(strictness=self.strictness),
                CommonWordMisuseRule(strictness=self.strictness),
                SentenceStartVarietyRule(strictness=self.strictness),
                BasicSubjectVerbAgreementRule(strictness=self.strictness),
                DeterminerQuantifierRule(strictness=self.strictness),
                NounOfAddressRule(strictness=self.strictness),
                ConjunctionUsageRule(strictness=self.strictness),
                DoubleNegativeRule(strictness=self.strictness),
                ProperNounCapitalizationRule(strictness=self.strictness),
                BasicVerbArgumentRule(strictness=self.strictness),
                WordOrderRule(strictness=self.strictness),
            ]
        else:
            self.rules = rules
            # Ensure custom rules also get the strictness if they don't have it
            for rule in self.rules:
                if not hasattr(rule, 'strictness'):
                    rule.strictness = self.strictness

    def apply_rules(self, text: str) -> str:
        """
        Applies all configured rules in sequence to a text string.

        Args:
            text (str): The raw text generated by the model.

        Returns:
            str: The cleaned and corrected text.
        """
        logger.debug(f"Applying rules to text (strictness={self.strictness}): '{text[:50]}...'")
        corrected_text = text
        for i, rule in enumerate(self.rules):
            original_text_len = len(corrected_text)
            corrected_text = rule.apply(corrected_text)
            if len(corrected_text) != original_text_len:
                logger.debug(f"  Rule #{i+1} ({rule.__class__.__name__}) applied changes.")
            else:
                logger.debug(f"  Rule #{i+1} ({rule.__class__.__name__}) no changes.")
        
        return corrected_text

# --- Example Usage ---
def main():
    """
    Example of how to use the RuleEnforcer.
    To see debug/warning messages, configure logging level:
    logging.basicConfig(level=logging.DEBUG, stream=sys.stdout, format='%(levelname)s: %(message)s')
    """
    # Example with default strictness
    enforcer_default = RuleEnforcer()
    raw_text_default = "the future of AI is the the future. it will be amazing amazing. we will see new things"
    print("---", "Default Strictness (0.5) ---")
    print(f"Raw text:    '{raw_text_default}'")
    clean_text_default = enforcer_default.apply_rules(raw_text_default)
    print(f"Clean text:  '{clean_text_default}'")
    print("--------------------------------")

    # Example with strictness 0.0 (very lenient)
    enforcer_lenient = RuleEnforcer(strictness=0.0)
    raw_text_lenient = "i am a boy. he is a girl. they are people. the the the. this is a test"
    print("---", "Lenient Strictness (0.0) ---")
    print(f"Raw text:    '{raw_text_lenient}'")
    clean_text_lenient = enforcer_lenient.apply_rules(raw_text_lenient)
    print(f"Clean text:  '{clean_text_lenient}'")
    print("--------------------------------")

    # Example with strictness 1.0 (very strict)
    enforcer_strict = RuleEnforcer(strictness=1.0)
    raw_text_strict = "this is a test of the rule enforcer. it has some repeated words. words words. and missing punctuation"
    print("---", "Strict Strictness (1.0) ---")
    print(f"Raw text:    '{raw_text_strict}'")
    clean_text_strict = enforcer_strict.apply_rules(raw_text_strict)
    print(f"Clean text:  '{clean_text_strict}'")
    print("--------------------------------")

    # New examples for the added rules
    print("\n--- New Rule Examples ---")
    logging.basicConfig(level=logging.DEBUG, stream=sys.stdout, format='%(levelname)s: %(message)s')

    # Punctuation Consistency
    print("\n--- Punctuation Consistency Rule ---")
    raw = "Hello   .world!!! How are you??"
    print(f"Raw: '{raw}'")
    print(f"Clean: '{RuleEnforcer(strictness=1.0, rules=[PunctuationConsistencyRule(strictness=1.0)]).apply_rules(raw)}'")

    # Common Word Misuse
    print("\n--- Common Word Misuse Rule ---")
    raw = "Their is a cat over there. Its a nice day. You're going to love it."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[CommonWordMisuseRule(strictness=1.0)]).apply_rules(raw)

    # Sentence Start Variety
    print("\n--- Sentence Start Variety Rule ---")
    raw = "The dog barked. The cat meowed. The bird sang. The sun shone."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[SentenceStartVarietyRule(strictness=1.0)]).apply_rules(raw)

    # Basic Subject-Verb Agreement
    print("\n--- Basic Subject-Verb Agreement Rule ---")
    raw = "He go to the store. They goes to the park. I am happy."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[BasicSubjectVerbAgreementRule(strictness=1.0)]).apply_rules(raw)

    # Determiner/Quantifier Rule
    print("\n--- Determiner/Quantifier Rule ---")
    raw = "One dog. Every cat. Some birds. Many people. 10 apples."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[DeterminerQuantifierRule(strictness=1.0)]).apply_rules(raw)

    # Noun of Address Rule
    print("\n--- Noun of Address Rule ---")
    raw = "John, come here. Hello Mary, how are you?"
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[NounOfAddressRule(strictness=1.0)]).apply_rules(raw)

    # Conjunction Usage Rule
    print("\n--- Conjunction Usage Rule ---")
    raw = "And so it begins. But I must go. He went, and then he came back."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[ConjunctionUsageRule(strictness=1.0)]).apply_rules(raw)

    # Double Negative Rule
    print("\n--- Double Negative Rule ---")
    raw = "I don't know nothing. He never said nothing."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[DoubleNegativeRule(strictness=1.0)]).apply_rules(raw)

    # Proper Noun Capitalization Rule
    print("\n--- Proper Noun Capitalization Rule ---")
    raw = "hello World. my name is John. i live in new york."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[ProperNounCapitalizationRule(strictness=1.0)]).apply_rules(raw)

    # Basic Verb Argument Rule
    print("\n--- Basic Verb Argument Rule ---")
    raw = "He eat. She give. They take the ball."
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[BasicVerbArgumentRule(strictness=1.0)]).apply_rules(raw)

    # Word Order Rule
    print("\n--- Word Order Rule ---")
    raw = "Goes he to the store. Is she happy?"
    print(f"Raw: '{raw}'")
    RuleEnforcer(strictness=1.0, rules=[WordOrderRule(strictness=1.0)]).apply_rules(raw)

if __name__ == "__main__":
    main()