
import unittest
import sys
import os
import logging
import re

# Adjust path to import modules from the project root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.rules.enforcer import (
    RuleEnforcer, Rule, RepeatedWordRule, CapitalizationRule,
    SentenceTerminationRule, PunctuationSpacingRule, SentenceLengthRule,
    PronounRule, VerbFormRule, AdjectiveRule
)

# Suppress logging output during tests for cleaner console output
logging.disable(logging.CRITICAL)

class TestRule(unittest.TestCase):
    def test_strictness_clamping(self):
        rule = Rule(strictness=1.5)
        self.assertEqual(rule.strictness, 1.0)
        rule = Rule(strictness=-0.5)
        self.assertEqual(rule.strictness, 0.0)
        rule = Rule(strictness=0.7)
        self.assertEqual(rule.strictness, 0.7)

class TestRepeatedWordRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = RepeatedWordRule()
        self.assertEqual(rule.strictness, 0.5)
        self.assertEqual(rule.apply("hello hello hello world"), "hello world")
        self.assertEqual(rule.apply("the the the quick brown fox"), "the quick brown fox")
        self.assertEqual(rule.apply("word word. word word"), "word word word") # Should not fix across punctuation
        self.assertEqual(rule.apply("a a a a a"), "a")
        self.assertEqual(rule.apply("test test"), "test test") # Default strictness (0.5) removes 3+ repetitions

    def test_strict_strictness(self):
        rule = RepeatedWordRule(strictness=1.0)
        self.assertEqual(rule.apply("test test"), "test") # Strictness 1.0 removes 2+ repetitions
        self.assertEqual(rule.apply("go go go"), "go")

    def test_lenient_strictness(self):
        rule = RepeatedWordRule(strictness=0.0)
        self.assertEqual(rule.apply("test test test"), "test") # Strictness 0.0 removes 3+ repetitions
        self.assertEqual(rule.apply("test test"), "test test")

class TestCapitalizationRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = CapitalizationRule()
        self.assertEqual(rule.apply("hello world. this is a test"), "Hello world. This is a test")
        self.assertEqual(rule.apply("first word.second word"), "First word.Second word")

    def test_lenient_strictness(self):
        rule = CapitalizationRule(strictness=0.0)
        self.assertEqual(rule.apply("hello world. this is a test"), "Hello world. this is a test") # Only first word capitalized

    def test_no_change_if_already_capitalized(self):
        rule = CapitalizationRule()
        self.assertEqual(rule.apply("Hello world. This is a test"), "Hello world. This is a test")

class TestSentenceTerminationRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = SentenceTerminationRule()
        self.assertEqual(rule.apply("This is a sentence"), "This is a sentence.")
        self.assertEqual(rule.apply("This is a sentence."), "This is a sentence.")
        self.assertEqual(rule.apply("Short"), "Short") # Not long enough for default strictness

    def test_strict_strictness(self):
        rule = SentenceTerminationRule(strictness=1.0)
        self.assertEqual(rule.apply("Short"), "Short.")
        self.assertEqual(rule.apply("Another one"), "Another one.")

    def test_lenient_strictness(self):
        rule = SentenceTerminationRule(strictness=0.0)
        self.assertEqual(rule.apply("This is a sentence"), "This is a sentence.")
        self.assertEqual(rule.apply("Short"), "Short")

class TestPunctuationSpacingRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = PunctuationSpacingRule()
        self.assertEqual(rule.apply("hello .world"), "hello. world")
        self.assertEqual(rule.apply("test , test"), "test, test")
        self.assertEqual(rule.apply("word!next"), "word! next")

    def test_lenient_strictness(self):
        rule = PunctuationSpacingRule(strictness=0.0)
        self.assertEqual(rule.apply("hello .world"), "hello .world") # No change

class TestSentenceLengthRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = SentenceLengthRule()
        # This rule primarily logs warnings, so we check if it doesn't alter text
        self.assertEqual(rule.apply("This is a very long sentence that should trigger a warning because it exceeds the default length."), 
                         "This is a very long sentence that should trigger a warning because it exceeds the default length..") # Adds period
        self.assertEqual(rule.apply("Short."), "Short.")

    def test_strict_strictness(self):
        rule = SentenceLengthRule(strictness=1.0)
        self.assertEqual(rule.apply("This is a very long sentence that should trigger a warning because it exceeds the strict length."), 
                         "This is a very long sentence that should trigger a warning because it exceeds the strict length..") # Adds period
        self.assertEqual(rule.apply("Short."), "Short.")

class TestPronounRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = PronounRule()
        # This rule primarily logs debug messages, so we check if it doesn't alter text
        self.assertEqual(rule.apply("I went to the store with him."), "I went to the store with him.")

    def test_strict_strictness(self):
        rule = PronounRule(strictness=1.0)
        self.assertEqual(rule.apply("She loves her cat."), "She loves her cat.")

class TestVerbFormRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = VerbFormRule()
        self.assertEqual(rule.apply("He is running and jumped."), "He is running and jumped.")

class TestAdjectiveRule(unittest.TestCase):
    def test_default_strictness(self):
        rule = AdjectiveRule()
        self.assertEqual(rule.apply("It was a beautiful and amazing day."), "It was a beautiful and amazing day.")

class TestRuleEnforcer(unittest.TestCase):
    def test_default_strictness(self):
        enforcer = RuleEnforcer()
        raw_text = "the the the quick brown fox. it was amazing amazing. word!next"
        expected_text = "The quick brown fox. It was amazing. Word! next."
        self.assertEqual(enforcer.apply_rules(raw_text), expected_text)

    def test_strict_strictness(self):
        enforcer = RuleEnforcer(strictness=1.0)
        raw_text = "the the quick brown fox. it was amazing amazing. word!next"
        expected_text = "The quick brown fox. It was amazing. Word! next."
        self.assertEqual(enforcer.apply_rules(raw_text), expected_text)

    def test_lenient_strictness(self):
        enforcer = RuleEnforcer(strictness=0.0)
        raw_text = "the the the quick brown fox. it was amazing amazing. word!next"
        # Expect less aggressive changes
        expected_text = "The the the quick brown fox. it was amazing amazing. word!next."
        self.assertEqual(enforcer.apply_rules(raw_text), expected_text)

if __name__ == '__main__':
    unittest.main()
