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
    report = ["\n--- Test 7: Advanced Text Analysis (Grammar, Lexicon, Context) ---"]
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
        if not context_analysis['paragraph_unity']:
            report.append(f"    Context Unity: OK")
        else:
            report.append(f"    Context Unity: Issues found - {context_analysis['paragraph_unity']}")
            
    return report

if __name__ == '__main__':
    # Example generated texts for testing
    generated_texts = [
        {"text": "The quick brown fox jumps over the lazy dog. A cat sat on the mat."},
        {"text": "My friend will eat an apple. The apple is red and delicious."},
        {"text": "This is a test sentence. This is another test sentence with different words to see how the analysis works."},
        {"text": "This is a paragraph with good unity. All the sentences are about the same topic. The topic is paragraph unity. This makes the paragraph easy to read and understand."},
        {"text": "This paragraph has some issues. It starts with one topic, but then it switches to another topic. For example, it talks about cats, but then it talks about cars. This makes the paragraph confusing."}
    ]
    
    # Run the analysis and print the report
    analysis_report = run_advanced_analysis_tests(generated_texts)
    for line in analysis_report:
        print(line)