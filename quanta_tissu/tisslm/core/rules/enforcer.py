
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
        # At strictness 0.0, only remove 3+ repetitions. At 1.0, remove 2+ repetitions.
        min_repetitions = 2 + (1 - self.strictness) # 2 at strictness 1.0, 3 at strictness 0.0
        
        # Regex to find a word followed by itself at least min_repetitions times
        # e.g., (\b\w+\b\s+)(\1){min_repetitions-1,}_SENTINEL_
        # The sentinel helps catch repetitions at the end of the string.
        pattern_str = r'(\b\w+\b\s+)(\1){' + str(int(min_repetitions) - 1) + r',}_SENTINEL_'
        pattern = re.compile(pattern_str, re.IGNORECASE)
        
        transformed_text = pattern.sub(r'\1', text + '_SENTINEL_')
        return transformed_text.replace('_SENTINEL_', '')

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
            pattern = re.compile(r'([.?!]\s+)([a-z])')
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
        # Only add if strictness is high or if it's clearly missing punctuation
        if last_char.isalnum() and self.strictness > 0.5:
            return text + '.'
        elif last_char.isalnum() and self.strictness <= 0.5:
            # Lenient: only add if text is long enough to be a sentence
            if len(text.split()) > 5: # Simple word count heuristic
                return text + '.'
        return text

class PunctuationSpacingRule(Rule):
    """Ensures correct spacing around punctuation based on strictness.
    e.g., 'word .' -> 'word.'
    """
    def apply(self, text: str) -> str:
        if self.strictness > 0.5:
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
    """Example of how to use the RuleEnforcer."""
    # Example with default strictness
    enforcer_default = RuleEnforcer()
    raw_text_default = "the future of AI is the the future. it will be amazing amazing. we will see new things"
    print("--- Default Strictness (0.5) ---")
    print(f"Raw text:    '{raw_text_default}'")
    clean_text_default = enforcer_default.apply_rules(raw_text_default)
    print(f"Clean text:  '{clean_text_default}'")
    print("--------------------------------")

    # Example with strictness 0.0 (very lenient)
    enforcer_lenient = RuleEnforcer(strictness=0.0)
    raw_text_lenient = "i am a boy. he is a girl. they are people. the the the. this is a test"
    print("--- Lenient Strictness (0.0) ---")
    print(f"Raw text:    '{raw_text_lenient}'")
    clean_text_lenient = enforcer_lenient.apply_rules(raw_text_lenient)
    print(f"Clean text:  '{clean_text_lenient}'")
    print("--------------------------------")

    # Example with strictness 1.0 (very strict)
    enforcer_strict = RuleEnforcer(strictness=1.0)
    raw_text_strict = "this is a test of the rule enforcer. it has some repeated words. and missing punctuation. also bad spacing"
    print("--- Strict Strictness (1.0) ---")
    print(f"Raw text:    '{raw_text_strict}'")
    clean_text_strict = enforcer_strict.apply_rules(raw_text_strict)
    print(f"Clean text:  '{clean_text_strict}'")
    print("--------------------------------")

if __name__ == "__main__":
    main()
