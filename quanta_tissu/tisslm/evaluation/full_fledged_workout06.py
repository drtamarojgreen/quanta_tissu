import os
import sys
import re

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.grammar_parser import GrammarParser
from quanta_tissu.tisslm.core.lexicon_analyzer import LexiconAnalyzer
from quanta_tissu.tisslm.core.context_reviewer import ContextReviewer

def run_advanced_analysis_tests(generated_texts):
    """Evaluates advanced text analysis tools on generated text."""
    report = ["\n--- Test 6: Advanced Text Analysis (Grammar, Lexicon, Context) ---"]
    grammar_parser = GrammarParser()
    lexicon_analyzer = LexiconAnalyzer()
    context_reviewer = ContextReviewer()

    for item in generated_texts:
        analysis_text = item['text']
        report.append(f"\n  --- Analysis for: '{analysis_text[:50]}...' ---")
        # Grammar
        sentences = re.split(r'(?<=[.?!])\s+', analysis_text)
        grammar_analysis = grammar_parser.analyze_sentence(sentences[0] if sentences else "")
        report.append(f"    Grammar (Noun Phrases): {grammar_analysis['noun_phrases']}")
        # Lexicon
        lexicon_analysis = lexicon_analyzer.analyze_text(analysis_text, top_n_collocations=3)
        report.append(f"    Lexicon (Top Bigrams): {lexicon_analysis['collocations']}")
        # Context
        context_analysis = context_reviewer.analyze_text(analysis_text)
        report.append(f"    Context Unity OK: {not context_analysis['paragraph_unity']}")
    return report
